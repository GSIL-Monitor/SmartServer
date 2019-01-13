#pragma once

int gateway_cmd(SOCKET_T sockfd,std::string& strCmd,ThreadInfoStruct* pInfo);
int Gateway_UpdateParam(SOCKET_T sockfd,std::string& strCmd);
int Gateway_Heart(SOCKET_T sockfd,std::string& strCmd);
int Gateway_MakePair(SOCKET_T user_sock,std::string& strCmd);
int Gateway_SetMulDevReg(SOCKET_T sockfd,std::string& strCmd);
int Gateway_SetParam(SOCKET_T sockfd,std::string& strCmd);
int Gateway_Rename(SOCKET_T user_sock,std::string& strCmd);

int get_gateway_position(std::string& strSerial);
