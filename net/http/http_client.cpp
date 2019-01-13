#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <algorithm>
#include <unistd.h>
#include <errno.h>
#include <atomic>
#include <thread>
#include <syslog.h>
#include "../../app/app_config.h"
#include "../../inc/thread_info.h"
#include "../../inc/mystring.h"
#include "../../net/http/http_get.h"
#include "../../net/http/http_post.h"
#include "../../inc/log.h"
#include "../../net/inc/socket_server.h"
#include "../../net/inc/atomic_raii.h"
//using namespace std;

#define HTTP_BUF_SIZE			1024 * 2
#define HTTP_HEADER_SIZE		1024 * 2
//void * http_client_thread(void *arg)
void * http_client_thread(SOCKET_T sockfd)
{
	//http_thread_num.fetch_add(1);
	ThreadNumRAII Raii(&http_thread_num);
	struct sockaddr_in addr;
	socklen_t addr_len = static_cast<socklen_t>(sizeof(addr));
	char strIP[22] = {0};
	
	if(0 == getpeername(sockfd, reinterpret_cast<struct sockaddr *>(&addr), &addr_len))
	{
		char *str = inet_ntoa(addr.sin_addr);
		unsigned short port = ntohs(addr.sin_port);
		snprintf(strIP, sizeof(strIP), "%s:%d", str, port);
	}
	syslog(LOG_INFO_FACILITY, "ThreadID=0x%X,IP=%s,New http client thread start....sockfd=%d,http_thread_num=%d\n",
			std::this_thread::get_id(), strIP,sockfd,http_thread_num.load(std::memory_order_relaxed));
	//SOCKET_T *pSocket = static_cast<SOCKET_T *>(arg);
	//SOCKET_T sockfd = *pSocket;
	//delete pSocket;
	
	ThreadInfoStruct info;
	info.UserInfoPosition = -1;
	info.GatewayInfoPosition = -1;
	//struct thread_info *pinfo = (struct thread_info *)arg;
	//int socket_fd = pinfo->socket_fd;
	//pthread_t tid = pinfo->tid;
	//delete pinfo;
	//int flags = fcntl(sockfd, F_GETFL, 0);
	//fcntl(socket_fd, F_SETFL, flags &~O_NONBLOCK);
	
	struct timeval ti;
	ti.tv_sec = SOCKET_RECV_TIMEOUT;	//Recv timeout in seconds
	ti.tv_usec = 0;
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&ti,sizeof(ti));

	char buf[HTTP_BUF_SIZE];

	size_t len = (size_t)HTTP_BUF_SIZE;
	int err;
	time_t timep;
	struct tm *p;

	ssize_t TotalLen = 0;
	ssize_t num;
	while (1)
	{
		//pthread_mutex_lock(&mutex);
		//memset(buf, 0, len);
		num = recv(sockfd, buf+TotalLen, HTTP_BUF_SIZE-TotalLen, 0/*MSG_WAITALL*/);
		if (num <0)
		{
			if(errno == 11)			
				LogE("http_client_thread recv timeout,thread exit.errno=%d\n",errno);
			else
				LogE("http_client_thread recv()<0,thread exit,errno=%d(%s)\n",errno,strerror(errno));
			break;
		}
		else if (num == 0)
		{
			LogE("http_client_thread  recv()=0,socket=%d closed,thread exit\n",sockfd);
			break;
		}
		TotalLen += num;
		if(TotalLen >= HTTP_BUF_SIZE)//recv too many data,consider it is not normal,close socket,exit thread;
		{
			LogE("http recv too much data,it is not normal,close socket and exit thread\n");
			break;
		}		
		buf[TotalLen] = '\0';
		if(TotalLen<3)
			continue;
		if(strncmp(buf,"GET",3) == 0)
		{
			//this is a GET request;
			if(strstr(buf,"\r\n\r\n")== NULL)			
				continue;
			int iRet = HttpGet(sockfd,buf);
			if(iRet !=0 )
				break;
			TotalLen=0;
			continue;
		}
		if(TotalLen<4)
			continue;
		if(strncmp(buf,"POST",4) !=0)
			break;//this is not a POST or GET request,exit thread;
			
		char *pHeaderEnd = strstr(buf,"\r\n\r\n");
		if(pHeaderEnd == NULL)
			continue;
			
		//char strHeader[HTTP_HEADER_SIZE];
		std::string strHeader;
		//char c = *(pHeaderEnd+4);
		//*(pHeaderEnd+4) = '\0';
		strHeader.append(buf,(int)(pHeaderEnd-buf+4));		
		
		std::string strContentLength;
		bool bRet = GetHeaderInfo(strHeader,"Content-Length",strContentLength);
		if(!bRet)
		{
			LogE("there is no Content-Length in header,exit thread\n");
			break;
		}
		int ContentLength = atoi(strContentLength.c_str());
		//printf("Content-Length:%s:%ld\n",strContentLength.c_str(),ContentLength);
		if(TotalLen-(int)(pHeaderEnd-buf+4) < ContentLength)
		{
			LogI("recv data is not enough,continue to recv\n");
			continue;
		}
		//printf("http recv completed\n");
		
		
		std::transform(strHeader.begin(),strHeader.end(),strHeader.begin(),::tolower);// to lower case
		std::string strConnection;
		bRet = GetHeaderInfo(strHeader,"connection",strConnection);
		if(!bRet)
		{
			LogE("there is no connection in header,exit thread\n");
			break;
		}
		//printf("connection:%s\n",strConnection.c_str());
		
		time(&timep);		
		p=localtime(&timep);
		if(ContentLength>0)
		{
			const char *pBody = pHeaderEnd+4;
			//LogI("[%d-%02d-%02d %02d:%02d:%02d]",
			//		(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,
			//		(0+p->tm_hour),p->tm_min,p->tm_sec);
			LogI("ThreadID=0x%X,HttpRecv:%s\r\n",std::this_thread::get_id(), pBody);
					/*
			LogI("[%d-%02d-%02d %02d:%02d:%02d] Http recv:%s\n",
					(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,
					(0+p->tm_hour),p->tm_min,p->tm_sec,pBody);*/
			HttpPost(sockfd,pBody,&info);
			TotalLen=0;
			if(strConnection=="keep-alive" || strConnection=="keepalive")
				continue;
			else if(strConnection=="close")
				break;
			continue;
			
		}
	/*	
		time(&timep);
		//p=gmtime(&timep);
		p=localtime(&timep);
		err = printf("[%d-%02d-%02d %02d:%02d:%02d] http_client_thread 0x%08X recv:%s\n",
			(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,
			(0+p->tm_hour),p->tm_min,p->tm_sec,
			tid, buf);
			*/
		usleep(100);
		//pthread_mutex_unlock(&mutex);
	}
thread_exit:
	close(sockfd);
	syslog(LOG_INFO_FACILITY, "ThreadID=0x%X,http_client_thread exit.", std::this_thread::get_id());
	return nullptr;
}
