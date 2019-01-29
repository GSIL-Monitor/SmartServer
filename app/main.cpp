#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <signal.h>
#include <syslog.h>
#include "app_config.h"
//#include "../net/socket_server.h"
//#include "../net/http_server.h"
#include "../net/gateway/gateway_client.h"
#include "../net/http/http_client.h"
#include "../net/inc/socket_server.h"
#include "../net/http/http_server.h"
//#include "http_server.h"
#include "../inc/log.h"
#include "../inc/account_info.h"
#include "../inc/gateway_info.h"
#include "../db/db_error.h"
#include "../db/db_user.h"
#include "../db/db_gateway/db_gateway.h"
#include "../inc/md5.h"
#include "../inc/function.h"
#include "../inc/conf_info.h"

std::vector<class AccountInfo> UserInfoVector;
std::vector<class GatewayInfo> GatewayInfoVector;

//using namespace std;

/*
void* thread_http_server(void *arg)
{
	//return create_http_thread_linux(HTTP_SERVER_PORT);
	return socket_server(HTTP_SERVER_PORT,"http server",http_client_thread);
}
void* thread_gateway_server(void *arg)
{
	return socket_server(GATEWAY_SERVER_PORT,"gateway server",gateway_client_thread);
	//return gateway_server_start(GATEWAY_SERVER_PORT);	
}*/

void* thread_cmd_input(void *arg)
{
	while(1)
	{
		std::cout<<"input command:";
		std::string str;
		std::cin>>str;
		if(str == "exit")
		{
			std::cout<<"the program will be exit"<<std::endl;
			//close(socket_fd);
			exit(0);
		}else if(str == "num"){
			std::cout<<"thread num = "<<http_thread_num.load(std::memory_order_relaxed)<<std::endl;
		}else
			std::cout<<"invalid command"<<std::endl;
		sleep(1);
	}
}

void* test(int a)
{
	LogD("test(): a=%d\n",a);
	return nullptr;
}

void* start_thread(void* arg)
{
	int ret  =CreateUserInfoVector(UserInfoVector);
	if(ret != REQUEST_SUCCESS)
	{
		//LogE("%s():CreateUserInfoVector failed,error=%d\n",__func__,ret);
		syslog(LOG_ERROR_FACILITY, "%s():CreateUserInfoVector failed,error=%d\n",__func__,ret);
		return nullptr;
	}
	//LogD("CreateUserInfoVector success.\n");
	syslog(LOG_DEBUG_FACILITY, "CreateUserInfoVector success.\n");
	ret = CreateGatewayInfoVector(GatewayInfoVector);
	if(ret != REQUEST_SUCCESS)
	{
		LogE("%s():CreateGatewayInfoVector failed,error=%d\n",__func__,ret);
		return nullptr;
	}
	//LogD("CreateGatewayInfoVector success.\n");
	syslog(LOG_DEBUG_FACILITY, "CreateGatewayInfoVector success.\n");
	std::thread http_thread(socket_server,HTTP_SERVER_PORT,"http server",http_client_thread);
   //	std::thread gateway_thread(thread_gateway_server,nullptr);
   	std::thread gateway_thread(socket_server,GATEWAY_SERVER_PORT,"gateway server",gateway_client_thread);
   	sleep(2);
   //	std::thread cmd_thread(thread_cmd_input,nullptr);
   std::thread http_server(http_server_start, nullptr);
   
   	   	
   http_thread.join();
   gateway_thread.join();
   http_server.join();
   //cmd_thread.join();
   return nullptr;
}

void handler(int sig)
{
	//LogI("I got a signal %d, I'm quitting.\n",sig);
	syslog(LOG_INFO_FACILITY, "I got a signal %d, I'm quitting.\n",sig);
	exit(0);
}

void test_func()
{
/* test md5
	char str[]="123456";
   std::string strMD5;
   GetMD5(str,strMD5);
   LogD("str= %s,strMD5= %s\n",str,strMD5.c_str());
*/
/* test random data
	for(int i=0;i<20;i++)
	{
		unsigned int a = GetRandomData(100000,999999);
		usleep(100000);
		LogD("random data a= %06d\n",a);
	}
*/	
/*
	std::string strVersion;
	int ret = GetKeyValue(std::string("/home/chao/projects/SmartServer/bin/setting.conf"),
					std::string("update"),std::string("version"),strVersion);
	if(ret < 0)
	{
		LogE("read conf file key value failed\n");
	}
*/
	
}

void run_background(void)
{
	if(daemon(1, 1) == -1)
	{
		LogE("daemon error\n");
		exit(1);
	}
	struct sigaction act;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if(sigaction(SIGQUIT, &act, NULL))
	{
		LogE("sigaction error\n");
		exit(0);
	}
}
//#define LogII(X,X) s
int main()
{
	//setbuf(stdout, NULL);
   syslog(LOG_INFO_FACILITY, "SmartServer start...!\n");
   //LogI("SmartServer start...!\n");
#if RUN_BACKGROUND
	run_background();
#endif   
   test_func();
   
   //	std::thread th(test,128);
	std::thread start(start_thread,nullptr);
	start.join();
   //std::thread http_thread(thread_http_server,nullptr);
   
   exit(0);
}
