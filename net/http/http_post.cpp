#include <stdio.h>
//#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
//#include <errno.h>
#include <vector>
#include <unistd.h>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <thread>
#include <syslog.h>
#include "../../app/app_config.h"
#include "../../inc/mystring.h"
#include "../../inc/thread_info.h"
#include "../../db/db_user.h"
#include "../../db/db_error.h"
#include "../../inc/account_info.h"
#include "../../inc/device_info.h"
#include "../../inc/gateway_info.h"
#include "../../net/http/http_post.h"
#include "../../inc/log.h"
#include "../../net/gateway/gateway_cmd.h"
#include "../../app/main.h"
#include "../../inc/email.h"
#include "../../inc/conf_info.h"
#include "../../inc/email_ssl.h"

int HttpSend(SOCKET_T sockfd,const char *buf,int len,int flags)
{
	char str[255];	
	int strlen = snprintf(str,sizeof(str),
					"HTTP/1.1 200 OK\r\nConnection:Keep-Alive\r\nContent-Length:%d\r\n\r\n",len);
	std::string strRet(str);
	strRet.append(buf);
	
	//LogD("Http send:%s\n",str);
	//sleep(60);
	int ret = send(sockfd,strRet.c_str(),strRet.size(),flags);
	syslog(LOG_INFO_FACILITY, "ThreadID=0x%X, HttpSend(%d bytes):%s\n",std::this_thread::get_id(),
			ret, strRet.c_str());
	return ret;
}

int HttpSend(SOCKET_T sockfd,std::string& strData,int flags)
{
	char str[255];	
	int strlen = snprintf(str,sizeof(str),
						"HTTP/1.1 200 OK\r\nConnection:Keep-Alive\r\nContent-Length:%d\r\n\r\n",strData.size());
	std::string strRet(str);
	strRet.append(strData);
	int ret = send(sockfd,strRet.c_str(),strRet.size(),flags);
	syslog(LOG_INFO_FACILITY, "ThreadID=0x%X, HttpSend(%d bytes):%s\n",std::this_thread::get_id(),
			ret, strRet.c_str());
	return ret;
}

int SendToGateway(SOCKET_T sockfd,std::string& strCmd)
{
	return send(sockfd,strCmd.c_str(),strCmd.size(),0);
}

int get_user_position(std::string& strUID)
{
	int pos = -1;
	unsigned int id = 0;
	int num = static_cast<int>( UserInfoVector.size() );
	try{
		id= std::stoi(strUID);
		for(pos = 0; pos<num; pos++)
		{
			if(id == UserInfoVector[pos].id)
				return pos;
		}		
	}catch(const std::invalid_argument& ia)
	{
		LogE("%s():invalid_argument exception,what()=%s\n",__func__,ia.what());
		return -1;
	}catch(const std::out_of_range& oor)
	{
		LogE("%s():out_of_range exception,what()=%s\n",__func__,oor.what());
		return -2;
	}	
	return -3;
}

