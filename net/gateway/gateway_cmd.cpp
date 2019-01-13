#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <stdexcept>	//invalid_argument
#include "../../app/app_config.h"
#include "../../inc/device_info.h"
#include "../../inc/account_info.h"
#include "../../inc/gateway_info.h"
#include "../../inc/mystring.h"
#include "../../inc/thread_info.h"
#include "gateway_cmd.h"
#include "../../db/db_gateway/db_gateway.h"
#include "../../db/db_user.h"
#include "../../db/db_error.h"
#include "../../inc/log.h"
#include "gateway_raii.h"
#include "../../inc/error.h"
#include "../../app/main.h"
#include "../../net/http/http_post.h"

int get_gateway_position(std::string& strSerial)
{
	int pos = -1;
	int num = static_cast<int>( GatewayInfoVector.size() );
	const char* str = strSerial.c_str();
	for(pos =0; pos<num; pos++)
	{
		auto info = GatewayInfoVector[pos];		
		if(strcmp(info.serial,str) == 0)
			return pos;
	}
	return -1;
}



int gateway_cmd(SOCKET_T gateway_sock,std::string& strCmd,ThreadInfoStruct* pInfo)
{
	std::string strAction = GetStringValue(strCmd,"Action");		
	if(strAction.empty())
		return -1;
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial.empty())
		return -2;
		
	if(pInfo->GatewayInfoPosition == -1)
	{
		pInfo->GatewayInfoPosition = get_gateway_position(strSerial);
		if(pInfo->GatewayInfoPosition == -1)
			return -4;	
		GatewayInfoVector[pInfo->GatewayInfoPosition].gateway_sock = gateway_sock;	
	}		
	
		
	if(strAction == "GetParam" || strAction == "Heart")	
	{
		Gateway_UpdateParam(gateway_sock,strCmd);	//
		return 10;
	}
		
	std::string strUID = GetStringValue(strCmd,"UID");
	if(strUID.empty())
		return -3;		
	
	
	
	if(pInfo->UserInfoPosition == -1)
	{
		pInfo->UserInfoPosition = get_user_position(strUID);
		if(pInfo->UserInfoPosition == -1)
			return -5;
	}
	SOCKET_T user_sock = 	UserInfoVector[pInfo->UserInfoPosition].user_sock;
	
	if(strAction == "MakePair")
		Gateway_MakePair(user_sock,strCmd);
	else if(strAction == "SetMulDevReg")
		Gateway_SetMulDevReg(user_sock,strCmd);
	else if(strAction == "SetParam")
		Gateway_SetParam(user_sock,strCmd);
	else if(strAction == "Rename")
		Gateway_Rename(user_sock,strCmd);
	return 0;
}

