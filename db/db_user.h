#pragma once

int User_Login(const char *strAccount,const char *strPassword,class AccountInfo &Account);
//int User_Register(const char * strAccount,const char *strPassword,const char *strNick);
int User_Register(const char * strAccount,const char *strPassword,const char *strNick,const char *strSerial,const char *strEmail);
int AddUser(const char *strAccount,const char *strSerial);
int GetUser(const char *strSerial,std::vector<class AccountInfo> &vect);
int DeleteUser(const char *strAccount);
int GetRights(const char *strAccount,class AccountInfo &info);
int SetRights(const char *strAccount,int devcnt,const char *strRights);
int ModifyPwd(const char* strAccount,const char* strNewPwd);
int ForgetPassword(const char *strAccount,const char *strEmail,std::string& strNewPwd,std::string& strNick);
int CheckEmail(const char* strAccount,class AccountInfo &info);
int AddEmail(const char* strAccount,const char* strEmail);
int GetMultiParam(const char* strSerial,unsigned char* rights,	int cnt,std::vector<class DeviceInfo>& infos);
int GetAllParam(const char* strSerial,std::vector<class DeviceInfo>& infos);
int Admin_AddDevice(std::string& strSerial);
int GetUserInfo(std::string& strUID,class AccountInfo& info);

int CreateUserInfoVector(std::vector<class AccountInfo>& vec);