int HttpPost(SOCKET_T sockfd,const char *pBody,ThreadInfoStruct *pInfo)
{	
	std::string strCmd(pBody);	
	std::string strAction = GetStringValue(strCmd,"Action");
	std::string strToken = GetStringValue(strCmd,"Token");
	//LogD("Token=%s\n",strToken.c_str());
	
	
	if(strAction=="CheckUpdate")
	{
		Http_CheckUpdate(sockfd,strCmd);
		return 0;
	}
			
	if(strToken.empty())
	{
		//process the cmd dose not need a token 
		
		if(strAction=="Login")	
			Http_UserLogin(sockfd,strCmd,pInfo);
		else if(strAction=="Register")	
			Http_UserRegister(sockfd,strCmd,pInfo);				
		else if(strAction=="ForgetPassword")
			Http_ForgetPassword(sockfd,strCmd);		
		return -1;
	}	
	
	std::string strUID = GetStringValue(strCmd,"UID");
	if(strUID.empty())
		return -2;
	if(pInfo->UserInfoPosition == -1)
	{
		pInfo->UserInfoPosition = get_user_position(strUID);
		if(pInfo->UserInfoPosition == -1)
			return -3;	
		UserInfoVector[pInfo->UserInfoPosition].user_sock = sockfd;	
	}
	if(strToken != UserInfoVector[pInfo->UserInfoPosition].token)
	{
		std::string strRet = "{\"Action\":\""+strAction+"\",\"Result\":\"TokenError\"}";
		HttpSend(sockfd, strRet);
		return 0;
	}
	
	//process the cmd must need a token
	if(strAction=="AddUser")
		Http_AddUser(sockfd,strCmd,pInfo);
	else if(strAction=="DeleteUser")
		Http_DeleteUser(sockfd,strCmd,pInfo);
	else if(strAction=="GetUser")
		Http_GetUser(sockfd,strCmd,pInfo);
	else if(strAction=="GetRights")
		Http_GetRights(sockfd,strCmd);
	else if(strAction=="SetRights")
		Http_SetRights(sockfd,strCmd);
	else if(strAction=="ModifyPwd")
		Http_ModifyPwd(sockfd,strCmd);
	else if(strAction=="CheckEmail")
		Http_CheckEmail(sockfd,strCmd);
	else if(strAction=="AddEmail")
		Http_AddEmail(sockfd,strCmd);
	else if(strAction=="GetMultiParam")
		Http_GetMultiParam(sockfd,strCmd);
	else if(strAction=="GetAllParam")
		Http_GetAllParam(sockfd,strCmd);
		
	
	
	SOCKET_T user_sock = sockfd;
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial.empty())
		return -4;
	if(pInfo->GatewayInfoPosition == -1)
	{
		pInfo->GatewayInfoPosition = get_gateway_position(strSerial);
		if(pInfo->GatewayInfoPosition == -1)
			return -5;
		//GatewayInfoVector[pInfo->GatewayInfoPosition].gateway_sock = 
	}
	SOCKET_T gateway_sock = GatewayInfoVector[pInfo->GatewayInfoPosition].gateway_sock;
	
	if(strAction == "SetParam")	
		Http_CmdToGateway(gateway_sock,user_sock,strCmd);		
	else if(strAction == "MakePair")
		Http_CmdToGateway(gateway_sock,user_sock,strCmd);
	else if(strAction == "SetMulDevReg" || strAction == "SetAllDevReg")
		Http_CmdToGateway(gateway_sock,user_sock,strCmd);
	else if(strAction == "Rename")
		Http_CmdToGateway(gateway_sock,user_sock,strCmd);
	else if(strAction == "DeleteDevice")
		Http_CmdToGateway(gateway_sock,user_sock,strCmd);
	return 0;
}

int Http_UserRegister(SOCKET_T sockfd,std::string &strCmd,ThreadInfoStruct *pInfo)
{
	char strRet[1024];
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount=="")
		return -1;
	std::string strPassword = GetStringValue(strCmd,"Password");
	if(strPassword=="")
		return -2;
	std::string strNick = GetStringValue(strCmd,"Nick");
	if(strNick=="")
		return -3;
	std::string strSerial = GetStringValue(strCmd,"Serial");
	std::string strEmail = GetStringValue(strCmd,"Email");
	int ret =0;
	/*
	if(strSerial=="")
	{
		//register a user account
		ret = User_Register(strAccount.c_str(),strPassword.c_str(),strNick.c_str());		
	}else
	{
		//register a admin account
		ret = User_Register(strAccount.c_str(),strPassword.c_str(),strNick.c_str(),strSerial.c_str());
	}*/
	ret = User_Register(strAccount.c_str(),strPassword.c_str(),strNick.c_str(),strSerial.c_str(),strEmail.c_str());
	if(ret == REQUEST_SUCCESS)	
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"Register\",\"Result\":\"Success\"}")	;
	else if(ret == ACCOUNT_EXIST)
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"Register\",\"Result\":\"AccountExist\"}")	;
	else
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"Register\",\"Result\":\"Failed\",\"ErrorCode\":%d}",ret)	;
	return HttpSend(sockfd,strRet,strlen(strRet),0);	;
}

int GetRightsString(std::string &strRights,const unsigned char *pbuf,int len)
{
	char str[3];
	for(int i=0;i<len;i++)
	{
		snprintf(str,sizeof(str),"%02X",*(pbuf+i));
		strRights.append(str);
	}
	return 0;
}

int GetRightsString_OldVersion(std::string &strRights,const unsigned char *pbuf,int len)
{
	char str[3];
	if(len ==0)
		return 0;
	strRights.append("*");
	for(int i=0;i<len;i++)
	{
		snprintf(str,sizeof(str),"%d*",*(pbuf+i));
		strRights.append(str);
	}
	return 0;
}
/*
int RightsToString(unsigned char devcnt,onst unsigned char *rights,std::string &strRights)
{
	char str[3];
	for(int i=0;i<devcnt;i++)
	{
		snprintf(str,sizeof(str),"%02X",*(pbuf+i));
		strRights.append(str);
	}
	return 0;
}
*/

