#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <syslog.h>

#include "../../app/app_config.h"
//#include "../../inc/thread_info.h"
//#include "../../net/http_client.h"
#include "../../inc/log.h"
//#include "../../net/gateway/gateway_server.h"

//unsigned int http_thread_num = 0;
//std::mutex http_thread_num_mutex;
std::atomic<unsigned int> http_thread_num(0);

void * socket_server(unsigned short port,const char *name,void *(*client_fun)(SOCKET_T sockfd))
//void * socket_server(unsigned short port,const char *name,void *(*client_fun)(void* arg))
{
	SOCKET_T sockServer,sockClient;
	struct sockaddr_in server_addr;
	//初始化SOCKET
	sockServer = socket(AF_INET, SOCK_STREAM, 0);
	if (sockServer == -1)
	{
		LogE("%s(): %s create socket error:%s(errno:%d)\n", __func__,name,strerror(errno), errno);
		return nullptr;
	}
	int on = 1;
	setsockopt(sockServer,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	//初始化
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
	server_addr.sin_port = htons(port);//设置的端口为SERVER_PORT

	//将本地地址绑定到所创建的套接字上  
	if (bind(sockServer, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		LogE("%s bind socket error:%s(errno:%d)\n", name,strerror(errno), errno);
		return nullptr;
	}
	//开始监听是否有客户端连接
	if (listen(sockServer, 10) == -1) {
		LogE("%s listen socket error:%s(errno:%d)\n", name,strerror(errno), errno);
		return nullptr;
	}
	//LogI("===== %s started, port:%d ======\n",name,port);
	syslog(LOG_INFO_FACILITY, "===== %s started, port:%d ======\n",name,port);
	//pthread_mutex_init(&mutex, NULL);
	while (1)
	{
		sockClient = accept(sockServer, (struct sockaddr *)NULL, NULL);
		if (sockClient == -1) {
			LogE("%s accept socket error:%s(errno:%d)\n", name, strerror(errno), errno);
			continue;
		}		
		//SOCKET_T *sockfd = new SOCKET_T;
		//*sockfd = sockClient;
		//printf("socket=%d\n",sockClient);
		//std::thread th(client_fun,static_cast<void*>(sockfd));
		std::thread th(client_fun,sockClient);
		th.detach();		
	}
}
