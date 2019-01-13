#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>	//close
#include "../app/app_config.h"
#include "../inc/log.h"
#include "base64.h"
/*
struct Base64Date6
{
unsigned int d4:6;
unsigned int d3:6;
unsigned int d2:6;
unsigned int d1:6;
};


char ConvertToBase64(char uc)
{
	if(uc <26)
	{
		return'A'+uc;
	}
	if(uc <52)
	{
		return'a'+(uc -26);
	}
	if(uc <62)
	{
		return'0'+(uc -52);
	}
	if(uc ==62)
	{
		return'+';
	}
	return'/';
}

void  EncodeBase64(char*dbuf,char*buf128,int len)
{
	struct  Base64Date6*ddd =NULL;
	int i =0;
	char buf[256]={0};
	char *tmp =NULL;
	char cc ='\0';
	memset(buf,0,256);
	strcpy(buf,buf128);
	for(i =1;i <=len/3;i++)
	{
		tmp =buf +(i-1)*3;
		cc =tmp[2];
		tmp[2]=tmp[0];
		tmp[0]=cc;
		ddd =(struct Base64Date6*)tmp;
		dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i-1)*4+2]=ConvertToBase64((unsigned int)ddd->d3);
		dbuf[(i-1)*4+3]=ConvertToBase64((unsigned int)ddd->d4);
	}
	if(len %3==1)
	{
		tmp =buf +(i-1)*3;
		cc =tmp[2];
		tmp[2]=tmp[0];
		tmp[0]=cc;
		ddd =(struct Base64Date6*)tmp;
		dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i-1)*4+2]='=';
		dbuf[(i-1)*4+3]='=';
	}
	if(len%3==2)
	{
		tmp =buf+(i-1)*3;
		cc =tmp[2];
		tmp[2]=tmp[0];
		tmp[0]=cc;
		ddd =(struct Base64Date6*)tmp;
		dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i-1)*4+2]=ConvertToBase64((unsigned int)ddd->d3);
		dbuf[(i-1)*4+3]='=';
	}
	return;
}
*/

class Socket_RAII{
public:
	Socket_RAII(SOCKET_T sockfd)
	{
		m_sockfd = sockfd;
	}
	~Socket_RAII()
	{
		close(m_sockfd);
		LogI("RAII close socket.\n");
	}
private:
	SOCKET_T m_sockfd;
};