int Http_UserLogin(SOCKET_T sockfd,std::string & strCmd,ThreadInfoStruct *pInfo)
{
	char strRet[1024];
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount=="")
		return -1;
	std::string strPassword = GetStringValue(strCmd,"Password");
	if(strPassword=="")
		return -2;
	class AccountInfo Account;
	int ret = User_Login(strAccount.c_str(),strPassword.c_str(), Account);
	if(ret == REQUEST_SUCCESS)
	{
		//int len=0;
		int lenAll=0;
		lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,
						"{\"Action\":\"Login\",\"Result\":\"Success\",\"Token\":\"%s\"",
						Account.token);	
						
		std::string str("");	
		if(strlen(Account.serial)==0)
			str.append("NULL");
		else
			str.append(Account.serial);
		lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,
					",\"Serial\":\"%s\"",str.c_str());
		
		std::string strAdmin;
		if(Account.admin==1)
			strAdmin = "admin";
		else
			strAdmin = "common";
		lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,
				",\"User_type\":\"%s\",\"Admin\":%d",strAdmin.c_str(),Account.admin);
		std::string strRights;
		GetRightsString_OldVersion(strRights,Account.rights,Account.devcnt);
		std::string strRightsX;
		GetRightsString(strRightsX,Account.rights,Account.devcnt);
		lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,
				",\"Nick\":\"%s\",\"UID\":%u,\"Devcnt\":%d,\"Rowcnt\":%d,\"Rights\":\"%s\",\"RightsX\":\"%s\"}",
				Account.nick,Account.uid,Account.devcnt,Account.rowcnt,strRights.c_str(),strRightsX.c_str());	
		//---------------------------------------------------------------------
		char str2[13];
		snprintf(str2, sizeof(str2), "%d", Account.id);		
		std::string strUID(str2);
		if(pInfo->UserInfoPosition == -1)
		{
			pInfo->UserInfoPosition = get_user_position(strUID);
			if(pInfo->UserInfoPosition == -1)
				return -3;	
			UserInfoVector[pInfo->UserInfoPosition].user_sock = sockfd;	
		}
		strncpy(UserInfoVector[pInfo->UserInfoPosition].token, Account.token, sizeof(Account.token));
		
		//---------------------------------------------------------------------	
	}else if(ret == USER_NOT_EXIST)
	{
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"Login\",\"Result\":\"UserNotExist\"}");		
	}else if(ret == PASSWORD_ERROR)
	{
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"Login\",\"Result\":\"PasswordError\"}");		
	}else if(ret == ACCOUNT_DISABLE)
	{
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"Login\",\"Result\":\"AccountDisable\"}");		
	}else
	{
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"Login\",\"Result\":\"Failed\",\"ErrorCode\":%d}",ret);		
	}
	return HttpSend(sockfd,strRet,strlen(strRet),0);	
}

int Http_AddUser(SOCKET_T sockfd,std::string &strCmd,ThreadInfoStruct *pInfo)
{	
	char strRet[1024];
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount=="")
		return -1;
	//unsigned int id = static_cast<unsigned int>( atoi( strId.c_str() ) )  ;
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial=="")
		return -2;
	//class AccountInfo Account;
	int ret = AddUser(strAccount.c_str(),strSerial.c_str());
	if(ret == REQUEST_SUCCESS)
		snprintf(strRet,sizeof(strRet),
				"{\"Action\":\"AddUser\",\"Result\":\"Success\",\"Account\":\"%s\"}",strAccount.c_str());
	else if(ret == USER_IS_ADMIN)
		snprintf(strRet,sizeof(strRet),
				"{\"Action\":\"AddUser\",\"Result\":\"UserIsAdmin\",\"Account\":\"%s\"}",strAccount.c_str());
	else if(ret == USER_NOT_EXIST)
		snprintf(strRet,sizeof(strRet),
				"{\"Action\":\"AddUser\",\"Result\":\"UserNotExist\",\"Account\":\"%s\"}",strAccount.c_str());
	else if(ret == USER_IS_MEMBER)
		snprintf(strRet,sizeof(strRet),
				"{\"Action\":\"AddUser\",\"Result\":\"UserExist\",\"Account\":\"%s\"}",strAccount.c_str());
	else if(ret == USER_BELONG_OTHER)
		snprintf(strRet,sizeof(strRet),
				"{\"Action\":\"AddUser\",\"Result\":\"UserBelongOther\",\"Account\":\"%s\"}",strAccount.c_str());
	else
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"AddUser\",\"Result\":\"Failed\",\"ErrorCode\":%d}",ret);
	return HttpSend(sockfd,strRet,strlen(strRet),0);	
}