int Gateway_UpdateDeviceInfo(std::string& strCmd)
{	
	std::string strAction = GetStringValue(strCmd,"Action");
	//SendACK_RAII Raii(sockfd,strAction);//use RAII to send ACK to gateway;	
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial.empty())
		return -1;	
	
	std::string strDevIndex = GetStringValue(strCmd,"DevIndex");
	std::string strAddress = GetStringValue(strCmd,"Address");
	std::string strName = GetStringValue(strCmd,"Name");
	std::string strResult = GetStringValue(strCmd,"Result");
	std::string strDevType = GetStringValue(strCmd,"DevType");
	std::string strRSSI = GetStringValue(strCmd,"RSSI");
	std::string strRnum = GetStringValue(strCmd,"Rnum");
	std::string strReg = GetStringValue(strCmd,"Reg");
	std::string strEnable = GetStringValue(strCmd,"Enable");
	class DeviceInfo info;
	try{
		//can not use length of sizeof(info.serial),consider the '\0'
		strncpy(info.serial,strSerial.c_str(),serial_length);	
		if(!strDevIndex.empty())
			info.devidx = static_cast<unsigned short>( std::stoi(strDevIndex) );	
		if(!strAddress.empty())
		{			
			strncpy(info.address,strAddress.c_str(),address_length);
			info.address_update = true;
		}
		if(!strName.empty())
		{
			strncpy(info.name,strName.c_str(),name_length);	
			info.name_update = true;	
		}	
		if(!strDevType.empty())	
		{	
			info.type = static_cast<unsigned char>( std::stoi(strDevType) );		
			info.type_update = true;
		}
		if(strResult == "Success")
			info.online = 1;		
		else
			info.online = 0;
		info.online_update = true;
		if(!strRSSI.empty())	
		{	
			info.rssi = static_cast<unsigned char>( std::stoi(strRSSI) );	
			info.rssi_update = true;
		}	
		if((!strRnum.empty()) && (!strReg.empty()))
		{		
			info.rnum = static_cast<unsigned char>( std::stoi(strRnum) );	
			size_t idx;			
			for(int i= 0;i<info.rnum;i++)
			{		
				std::string strHex = strReg.substr(i*2,2);
				//LogD("strHex=%s\n",strHex.c_str());				
				info.reg[i] = static_cast<unsigned char>( std::stoi(strHex,&idx,16) );		
			}
			info.rnum_update = true;
			info.reg_update = true;
		}
		if(strEnable.empty())
			info.enable = 1;
		else
			info.enable = static_cast<unsigned char>( std::stoi(strEnable) );
		info.enable_update = true;
		UpdateDeviceInfo(info);
		return 0;
	}catch(const std::invalid_argument& ia)
	{
		LogE("%s():invalid_argument exception,what()=%s\n",__func__,ia.what());
		return INVALID_ARGUMENT_EXCEPTION;		
	}catch(const std::out_of_range& oor)
	{
		LogE("%s():out_of_rang exception,what()=%s\n",__func__,oor.what());
		return OUT_OF_RANGE_EXCEPTION;
	}catch(const std::bad_alloc& ba)
	{
		LogE("%s():bad_alloc exception,what()=%s\n",__func__,ba.what());
		return BAD_ALLOC_EXCEPTION;
	}	
}

//Action=GetParam*Type=Device*Serial=146123090211*DevIndex=0*Address=0212FCC0*Name=abc*Result=AckTimeout*
//Action=GetParam*Type=Device*Serial=146123090211*DevIndex=0*Address=0212FCC0*Name=ÂÌÉ«¼¼ÊõÊÒ-1ÅÅ*
//       Result=Success*DevType=208*RSSI=162*Rnum=1*Reg=AA*
//Update device params into database
int Gateway_UpdateParam(SOCKET_T gateway_sock,std::string& strCmd)
{	
	std::string strAction = GetStringValue(strCmd,"Action");
	SendACK_RAII Raii(gateway_sock,strAction);//use RAII to send ACK to gateway;	
	return Gateway_UpdateDeviceInfo(strCmd);
}

