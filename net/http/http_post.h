#pragma once
//#include "../../src/portable/platform.h"
//#include "../../HttpServer.h"
int HttpSend(SOCKET_T sockfd,std::string& strData,int flags = 0);
int HttpSend(SOCKET_T sockfd,const char *buf,int len,int flags = 0);
int HttpPost(SOCKET_T sockfd,const char *pBody,ThreadInfoStruct *pInfo);
int Http_UserLogin(SOCKET_T sockfd,std::string & strCmd,ThreadInfoStruct *pInfo);
int Http_UserRegister(SOCKET_T sockfd,std::string &strCmd,ThreadInfoStruct *pInfo);
int Http_AddUser(SOCKET_T sockfd,std::string &strCmd,ThreadInfoStruct *pInfo);
int Http_DeleteUser(SOCKET_T sockfd,std::string &strCmd,ThreadInfoStruct *pInfo);
int Http_GetUser(SOCKET_T sockfd,std::string &strCmd,ThreadInfoStruct *pInfo);
int Http_GetRights(SOCKET_T sockfd,std::string &strCmd);
int Http_SetRights(SOCKET_T sockfd,std::string &strCmd);
int Http_ModifyPwd(SOCKET_T sockfd,std::string& strCmd);
int Http_CheckUpdate(SOCKET_T sockfd,std::string& strCmd);
int Http_ForgetPassword(SOCKET_T sockfd,std::string& strCmd);
int Http_CheckEmail(SOCKET_T sockfd,std::string& strCmd);
int Http_AddEmail(SOCKET_T sockfd,std::string& strCmd);
int Http_GetMultiParam(SOCKET_T sockfd,std::string& strCmd);
int Http_GetAllParam(SOCKET_T sockfd,std::string& strCmd);
int Http_SetParam(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd);
int Http_MakePair(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd);
int Http_SetMulDevReg(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd);
int Http_Rename(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd);
int Http_CmdToGateway(SOCKET_T gateway_sock,SOCKET_T user_sock,std::string& strCmd);

int get_user_position(std::string& strUID);