/**
 * Process the DeleteUser Cmd from Http
*/
int Http_DeleteUser(SOCKET_T sockfd,std::string &strCmd,ThreadInfoStruct *pInfo)
{
	char strRet[1024];
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount == "")
		return -1;
	int ret = DeleteUser(strAccount.c_str());
	if(ret== REQUEST_SUCCESS)
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"DeleteUser\",\"Result\":\"Success\"}");
	else
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"DeleteUser\",\"Result\":\"Failed\",\"ErrorCode\":%d}",ret);
	return HttpSend(sockfd,strRet,strlen(strRet),0);	
}


int Http_GetUser(SOCKET_T sockfd,std::string &strCmd,ThreadInfoStruct *pInfo)
{	
	//LogD("Http GEtUser\n");
	//char strRet[1024];
	std::string strRet;
	char str[255];
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial=="")
		return -1;	
	vector<class AccountInfo> users;	
	int ret = GetUser(strSerial.c_str(),users);
	if(ret == REQUEST_SUCCESS)
	{
		int lenAll = 0;	
		int len =0;		
		len = snprintf(str,sizeof(str),
						"{\"Action\":\"GetUser\",\"Result\":\"Success\",\"Count\":%d,\"Users\":[",users.size());	
		strRet.append(str);				
		
		//LogD("infos.size()=%d\n",users.size());		
		for(auto user:users)
		{
			len = snprintf(str,sizeof(str),
						"{\"Id\":%d,\"Account\":\"%s\",\"Nick\":\"%s\",",user.id,user.account,user.nick);	
			strRet.append(str);
			len = snprintf(str,sizeof(str),
						"\"Admin\":%d,\"Devcnt\":%d,\"Rowcnt\":%d,\"Enable\":%d,",
						user.admin,user.devcnt,user.rowcnt,user.enable);	
			strRet.append(str);
			std::string strRights;
			GetRightsString_OldVersion(strRights,user.rights,user.devcnt);
			std::string strRightsX;		
			GetRightsString(strRightsX,user.rights,user.devcnt);	
			//lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,
			//			"\"Rights\":\"%s\",\"RightsX\":\"%s\"},",strRights.c_str(),strRightsX.c_str());	
			strRet += "\"Rights\":\""+strRights+"\",\"RightsX\":\""+strRightsX+"\"},";						
		}
		strRet.resize(strRet.size()-1);
		//lenAll --;//remove the last ','
		//lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,"%s","]}");		
		strRet.append("]}");
	}		
	else if(ret == USER_NOT_EXIST)
		strRet = "{\"Action\":\"GetUser\",\"Result\":\"UserNotExist\"}";
	else
	{
		snprintf(str,sizeof(str),"{\"Action\":\"GetUser\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);
		strRet.append(str);
	}
	return HttpSend(sockfd,strRet);	
}

int Http_GetUser_bak(SOCKET_T sockfd,std::string &strCmd,ThreadInfoStruct *pInfo)
{	
	//LogD("Http GEtUser\n");
	char strRet[1024];
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial=="")
		return -1;	
	vector<class AccountInfo> users;	
	int ret = GetUser(strSerial.c_str(),users);
	if(ret == REQUEST_SUCCESS)
	{
		int lenAll = 0;		
		lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,
						"{\"Action\":\"GetUser\",\"Result\":\"Success\",\"Count\":%d,\"Users\":[",users.size());					
		
		//LogD("infos.size()=%d\n",users.size());		
		for(auto user:users)
		{
			lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,
						"{\"Id\":%d,\"Account\":\"%s\",\"Nick\":\"%s\",",user.id,user.account,user.nick);	
			lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,
						"\"Admin\":%d,\"Devcnt\":%d,\"Rowcnt\":%d,\"Enable\",%d,",
						user.admin,user.devcnt,user.rowcnt,user.enable);	
			std::string strRights;
			GetRightsString_OldVersion(strRights,user.rights,user.devcnt);
			std::string strRightsX;		
			GetRightsString(strRightsX,user.rights,user.devcnt);	
			lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,
						"\"Rights\":\"%s\",\"RightsX\":\"%s\"},",strRights.c_str(),strRightsX.c_str());							
		}
		lenAll --;//remove the last ','
		lenAll += snprintf(strRet+lenAll,sizeof(strRet)-lenAll,"%s","]}");		
	}		
	else if(ret == USER_NOT_EXIST)
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"GetUser\",\"Result\":\"UserNotExist\"}");
	else
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"GetUser\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);
	return HttpSend(sockfd,strRet,strlen(strRet),0);	
}