int Gateway_UpdateParam3(SOCKET_T sockfd,std::string& strCmd)
{	
	std::string strAction = GetStringValue(strCmd,"Action");
	SendACK_RAII Raii(sockfd,strAction);//use RAII to send ACK to gateway;	
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial.empty())
		return -1;	
	
	std::string strDevIndex = GetStringValue(strCmd,"DevIndex");
	std::string strAddress = GetStringValue(strCmd,"Address");
	std::string strName = GetStringValue(strCmd,"Name");
	std::string strResult = GetStringValue(strCmd,"Result");
	std::string strDevType = GetStringValue(strCmd,"DevType");
	std::string strRSSI = GetStringValue(strCmd,"RSSI");
	std::string strRnum = GetStringValue(strCmd,"Rnum");
	std::string strReg = GetStringValue(strCmd,"Reg");
	std::string strEnable = GetStringValue(strCmd,"Enable");
	class DeviceInfo info;
	try{
		//can not use length of sizeof(info.serial),consider the '\0'
		strncpy(info.serial,strSerial.c_str(),serial_length);	
		if(!strDevIndex.empty())
			info.devidx = static_cast<unsigned short>( std::stoi(strDevIndex) );	
		if(!strAddress.empty())
		{			
			strncpy(info.address,strAddress.c_str(),address_length);
			info.address_update = true;
		}
		if(!strName.empty())
		{
			strncpy(info.name,strName.c_str(),name_length);	
			info.name_update = true;	
		}	
		if(!strDevType.empty())	
		{	
			info.type = static_cast<unsigned char>( std::stoi(strDevType) );		
			info.type_update = true;
		}
		if(strResult == "Success")
			info.online = 1;		
		else
			info.online = 0;
		info.online_update = true;
		if(!strRSSI.empty())	
		{	
			info.rssi = static_cast<unsigned char>( std::stoi(strRSSI) );	
			info.rssi_update = true;
		}	
		if((!strRnum.empty()) && (!strReg.empty()))
		{		
			info.rnum = static_cast<unsigned char>( std::stoi(strRnum) );	
			size_t idx;			
			for(int i= 0;i<info.rnum;i++)
			{		
				std::string strHex = strReg.substr(i*2,2);
				//LogD("strHex=%s\n",strHex.c_str());				
				info.reg[i] = static_cast<unsigned char>( std::stoi(strHex,&idx,16) );		
			}
			info.rnum_update = true;
			info.reg_update = true;
		}
		if(strEnable.empty())
			info.enable = 1;
		else
			info.enable = static_cast<unsigned char>( std::stoi(strEnable) );
		info.enable_update = true;
		UpdateDeviceInfo(info);
		return 0;
	}catch(const std::invalid_argument& ia)
	{
		LogE("%s():invalid_argument exception,what()=%s\n",__func__,ia.what());
		return -2;		
	}catch(const std::out_of_range& oor)
	{
		LogE("%s():out_of_rang excpetion,what()=%s\n",__func__,oor.what());
		return -3;
	}catch(const std::bad_alloc& ba)
	{
		LogE("%s():bad_alloc excpetion,what()=%s\n",__func__,ba.what());
		return -4;
	}	
}


int Gateway_Heart(SOCKET_T sockfd,std::string& strCmd)
{	
	return 0;
}

/**
 *Process the gateway cmd MakePair,update the database info and send the cmd to user client
 @Param user_sock:socket of the user client
 @Param strCmd:Cmd from the gateway
 @return:0 for success ,other value for error.
*/
int Gateway_MakePair(SOCKET_T user_sock,std::string& strCmd)
{		
	std::string strResult = GetStringValue(strCmd,"Result");
	if(strResult == "DeviceFull")
	{
		std::string strRet = "{\"Action\":\"MakePair\",\"Result\":\"DeviceFull\"}";
		HttpSend(user_sock,strRet);
		return -1;
	}	
	if(strResult != "Success")
		return -2;	
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial.empty())
		return -3;
	Admin_AddDevice(strSerial);
	Gateway_UpdateDeviceInfo(strCmd);			

	std::string strDevType = GetStringValue(strCmd,"DevType");
	std::string strDevIndex = GetStringValue(strCmd,"DevIndex");
	std::string strName = GetStringValue(strCmd,"Name");
	std::string strRSSI = GetStringValue(strCmd,"RSSI");
	std::string strRnum = GetStringValue(strCmd,"Rnum");
	std::string strReg = GetStringValue(strCmd,"Reg");
	
	std::string strRet("{\"Action\":\"MakePair\",\"Result\":\"Result\"");
	strRet += ",\"DevIndex\":"+strDevIndex+",\"Name\":\""+strName+"\"";
	strRet += ",\"DevType\":"+strDevType+",\"RSSI\":"+strRSSI;
	strRet += ",\"Rnum\":"+strRnum+",\"Reg\":\""+strReg+"\"}";
	return HttpSend(user_sock,strRet);			
}

