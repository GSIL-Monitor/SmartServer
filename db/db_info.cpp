#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "../app/app_config.h"
#include "../inc/account_info.h"
#include "../inc/device_info.h"
#include "../inc/gateway_info.h"

/**
 * Get account information from a database row
 @param row:reference to a row
 @param info:reference to a UserInfo class object to store the account information
 @return:void
*/
void GetAccountInfo(MYSQL_ROW &row,class AccountInfo &info)
{
	info.id = static_cast<unsigned int>( atoi(row[0]));//id
	
	memset(info.account,0,sizeof(info.account));
	strncpy(info.account,row[1],account_length);//account;
	
	memset(info.password,0,sizeof(info.password));
	strncpy(info.password,row[2],password_length);//copy password
	
	//LogD("sizeof password=%d\n",sizeof(info.password));
	memset(info.nick,0,sizeof(info.nick));	//nick
	strncpy(info.nick,row[3],nick_length);
	
	info.uid = info.id;
	
	memset(info.email,0,sizeof(info.email));
	strncpy(info.email,row[4],email_length);//email
	
	info.admin = static_cast<unsigned char> (atoi(row[5]));//admin
	
	memset(info.serial,0,sizeof(info.serial));
	strncpy(info.serial,row[6],serial_length);	//serial
	
	info.devcnt = static_cast<unsigned short>(atoi(row[7]));	//devcnt
	info.rowcnt = static_cast<unsigned char>(atoi(row[8]));	//rowcnt
	info.enable = static_cast<unsigned char>(atoi(row[9]));//enable
	memcpy(info.rights, row[10], sizeof(info.rights));//rigths
}

void GetDeviceInfo(MYSQL_ROW &row,class DeviceInfo &info)
{
	info.id = static_cast<unsigned int>( atoi(row[0]));//id	
	strncpy(info.serial,row[1],serial_length);	//serial	
	info.devidx = static_cast<unsigned short>(atoi(row[2]));	//devidx
	strncpy(info.address,row[3],address_length);	//address
	strncpy(info.name,row[4],name_length);	//serial
	//memcpy(info.name, row[4], sizeof(info.name));//reg
	//LogD("utf8:%02X %02X %02X\n",info.name[0],info.name[1],info.name[2]);
	info.type = static_cast<unsigned char>(atoi(row[5]));	//type
	info.online = static_cast<unsigned char>(atoi(row[6]));//online
	info.rssi = static_cast<unsigned char>(atoi(row[7]));	//rssi
	info.rnum = static_cast<unsigned char>(atoi(row[8]));//rnum
	info.enable = static_cast<unsigned char>(atoi(row[9]));	//enable	
	memcpy(info.reg, row[10], sizeof(info.reg));//reg
}

void GetGatewayInfo(MYSQL_ROW &row,class GatewayInfo &info)
{
	info.id = static_cast<unsigned int>( atoi(row[0]) );//id
	strncpy(info.serial,row[1],serial_length);//serial
	info.enable = static_cast<unsigned char>( atoi(row[2]) );//enable
	info.online = static_cast<unsigned char>( atoi(row[3]) );//enable
}