int Http_GetRights(SOCKET_T sockfd,std::string &strCmd)
{
	char strRet[1024];
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount == "")
		return -1;
	class AccountInfo info;
	int ret = GetRights(strAccount.c_str(),info);
	if(ret == REQUEST_SUCCESS)
	{
		if(info.devcnt == 0)		
			snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"GetRights\",\"Result\":\"NoRights\"}");
		else
		{
			std::string strRights;//rights string for the old format
			GetRightsString_OldVersion(strRights,info.rights,info.devcnt);
			
			std::string strRightsNew;			
			GetRightsString(strRightsNew,info.rights,info.devcnt);
			snprintf(strRet,sizeof(strRet),
				"{\"Action\":\"GetRights\",\"Result\":\"Success\",\"Devcnt\":%d,\"Rights\":\"%s\",\"RightsX\":\"%s\"}",
				info.devcnt,strRights.c_str(),strRightsNew.c_str());
		}
	}else		
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"GetRights\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);
	return HttpSend(sockfd,strRet,strlen(strRet),0);
}
/**
 *Convert the rights string from the old format like "*0*1*2*3*" to the new format like "00010203"
 @param devcnt:the user devcnt
 @pram strRightsOld:reference of the source rights string like "*0*1*2*3*"
 @param strRights:reference of the dest rights string
 @return:0 if success,<0 if failed
*/
int RightsFormatConvert(int devcnt,std::string &strRightsOld,std::string &strRights)
{
	std::size_t indexL = 0;
	std::size_t indexR = 0;
	unsigned char right;
	char strHex[3]; 
	for(int i=0;i<devcnt;i++)
	{
		indexL = strRightsOld.find("*",indexL);
		if(indexL == std::string::npos)
			return -1;		
		std::size_t indexR = strRightsOld.find("*",indexL+1);
		if(indexR == std::string::npos)
			return -2;		
		std::string str = strRightsOld.substr(indexL+1,indexR-indexL-1);		
		right = static_cast<unsigned char>(atoi(str.c_str()));
		indexL = indexR;
		snprintf(strHex,sizeof(strHex),"%02X",right);
		strRights.append(strHex);
	}	
	
	return 0;
}

int Http_SetRights(SOCKET_T sockfd,std::string &strCmd)
{
	char strRet[1024];	
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount == "")
		return -1;	
	std::string strDevcnt = GetStringValue(strCmd,"Devcnt");
	if(strDevcnt == "")
		return -2;	
	int devcnt = atoi(strDevcnt.c_str());
	std::string strRights;
	//Get the substr of Rights;
	std::size_t indexL = strCmd.find("Rights=*");
	if(indexL == std::string::npos)
	{
		//new string format like "Rights=0001020304";
		strRights = GetStringValue(strCmd,"Rights");
	}else
	{
		//old string format link "Rights=*0*1*2*3*4*";
		strRights = strCmd.substr(indexL+7);
		std::string strRightsOld(strRights);
		strRights="";	
		RightsFormatConvert(devcnt,strRightsOld,strRights);
	}
	/*
	
	std::size_t indexR = strCmd.find("=",indexL+7);
	if(indexR == std::string::npos)
	{
		strRights = strCmd.substr(indexL+7);
	}else
	{
		indexR = strCmd.find("=",indexL+7);
		strRights = strCmd.substr(indexL+7,indexR-indexL-7);
	}
	
	if(strRights == "")
		return -4;
	LogD("333,strRights=%s\n",strRights.c_str());
	if(strRights.find("*") != std::string::npos)
	{
		//string format like "*0*1*2*3*"
		std::string strRightsOld(strRights);
		strRights="";	
		RightsFormatConvert(devcnt,strRightsOld,strRights);
	}else
	{
		//string format like "00010203"
	}*/	
	int ret = SetRights(strAccount.c_str(),devcnt,strRights.c_str());
	if(ret == REQUEST_SUCCESS)
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"SetRights\",\"Result\":\"Success\"}");
	else
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"SetRights\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);
		
	return HttpSend(sockfd,strRet,strlen(strRet),0);
}


/**
 *Process Modify password for thd account
 @param sockfd:the socket of the http link
 @param strCmd:the cmd in the http packet;
 @return:nums of data send by http
*/
int Http_ModifyPwd(SOCKET_T sockfd,std::string& strCmd)
{
	char strRet[1024];
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount.empty())
		return -1;
	std::string strNewPwd = GetStringValue(strCmd,"NewPwd");
	if(strNewPwd.empty())
		return -2;
	int ret = ModifyPwd(strAccount.c_str(),strNewPwd.c_str());
	if(ret == REQUEST_SUCCESS)
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"ModifyPwd\",\"Result\":\"Success\"}");
	else
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"ModifyPwd\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);
	return HttpSend(sockfd,strRet,strlen(strRet),0);
}


