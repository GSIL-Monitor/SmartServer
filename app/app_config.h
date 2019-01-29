#pragma once

#define HTTP_SERVER_PORT			8000	//define user server thread tcp port
#define GATEWAY_SERVER_PORT		9000	//define gateway server thread tcp port

#define PLATFORM_LINUX			1	//linux platform
#define PLATFORM_WINDOWS			0	//windows platform

#define USE_LINUX_PTHREAD				1	//use linux pthread to create thread
#define USE_STD_THREAD					0	//use c++ thread to create thread


#define LOG_DEBUG_ENABLE		1 //enable LogD() function
#define LOG_ERROR_ENABLE		1 //enable LogE() function
#define LOG_INFO_ENABLE		1 //enable LogI() function
#define LOG_FILE_ENABLE		1 //enable LogF() function,write log info into a  file


#define LOG_ERROR_FACILITY				131 //error log 16*8+3
#define LOG_INFO_FACILITY				134 //local0 info log 16*8+6
#define LOG_DEBUG_FACILITY				135 //local0 debug log 16*8+7

#define RUN_BACKGROUND		0 //enable the program running in background


//#define MYSQL_SERVER_IP			"172.18.238.206" //My aliyun ECS LAN
//#define MYSQL_SERVER_IP			"39.108.12.199" //My aliyun ECS WAN
#define MYSQL_SERVER_IP			"192.168.108.70" //My company computer
//#define MYSQL_SERVER_IP			"192.168.2.117"	//My home computer
#define MYSQL_SERVER_PORT		3306
#define MYSQL_USER_ID				"root"
#define MYSQL_USER_PWD			"jns12345"
#define MYSQL_DB_ACCOUNT			"smart_account"
#define MYSQL_DB_DEVINFO			"smart_devinfo"
#define MYSQL_DB_GATEWAY			"smart_gateway"

typedef int SOCKET_OS;
typedef int SOCKET_T;		//define the socket type used in linux platform	
#define INVALID_SOCKET		-1
#define SOCKET_ERROR			-1

#define SOCKET_RECV_TIMEOUT		120 //socket recv timeout in seconds