int SendEmail(
	const char *smtp,
	unsigned short port,
	const char *account,
	const char *password,
	const char* email,
	const char* body,
	const char* from_name,
	const char* to_name,
	const char* subject )
{
	//SOCKET_T sockServer = 0;
	struct sockaddr_in server_addr;
	//sockServer = socket(AF_INET, SOCK_STREAM, 0);
	//if(sockServer == -1)
	//{
	//	LogE("%s():create socket error:%s(errno:%d)\n", __func__,strerror(errno), errno);
	//	return -1;
	//}
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	//inet_pton(AF_INET, "183.232.125.184", &server_addr.sin_addr.s_addr);
	server_addr.sin_port = htons(port);
	hostent* hptr =gethostbyname(smtp); // 用的是163服务器
	server_addr.sin_addr = *(struct in_addr*)*hptr->h_addr_list;
	////memcpy(&server_addr.sin_addr.S_un.S_addr,hptr->h_addr_list[0],hptr->h_length);	
	SOCKET_T sockfd = socket(AF_INET, SOCK_STREAM, 0);
	Socket_RAII Raii(sockfd);
	printf("111\n");
	int ret = connect(sockfd,(struct sockaddr*)(&server_addr),sizeof(server_addr));
	if(ret == -1)
	{
		printf("%s():connect email server failed:%s(errno:%d)\n",__func__,strerror(errno),errno);
		return -1;
	}
	printf("222\n");
	char rbuf[255];	
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);
	if(ret <= 0)	
		return -2;
	if(strncmp(rbuf,"220",3) != 0)
		return -3;
	
	char sbuf[1500];	
	// EHLO
	int len = snprintf(sbuf,sizeof(sbuf),"EHLO HYL-PC\r\n");
	ret = send(sockfd,sbuf,len,0);
	if(ret == SOCKET_ERROR)
		return -4;
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);	
	if(ret <= 0)	
		return -5;
	if(strncmp(rbuf,"250",3) != 0)
		return -6;
		
	// AUTH LOGIN
	len = snprintf(sbuf,sizeof(sbuf),"AUTH LOGIN\r\n");
	ret = send(sockfd,sbuf,len,0);
	if(ret == SOCKET_ERROR)
		return -7;
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);	
	if(ret <= 0)	
		return -8;
	if(strncmp(rbuf,"334",3) != 0)
		return -9;
	
	//USER
	len = snprintf(sbuf,sizeof(sbuf),"%s",account);
	char login[128];
	memset(login,0,sizeof(login));
	EncodeBase64(login,sbuf,len);
	len = snprintf(sbuf,sizeof(sbuf),"%s\r\n",login);
	ret = send(sockfd,sbuf,len,0);
	if(ret == SOCKET_ERROR)
		return -10;
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);	
	if(ret <= 0)	
		return -11;
	if(strncmp(rbuf,"334",3) != 0)
		return -12;
		
	//PASSWORD
	len = snprintf(sbuf,sizeof(sbuf),"%s",password);
	//char login[128];
	memset(login,0,sizeof(login));
	EncodeBase64(login,sbuf,len);
	len = snprintf(sbuf,sizeof(sbuf),"%s\r\n",login);
	ret = send(sockfd,sbuf,len,0);
	if(ret == SOCKET_ERROR)
		return -13;
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);	
	if(ret <= 0)	
		return -14;
	if(strncmp(rbuf,"235",3) != 0)
		return -15;
	
	// MAIL FROM
	len = snprintf(sbuf,sizeof(sbuf),"MAIL FROM:<%s>\r\n",account);
	ret = send(sockfd,sbuf,len,0);
	if(ret == SOCKET_ERROR)
		return -16;
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);	
	if(ret <= 0)	
		return -17;
	if(strncmp(rbuf,"250",3) != 0)
		return -18;	
	
	// RCPT TO
	len = snprintf(sbuf,sizeof(sbuf),"RCPT TO:<%s>\r\n",email);
	ret = send(sockfd,sbuf,len,0);
	if(ret == SOCKET_ERROR)
		return -19;
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);	
	if(ret <= 0)	
		return -20;
	if(strncmp(rbuf,"250",3) != 0)
		return -21;
		
	// DATA 准备开始发送邮件内容
	len = snprintf(sbuf,sizeof(sbuf),"DATA\r\n");
	ret = send(sockfd,sbuf,len,0);
	if(ret == SOCKET_ERROR)
		return -22;
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);	
	if(ret <= 0)	
		return -23;
	if(strncmp(rbuf,"354",3) != 0)
		return -24;		
		
	// 发送邮件内容，\r\n.\r\n内容结束标记
	len = snprintf(sbuf,sizeof(sbuf),
			"From: %s<%s>\r\n""To: %s<%s>\r\n""Subject: %s\r\n\r\n""%s\r\n.\r\n",
			from_name,account,to_name,email,subject,body);
	ret = send(sockfd,sbuf,len,0);
	if(ret == SOCKET_ERROR)
		return -25;
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);	
	if(ret <= 0)	
		return -25;
	if(strncmp(rbuf,"250",3) != 0)
		return -27;	
		
	// QUIT
	len = snprintf(sbuf,sizeof(sbuf),"QUIT\r\n");
	ret = send(sockfd,sbuf,len,0);
	if(ret == SOCKET_ERROR)
		return -28;
	memset(rbuf,0,sizeof(rbuf));
	ret = recv(sockfd,rbuf,sizeof(rbuf),0);	
	if(ret <= 0)	
		return -29;
	if(strncmp(rbuf,"221",3) != 0)
		return -30;		
		
	return 0;
};