int Http_CheckUpdate(SOCKET_T sockfd,std::string& strCmd)
{
	char strRet[1024];
	char *dir = get_current_dir_name();
	std::string filename(dir);
	filename.append("/SmartServer.conf");
	//HttpSend(sockfd,filename);
	std::string strVersion;
	int ret = GetKeyValue(filename, std::string("update"), std::string("version"), strVersion);
	if(ret < 0)
	{
		return -1;
	}
	char strFile[255];
	snprintf(strFile,sizeof(strFile),"%s/update/SmartHome V%s.apk",dir,strVersion.c_str());
	std::ifstream ifs(strFile);
	if(!ifs.is_open())
	{
		syslog(LOG_DEBUG_FACILITY, "Open update apk file failed, file= %s\n", strFile);
		return -2;
	}
	ifs.seekg(0, ifs.end);
	unsigned long fileLen = static_cast<unsigned long>(ifs.tellg());
	ifs.close();
	
	snprintf(strRet,sizeof(strRet),
		"{\"Action\":\"CheckUpdate\",\"Result\":\"Success\",\"Ver\":\"%s\",\"Size\":%lu}",
		strVersion.c_str(), fileLen);
	return HttpSend(sockfd,strRet,strlen(strRet),0);
}

int Http_ForgetPassword(SOCKET_T sockfd,std::string& strCmd)
{
	char strRet[1024];
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount.empty())
		return -1;
	std::string strEmail = GetStringValue(strCmd,"Email");
	if(strEmail.empty())
		return -2;
	std::string strNewPwd;
	std::string strNick;
	int ret = ForgetPassword(strAccount.c_str(),strEmail.c_str(),strNewPwd,strNick);
	if(ret == REQUEST_SUCCESS)
	{
		std::string body;//(strNick);
		body.append(":您好!\n\n      由于您忘记了登录密码，系统为您生成的随机密码为:");
		body.append(strNewPwd);
		body.append("，请您登录后尽快修改密码。\n\n\n");
		body.append("本邮件由系统自动发出，请勿回复！\n\n广东建科");
		//char strGB2312[1024];
		/*
		char *pstrGB2312 = new (std::nothrow) char[body.size()+1];
		if(pstrGB2312 == nullptr)
			return -3;
		//shared_ptr<char> ptrGB2312 = make_shared<char>[body.size()];
		//memset()
		memset(pstrGB2312,0,body.size()+1);
		charset_convert_UTF8_TO_GB2312((char*)body.c_str(),body.size(),pstrGB2312,body.size()+1);	
		body = strNick + pstrGB2312;
		delete[] pstrGB2312;
		*/
		std::string body_gb2312;
		charset_UTF8_TO_GB2312(body,body_gb2312);
		body = strNick + body_gb2312;
		
		std::string from_name_utf8="广东建科";
		std::string from_name_gb2312;
		charset_UTF8_TO_GB2312(from_name_utf8,from_name_gb2312);
		/*
		char *ptr_from_name_gb2312 = new (std::nothrow) char[from_name_utf8.size()+1];
		if(ptr_from_name_gb2312 == nullptr)
			return -4;		
		int a = sizeof(ptr_from_name_gb2312[]);
		memset(ptr_from_name_gb2312,0,from_name_utf8.size()+1);
		charset_convert_UTF8_TO_GB2312((char*)from_name_utf8.c_str(),from_name_utf8.size(),
						ptr_from_name_gb2312,from_name_utf8.size()+1);	
		std::string from_name(ptr_from_name_gb2312);
		delete[] ptr_from_name_gb2312;
		*/
		
		std::string subject_utf8="找回密码";
		std::string subject_gb2312;
		charset_UTF8_TO_GB2312(subject_utf8,subject_gb2312);
		/*
		char *ptr_subject_gb2312 = new (std::nothrow) char[subject_utf8.size()+1];
		if(ptr_subject_gb2312 == nullptr)
			return -5;		
		memset(ptr_subject_gb2312,0,subject_utf8.size()+1);
		charset_convert_UTF8_TO_GB2312((char*)subject_utf8.c_str(),subject_utf8.size(),
						ptr_subject_gb2312,subject_utf8.size()+1);	
		std::string subject(ptr_subject_gb2312);
		delete[] ptr_subject_gb2312;	
		*/
		/*
		ret = SendEmail("smtp.exmail.qq.com",25,"taoxiaochao@gdjky.com","Jns12345",
					strEmail.c_str(),body.c_str(),from_name_gb2312.c_str(),
					strNick.c_str(),subject_gb2312.c_str());
					*/
		ret = SendEmail_SSL("smtp.exmail.qq.com",465,"taoxiaochao@gdjky.com","Jns12345",
					strEmail.c_str(),body.c_str(),from_name_gb2312.c_str(),
					strNick.c_str(),subject_gb2312.c_str());
		snprintf(strRet,sizeof(strRet),
				"{\"Action\":\"ForgetPassword\",\"Result\":\"Success\",\"Email\":\"%s\"}",strEmail.c_str());	
		
	}
	else if(ret == EMAIL_ERROR)
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"ForgetPassword\",\"Result\":\"EmailError\"}");
	else if(ret == USER_NOT_EXIST)
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"ForgetPassword\",\"Result\":\"AccountError\"}");
	else
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"ForgetPassword\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);
	return HttpSend(sockfd,strRet,strlen(strRet),0);
}

