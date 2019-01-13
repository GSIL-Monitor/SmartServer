#pragma once
#include <cstring>

#define serial_length			24

class GatewayInfo{
public:
	unsigned int id;	
	char serial[serial_length+1];	
	unsigned char enable;
	unsigned char online;
	
	SOCKET_T gateway_sock;
	
	GatewayInfo()
	{		
		memset(serial,0,sizeof(serial));
		enable = 0;
		online = 0;		
	}
};

