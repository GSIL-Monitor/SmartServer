#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <syslog.h>
#include <arpa/inet.h>
#include "../../app/app_config.h"
#include "../../inc/thread_info.h"
#include "../../net/http/http_client.h"
#include "../../inc/log.h"
#include "gateway_cmd.h"

#define GATEWAY_BUF_SIZE			1024 * 2 

void* gateway_client_thread(SOCKET_T sockfd)
//void* gateway_client_thread(void* arg)
{
	struct sockaddr_in addr;
	socklen_t addr_len = static_cast<socklen_t>(sizeof(addr));
	char strIP[22] = {0};
	
	if(0 == getpeername(sockfd, reinterpret_cast<struct sockaddr *>(&addr), &addr_len))
	{
		char *str = inet_ntoa(addr.sin_addr);
		unsigned short port = ntohs(addr.sin_port);
		snprintf(strIP, sizeof(strIP), "%s:%d", str, port);
	}
	
	syslog(LOG_INFO_FACILITY, "ThreadID=0x%X,IP=%s,New gateway client thread start....,sockfd=%d\n",
				std::this_thread::get_id(), strIP, sockfd);
	ThreadInfoStruct info;
	info.UserInfoPosition = -1;
	info.GatewayInfoPosition = -1;

	char buf[GATEWAY_BUF_SIZE];

	size_t len = (size_t)GATEWAY_BUF_SIZE;
	int err;
	time_t timep;
	struct tm *p;
	
	time(&timep);		
	p=localtime(&timep);		
	char strRet[255];		
	int len1 = snprintf(strRet, sizeof(strRet), "Action=Identify*Time=%04d-%02d-%02d %02d:%02d:%02d*",
				(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,
				(0+p->tm_hour),p->tm_min,p->tm_sec);
	send(sockfd,strRet,len1,0);

	ssize_t TotalLen = 0;
	ssize_t num;
	while (1)
	{
		num = recv(sockfd, buf+TotalLen, GATEWAY_BUF_SIZE-TotalLen, 0/*MSG_WAITALL*/);
		if (num <0)
		{
			LogE("gateway_client_thread  recv()<0,there is error,thread exit\n");
			break;
		}
		else if (num == 0)
		{
			LogE("gateway_client_thread  recv()=0,socket closed,thread exit\n");
			break;
		}
		TotalLen += num;
		if(TotalLen >= GATEWAY_BUF_SIZE)//recv too many data,consider it is not normal,close socket,exit thread;
		{
			LogE("Gateway recv too much data,it is not normal,close socket and exit thread\n");
			break;
		}		
		buf[TotalLen] = '\0';
		time(&timep);		
		p=localtime(&timep);		
			
		LogI("[%d-%02d-%02d %02d:%02d:%02d]",
				(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,
				(0+p->tm_hour),p->tm_min,p->tm_sec);
		LogI(" Gateway recv:%s\r\n",buf);
		std::string strBody(buf);
		int ret = gateway_cmd(sockfd,strBody,&info);
		//LogD("ret = gateway_cmd() = %d\n",ret);
		TotalLen = 0;
	}
	close(sockfd);
	return nullptr;
}
