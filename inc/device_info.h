#pragma once

#define serial_length			24
#define address_length		8
#define name_length			32
#define reg_length				64

class DeviceInfo{
public:
	unsigned int id;	
	char serial[serial_length+1];	
	unsigned short devidx; //device index	
	char address[address_length+1];
	char name[name_length+1];	
	unsigned char type;
	unsigned char online;
	unsigned char rssi;
	unsigned char rnum;
	unsigned char enable;
	unsigned char reg[reg_length];	
	//
	bool serial_update;
	bool devidx_update;
	bool address_update;
	bool name_update;
	bool type_update;
	bool online_update;
	bool rssi_update;
	bool rnum_update;
	bool enable_update;
	bool reg_update;
	
	DeviceInfo()
	{		
		memset(serial,0,sizeof(serial));
		memset(address,0,sizeof(address));
		memset(name,0,sizeof(name));
		memset(reg,0,sizeof(reg));
		
		serial_update = false;
		devidx_update = false;
		address_update = false;
		name_update = false;
		type_update = false;
		online_update = false;
		rssi_update = false;
		rnum_update = false;
		enable_update = false;
		reg_update = false;
	}
};

class DevInfoUpdateCtrl{
public:
	
	DevInfoUpdateCtrl()
	{
		
	}
};

#define DEVICE_TYPE_LAMP1	0xD0