int Gateway_SetMulDevRegResult(SOCKET_T user_sock,std::string strCmd)
{
	std::string strRet;
	std::string strUID = GetStringValue(strCmd,"UID");
	std::string strResult = GetStringValue(strCmd,"Result");
	std::vector<class DeviceInfo> DevInfos;
	class AccountInfo info;
	int ret = GetUserInfo(strUID,info);
	if(ret != REQUEST_SUCCESS)
	{
		strRet = "{\"Action\":\"SetMulDevReg\",\"Result\":\"GetUserInfoError\"}";
		HttpSend(user_sock,strRet);
		return ret;
	}
	if(info.admin == 1)
		ret = GetAllParam(info.serial, DevInfos);
	else
		ret = GetMultiParam(info.serial, info.rights, info.devcnt, DevInfos);
	if(ret != REQUEST_SUCCESS)
	{
		strRet = "{\"Action\":\"SetMulDevReg\",\"Result\":\"GetDevInfoError\"}";
		HttpSend(user_sock,strRet);
		return ret;
	}
	strRet = "{\"Action\":\"SetMulDevReg\",\"Result\":\"" + strResult + "\",\"DevParam\":[";		
	char str[255];
	std::string strReg;
	for(auto dev:DevInfos)
	{
		snprintf(str,sizeof(str),"{\"DevIndex\":%d,",dev.devidx);
		strRet.append(str);
		if(dev.online == 1)
			snprintf(str,sizeof(str),"%s","\"Result\":\"Success\",");
		else
			snprintf(str,sizeof(str),"%s","\"Result\":\"AckTimeout\",");
		strRet.append(str);
		snprintf(str,sizeof(str),"\"RSSI\":%d,\"Rnum\":%d,", dev.rssi,dev.rnum);
		strRet.append(str);
		HexArrayToString(strReg,dev.reg,dev.rnum);
		snprintf(str,sizeof(str),"\"RegValue\":\"%s\"},",strReg.c_str());
		strRet.append(str);
	}		
	strRet.resize(strRet.size()-1);//delete the last character ','
	strRet.append("]}");
	HttpSend(user_sock,strRet);
	return  0;
}

/**
 *Precess gateway cmd SetMulDevReg,update the device info into database
 @param sockfd:the socket of gateway
 @param strCmd:the cmd form gateway
 @return:0 for success,other value for error;
*/
int Gateway_SetMulDevReg(SOCKET_T user_sock,std::string& strCmd)
{
	std::string strResult = GetStringValue(strCmd,"Result");	
	
	if(strResult == "AllDevSuccess" || strResult == "PartDevSuccess" || strResult == "AllDevFail")
	{
		return Gateway_SetMulDevRegResult(user_sock,strCmd);		
	}
	return Gateway_UpdateDeviceInfo(strCmd);		
}

int Gateway_SetParam(SOCKET_T user_sock,std::string& strCmd)
{

	int ret = Gateway_UpdateDeviceInfo(strCmd);
	std::string strResult = GetStringValue(strCmd,"Result");	
	std::string strDevIndex = GetStringValue(strCmd,"DevIndex");
	if(strResult == "AckTimeout")
	{
		std::string strRet("{\"Action\":\"SetParam\",\"Result\":\"AckTimeout\",\"DevIndex\":");
		strRet += "\""+strDevIndex+"\"}";
		return HttpSend(user_sock,strRet);		
	}
	std::string strRSSI = GetStringValue(strCmd,"RSSI");
	std::string strRnum = GetStringValue(strCmd, "Rnum");
	std::string strReg = GetStringValue(strCmd, "Reg");
	std::string strRet("{\"Action\":\"SetParam\",\"Result\":\"Success\"");
	strRet += ",\"DevIndex\":"+strDevIndex+",\"RSSI\":"+strRSSI;
	strRet += ",\"Rnum\":"+strRnum+",\"RegValue\":\""+strReg+"\"}";
	return HttpSend(user_sock,strRet);
}

int Gateway_Rename(SOCKET_T user_sock,std::string& strCmd)
{
	std::string strResult = GetStringValue(strCmd,"Result");
	if(strResult != "Success")
		return -1;
	int ret = Gateway_UpdateDeviceInfo(strCmd);
	std::string strDevIndex = GetStringValue(strCmd,"DevIndex");
	std::string strName = GetStringValue(strCmd, "Name");
	std::string strRet("{\"Action\":\"Rename\",\"Result\":\"Success\",");
	strRet += "\"DevIndex\":"+strDevIndex+",\"Name\":\""+strName+"\"}";
	return HttpSend(user_sock,strRet);	
}


