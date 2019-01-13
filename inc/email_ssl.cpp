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
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../app/app_config.h"
#include "base64.h"

#define FAIL -1






SOCKET_OS OpenConnection(const char *smtp,	unsigned short port)
{
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
	SOCKET_OS sockfd = socket(AF_INET, SOCK_STREAM, 0);
//	Socket_RAII Raii(sockfd);

	int ret = connect(sockfd,(struct sockaddr*)(&server_addr),sizeof(server_addr));
	if(ret == -1)
	{
		printf("%s():connect email server failed:%s(errno:%d)\n",__func__,strerror(errno),errno);
		return -1;
	}
	return sockfd;
}

SSL_CTX* InitCTX(void)
{
	SSL_METHOD *method;
	SSL_CTX *ctx;
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	method = (SSL_METHOD*)SSLv23_client_method();
	ctx = SSL_CTX_new(method);
	if(ctx == NULL)
	{
		ERR_print_errors_fp(stderr);
		printf("Error:%s\n",stderr);
		abort();
	}
	return ctx;
}

void ShowCerts(SSL* ssl)
{
	X509 *cert = SSL_get_peer_certificate(ssl);
	if(cert != NULL)
	{
		printf("Server certificates:\n");
		char *line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("Subjects: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("Issuer: %s\n", line);
		free(line);
		X509_free(cert);
	}else{
		printf("No certificates.\n");
	}
}

int SSL_email(SSL *ssl,
	const char *account,
	const char *password,
	const char* email,
	const char* body,
	const char* from_name,
	const char* to_name,
	const char* subject )
{
	char buf[1024];	 //recv buffer
	char sbuf[1500]; //send buffer
	int ret = 0;
	int len = 0;
	
	int num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"220",3) != 0)
		return -1;	
		
	// EHLO
	len = snprintf(sbuf,sizeof(sbuf),"EHLO HYL-PC\r\n");
	ret = SSL_write(ssl, sbuf, len);
	num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"250",3) != 0)
		return -2;
	
	// AUTH LOGIN
	len = snprintf(sbuf,sizeof(sbuf),"AUTH LOGIN\r\n");
	ret = SSL_write(ssl, sbuf, len);
	num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"334",3) != 0)
		return -3;
	buf[0] = 0; //make different from next 334 code
	
	//USER
	len = snprintf(sbuf,sizeof(sbuf),"%s",account);
	char login[128];
	memset(login,0,sizeof(login));
	EncodeBase64(login,sbuf,len);
	len = snprintf(sbuf,sizeof(sbuf),"%s\r\n",login);
	ret = SSL_write(ssl, sbuf, len);
	num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"334",3) != 0)
		return -4;
	
	//PASSWORD
	len = snprintf(sbuf,sizeof(sbuf),"%s",password);		
	memset(login,0,sizeof(login));
	EncodeBase64(login,sbuf,len);
	len = snprintf(sbuf,sizeof(sbuf),"%s\r\n",login);
	ret = SSL_write(ssl, sbuf, len);
	num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"235",3) != 0)
		return -5;
	
	// MAIL FROM
	len = snprintf(sbuf,sizeof(sbuf),"MAIL FROM:<%s>\r\n",account);
	ret = SSL_write(ssl, sbuf, len);
	num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"250",3) != 0)
		return -6;
	buf[0] = 0; //make different from next 250 code
	
	// RCPT TO
	len = snprintf(sbuf,sizeof(sbuf),"RCPT TO:<%s>\r\n",email);
	ret = SSL_write(ssl, sbuf, len);
	num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"250",3) != 0)
		return -7;
	
	// DATA 准备开始发送邮件内容
	len = snprintf(sbuf,sizeof(sbuf),"DATA\r\n");
	ret = SSL_write(ssl, sbuf, len);
	num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"354",3) != 0)
		return -8;
	
	// 发送邮件内容，\r\n.\r\n内容结束标记
	len = snprintf(sbuf,sizeof(sbuf),
			"From: %s<%s>\r\n""To: %s<%s>\r\n""Subject: %s\r\n\r\n""%s\r\n.\r\n",
			from_name,account,to_name,email,subject,body);
	ret = SSL_write(ssl, sbuf, len);
	num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"250",3) != 0)
		return -9;
	
	// QUIT
	len = snprintf(sbuf,sizeof(sbuf),"QUIT\r\n");
	ret = SSL_write(ssl, sbuf, len);
	num = SSL_read(ssl, buf, sizeof(buf));
	buf[num] = 0;
	printf("SSL_read %d bytes: %s\n", num, buf);
	if(strncmp(buf,"221",3) != 0)
		return -10;
	return 0;
}

int SendEmail_SSL(
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
	int ret = 0;
	SSL_library_init();
	SSL_CTX *ctx = InitCTX();
	SOCKET_OS sockServer = OpenConnection(smtp, port);
	SSL *ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sockServer);
	if(SSL_connect(ssl) == FAIL)
	{
		printf("SSL_connect failed:%s\n", stderr);
		ERR_print_errors_fp(stderr);
		ret = -100;
	}else{
		printf("SSL_connect success\n");
		printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
		ShowCerts(ssl);
		ret = SSL_email(ssl, account, password, email,
									body, from_name, to_name, subject);
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}
	close(sockServer);
	SSL_CTX_free(ctx);
	return ret;
	
}