int Http_CheckEmail(SOCKET_T sockfd,std::string& strCmd)
{
	char strRet[1024];	
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount.empty())
		return -1;
	class AccountInfo info;
	int ret = CheckEmail(strAccount.c_str(),info);
	if(ret == REQUEST_SUCCESS)
		snprintf(strRet,sizeof(strRet),
			"{\"Action\":\"CheckEmail\",\"Result\":\"Success\",\"Email\":\"%s\"}",info.email);	
	else if(ret == EMAIL_EMPTY)
		snprintf(strRet,sizeof(strRet),"%s","{\"Action\":\"CheckEmail\",\"Result\":\"NoEmail\"}");
	else
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"ForgetPassword\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);	
	return HttpSend(sockfd,strRet,strlen(strRet),0);
}

int Http_AddEmail(SOCKET_T sockfd,std::string& strCmd)
{
	char strRet[1024];	
	std::string strAccount = GetStringValue(strCmd,"Account");
	if(strAccount.empty())
		return -1;
	std::string strEmail = GetStringValue(strCmd,"Email");
	if(strEmail.empty())
		return -2;	
	int ret = AddEmail(strAccount.c_str(),strEmail.c_str());
	if(ret == REQUEST_SUCCESS)
		snprintf(strRet,sizeof(strRet),"%s",	"{\"Action\":\"AddEmail\",\"Result\":\"Success\"}");		
	else
		snprintf(strRet,sizeof(strRet),"{\"Action\":\"AddEmail\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);	
	return HttpSend(sockfd,strRet,strlen(strRet),0);
}

//strRights=&0&1&2&3&4&
int GetRightsFromHttpCmd(std::string& strRights,unsigned char *rights,int cnt)
{
	std::size_t indexL = 0;
	std::size_t indexR = 0;
	unsigned char right;
	char strHex[3]; 
	for(int i=0;i<cnt;i++)
	{
		indexL = strRights.find("&",indexL);
		if(indexL == std::string::npos)
			return -1;		
		std::size_t indexR = strRights.find("&",indexL+1);
		if(indexR == std::string::npos)
			return -2;		
		std::string str = strRights.substr(indexL+1,indexR-indexL-1);		
		right = static_cast<unsigned char>(atoi(str.c_str()));
		indexL = indexR;
		//snprintf(strHex,sizeof(strHex),"%02X",right);
		//strRights.append(strHex);
		*(rights+i) = right;
	}	
	
	return 0;
}



int Http_GetMultiParam(SOCKET_T sockfd,std::string& strCmd)
{
	std::string strRet;
	char str[255];		
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial.empty())
		return -1;	
	std::string strCnt = GetStringValue(strCmd,"Cnt");
	if(strCnt.empty())
		return -2;	
	std::string strRights = GetStringValue(strCmd,"Rights");
	if(strRights.empty())
		return -3;	
	int cnt = std::stoi(strCnt);
	unsigned char rights[255];
	GetRightsFromHttpCmd(strRights,rights,cnt);
	std::vector<class DeviceInfo> infos;
	infos.reserve(cnt);
	std::string strReg;	
	int ret = GetMultiParam(strSerial.c_str(),rights,cnt,infos);	
	if(ret == REQUEST_SUCCESS)		
	{
		snprintf(str,sizeof(str),"%s",	"{\"Action\":\"GetMultiParam\",\"Result\":\"Success\",\"DevParam\":[");		
		strRet.append(str);
		for(auto info:infos)
		{
			snprintf(str,sizeof(str),"{\"DevIndex\":%d,",info.devidx);
			strRet.append(str);
			if(info.online == 1)
				snprintf(str,sizeof(str),"%s","\"Result\":\"Success\",");
			else
				snprintf(str,sizeof(str),"%s","\"Result\":\"AckTimeout\",");
			strRet.append(str);
			snprintf(str,sizeof(str),"\"Name\":\"%s\",\"DevType\":%d,\"RSSI\":%d,\"Rnum\":%d,",
			info.name,info.type,info.rssi,info.rnum);
			strRet.append(str);
			HexArrayToString(strReg,info.reg,info.rnum);
			snprintf(str,sizeof(str),"\"RegValue\":\"%s\"},",strReg.c_str());
			strRet.append(str);
		}
		//strRet.erase(strRet.size()-1,1);//delete the last ','
		strRet.resize(strRet.size()-1);//delete the last character ','
		strRet.append("]}");
	}
	else
	{
		snprintf(str,sizeof(str),"{\"Action\":\"GetMultiParam\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);	
		strRet.append(str);
	}
	return HttpSend(sockfd,strRet,0);	
}

int Http_GetAllParam(SOCKET_T sockfd,std::string& strCmd)
{
	std::string strRet;
	char str[255];	
	std::string strSerial = GetStringValue(strCmd,"Serial");
	if(strSerial.empty())
		return -1;		
	std::vector<class DeviceInfo> infos;	
	std::string strReg;
	int ret = GetAllParam(strSerial.c_str(),infos);
	if(ret == REQUEST_SUCCESS)		
	{
		snprintf(str,sizeof(str),"%s",	"{\"Action\":\"GetAllParam\",\"Result\":\"Success\",\"DevParam\":[");		
		strRet.append(str);
		for(auto info:infos)
		{
			snprintf(str,sizeof(str),"{\"DevIndex\":%d,",info.devidx);
			strRet.append(str);
			if(info.online == 1)
				snprintf(str,sizeof(str),"%s","\"Result\":\"Success\",");
			else
				snprintf(str,sizeof(str),"%s","\"Result\":\"AckTimeout\",");
			strRet.append(str);
			snprintf(str,sizeof(str),"\"Name\":\"%s\",\"DevType\":%d,\"RSSI\":%d,\"Rnum\":%d,",
			info.name,info.type,info.rssi,info.rnum);
			strRet.append(str);
			HexArrayToString(strReg,info.reg,info.rnum);
			snprintf(str,sizeof(str),"\"RegValue\":\"%s\"},",strReg.c_str());
			strRet.append(str);
		}		
		strRet.resize(strRet.size()-1);//delete the last character ','
		strRet.append("]}");
	}
	else
	{
		snprintf(str,sizeof(str),"{\"Action\":\"GetAllParam\",\"Result\":\"Faild\",\"ErrorCode\":%d}",ret);	
		strRet.append(str);
	}
	return HttpSend(sockfd,strRet,0);	
}

int Http_CmdToGateway(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd)
{
	std::string strAction = GetStringValue(strCmd,"Action");
	std::string strRet;
	if(gateway_sock == INVALID_SOCKET)
	{	
		strRet = "{\"Action\":\"" + strAction + "\",\"Result\":\"HostOffline\"}";
		HttpSend(user_sock,strRet);
		return -1;
	}
	int ret = SendToGateway(gateway_sock,strCmd);	
	if(ret < 0)
	{
		strRet = "{\"Action\":\"" + strAction + "\",\"Result\":\"HostOffline\"}";
		HttpSend(user_sock,strRet);
		return -2;
	}
	return ret;
}

/*
int Http_SetParam(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd)
{
	std::string strRet;
	if(gateway_sock == INVALID_SOCKET)
	{	
		strRet = "{\"Action\":\"SetParam\",\"Result\":\"HostOffline\"}";
		HttpSend(user_sock,strRet);
		return -1;
	}
	int ret = SendToGateway(gateway_sock,strCmd);	
	if(ret < 0)
	{
		strRet = "{\"Action\":\"SetParam\",\"Result\":\"HostOffline\"}";
		HttpSend(user_sock,strRet);
		return -2;
	}
	return ret;
}

int Http_MakePair(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd)
{
	std::string strRet;
	if(gateway_sock == INVALID_SOCKET)
	{	
		strRet = "{\"Action\":\"MakePair\",\"Result\":\"HostOffline\"}";
		HttpSend(user_sock,strRet);
		return -1;
	}
	int ret = SendToGateway(gateway_sock,strCmd);	
	if(ret < 0)
	{
		strRet = "{\"Action\":\"MakePair\",\"Result\":\"HostOffline\"}";
		HttpSend(user_sock,strRet);
		return -2;
	}
	return ret;
}

int Http_SetMulDevReg(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd)
{
	int ret = SendToGateway(gateway_sock,strCmd);
	if(ret < 0)
	{
		strRet = "{\"Action\":\"SetMulDevReg\",\"Result\":\"HostOffline\"}";
		HttpSend(user_sock,strRet);
	}
	return ret;
}

int Http_Rename(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd)
{
	int ret = SendToGateway(gateway_sock,strCmd);
	if(ret < 0)
	{
		strRet = "{\"Action\":\"Rename\",\"Result\":\"HostOffline\"}";
		HttpSend(user_sock,strRet);
	}
	return ret;
}*/

