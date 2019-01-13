#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <stdexcept>
#include <syslog.h>
#include "../app/app_config.h"
#include "../db/db_error.h"
#include "../db/db_user.h"
#include "../inc/account_info.h"
#include "../inc/device_info.h"
#include "../inc/function.h"
#include "../inc/log.h"
#include "../inc/error.h"
#include "../db/db_raii.h"
#include "../inc/mystring.h"
#include "../db/db_info.h"
#include "../inc/md5.h"

/**
  *Init and connect mysql server
  @param pmysql:the pointer to a MYSQL object
  @return MYSQL_OPEN_SUCCESS if success,else return MYSQL_INIT_ERROR or  MYSQL_REAL_CONNECT_ERROR
*/
/*
int DB_Open(MYSQL *pmysql)
{
	assert(pmysql!=NULL);
	if(NULL==mysql_init(pmysql)){
		LogE("%s(): mysql_init failed:%s\n",__func__,mysql_error(pmysql));
		return MYSQL_INIT_ERROR;
	}
	LogD("mysql_init success\n");
	if(NULL==mysql_real_connect(pmysql,
				MYSQL_SERVER_IP,
				MYSQL_USER_ID,
				MYSQL_USER_PWD,
				MYSQL_DB_ACCOUNT,
				MYSQL_SERVER_PORT,
				NULL,0))
	{
		LogE("%s(): mysql_real_connect failed:%s\n",__func__,mysql_error(pmysql));
		return MYSQL_REAL_CONNECT_ERROR;
	}
	LogD("mysql_real_connect success\n");
	return MYSQL_OPEN_SUCCESS;
}
*/
/**
  *Close mysql connect
  @param pmysql:the pointer to a MYSQL object
  @ return:void
*/
/*
void DB_Close(MYSQL *pmysql)
{
	mysql_close(pmysql);
}
*/

/**
 *Process user login message,check the account and password
 @param strAccount:user account
 @param strPassword:user password
 @param Account:to store the account infomation from database if login success 
 @return USER_LOGIN_SUCCESS if successed,or other value if failed
*/
int User_Login(const char *strAccount,const char *strPassword,class AccountInfo &Account)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	//Use RAII
	MYSQL_RAII Raii(&mysql,&res);
	int ret = Raii.GetMysqlState();
	if(ret != MYSQL_OPEN_SUCCESS)
		return ret;		
		
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),"select * from account where account='%s' order by id desc",strAccount);
	if(len <=0)
		return MYSQL_SNPRINTF_ERROR;	
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
//	LogD("mysql_real_query success.\n");
	
	
	res = mysql_store_result(&mysql); //Release the result by RAII
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
//	LogD("mysql_store_result success,res=%x.\n",res);
	int rows = mysql_num_rows(res);
//	LogD("total rows is %d\n",rows);
	if(rows <= 0)		
		return USER_NOT_EXIST;
	int fields = mysql_num_fields(res);
//	LogD("total fields is %d\n",fields);
	MYSQL_ROW row = mysql_fetch_row(res);
	
	GetAccountInfo(row,Account);//get account info from database
//	LogD("%s: row[2]=%s,pwd=%s\n",__func__,row[2],Account.password);//row[2] is password
	if(strcmp(Account.password,strPassword) != 0)	
		return PASSWORD_ERROR;
	//Account and Password are both right;
	
	//Account.enable = static_cast<unsigned char>(atoi(row[9]));
//	LogD("enable=%d\n",Account.enable);
	if(Account.enable == 0)
		return ACCOUNT_DISABLE;
	GenerateGUID(Account.token);	// generate a guid token	
/*	
	time_t timep;
	time(&timep);				
	struct tm *p = localtime(&timep);
	char strDate[11];
	snprintf(strDate,sizeof(strDate),"%d-%02d-%02d",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday);			
	
	len = sprintf(strSQL,"update Account set login_date='%s' where account=%s",strDate,strAccount);
	rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)	
		printf("mysql_real_query update login_date failed:%s\n",mysql_error(&mysql));
	else
		printf("mysql_real_query update login_date success.\n");		
*/
//	mysql_free_result(res);
//	DB_Close(&mysql);
	return REQUEST_SUCCESS;	
}


int User_Register(const char * strAccount,const char *strPassword,
						const char *strNick,const char *strSerial,
						const char *strEmail)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	//Use RAII
	MYSQL_RAII Raii(&mysql,&res);
	int ret = Raii.GetMysqlState();
	if(ret != MYSQL_OPEN_SUCCESS)
		return ret;				
		
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),"select * from account where account=%s order by id desc",strAccount);
	if(len <=0)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	LogD("mysql_real_query success.\n");	
	
	res = mysql_store_result(&mysql); //realese the result by RAII
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
	LogD("mysql_store_result success,res=%x.\n",res);
	int rows = mysql_num_rows(res);
	LogD("total rows is %d\n",rows);
	if(rows >= 1)		
		return ACCOUNT_EXIST;
	int admin = 0;
	if(strlen(strSerial) > 0)
		admin = 1;
	len = snprintf(strSQL,sizeof(strSQL),
				"insert into account values(0,'%s','%s','%s','%s',%d,'%s',0,0,1,UNHEX('00'))",
			strAccount,strPassword,strNick,strEmail,admin,strSerial);
	if(len <=0)
		return MYSQL_SNPRINTF_ERROR;
	rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): insert a new account into database failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	LogD("insert a new account into database success.\n");		
	return REQUEST_SUCCESS;
}

int AddUser(const char *strAccount,const char *strSerial)
{
//	int result = 0;
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	//LogD("mysql=%x\n",&mysql);
	//Use RAII
	MYSQL_RAII Raii(&mysql,&res);
	int ret = Raii.GetMysqlState();
	if(ret != MYSQL_OPEN_SUCCESS)
		return ret;	
	
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),"select * from account where account='%s'",strAccount);
	if(len <=0)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	LogD("%s(): mysql_real_query success.\n",__func__);
	
	
	res = mysql_store_result(&mysql); // release result by RAII
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}	
	LogD("%s(): mysql_store_result success,res=%x\n",__func__,res);
	int rows = mysql_num_rows(res);
	LogD("%s(): total rows is %d\n",__func__,rows);
	if(rows < 1)		
		return USER_NOT_EXIST;
	strSQL[255];
	MYSQL_ROW row = mysql_fetch_row(res);	
	class AccountInfo info;
	GetAccountInfo(row,info);//get account info from database
	//mysql_free_result(res);//release the result
	
	if(info.admin == 1)
		return USER_IS_ADMIN;
	
	if(strlen(info.serial) == 0)
	{
		len = snprintf(strSQL,sizeof(strSQL),"update account set serial='%s' where id=%d",strSerial,info.id);
		if(len <=0)
			return MYSQL_SNPRINTF_ERROR;
		rc = mysql_real_query(&mysql,strSQL,len);
		if(rc != 0)
		{
			LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
			return MYSQL_REAL_QUERY_ERROR;
		}
		LogD("%s(): add user success.\n",__func__);	
		return REQUEST_SUCCESS;
	}
	if(strcmp(info.serial,strSerial) == 0)
	{
		//Serial exist and is same of strSerial
		return USER_IS_MEMBER;
	}else
	{
		//serial exist,the user belongs to anther team
		return USER_BELONG_OTHER; 
	}
}

int DeleteUser(const char *strAccount)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	MYSQL_RAII Raii(&mysql,&res);
	int ret = Raii.GetMysqlState();
	if(ret != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),
					"update account set serial='', devcnt=0 where account='%s'",strAccount);
	if(len <=0)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0) 
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	LogD("%s(): mysql_real_query success\n",__func__);
	return REQUEST_SUCCESS;
}


/* Get all ther user info with the same serial 
 * @param strSerial:the same serial of the users
 * @param vect:a vector to store the users infomation
 * @return:REQUEST_SUCCESS for successed,USER_NOT_EXIST for no user,other value for other errors;
**/
int GetUser(const char *strSerial,std::vector<class AccountInfo> &vect)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	class MYSQL_RAII Raii(&mysql,&res);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	
	char strSQL[256];
	int len = snprintf(strSQL,sizeof(strSQL),"select * from account where serial='%s' order by id asc",strSerial);
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	res = mysql_store_result(&mysql);
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
	int rows = mysql_num_rows(res);
	if(rows < 1)
		return USER_NOT_EXIST;
	//LogD("%s(): rows=%d\n",__func__,rows);	
	MYSQL_ROW row;
	vect.reserve(rows);
	for(int i=0;i<rows;i++)
	{
		row = mysql_fetch_row(res);	
		class AccountInfo info;
		info.id = static_cast<unsigned int>( atoi(row[0]) );
		strncpy(info.account,row[1],sizeof(info.account));
		strncpy(info.nick,row[3],sizeof(info.nick));
		info.admin =  static_cast<unsigned char>( atoi(row[5]));//admin
		info.devcnt =  static_cast<unsigned char>( atoi(row[7]));//devcnt
		info.rowcnt =  static_cast<unsigned char>( atoi(row[8]));//rowcnt
		info.enable =  static_cast<unsigned char>( atoi(row[9]));//enable
		memcpy(info.rights, row[10], sizeof(info.rights));//rights
		vect.push_back(info);
	}
	return REQUEST_SUCCESS;
}

int GetRights(const char *strAccount,class AccountInfo &info)
{
	MYSQL mysql;
	MYSQL_RES *res =  NULL;
	MYSQL_RAII Raii(&mysql,&res);
	int ret = Raii.GetMysqlState();
	if(ret != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),
					"select * from account where account='%s'",strAccount);
	if(len<=0)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	res = mysql_store_result(&mysql);
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
	int rows = mysql_num_rows(res);
	if(rows < 1)
		return USER_NOT_EXIST;
	MYSQL_ROW row = mysql_fetch_row(res);
	GetAccountInfo(row,info);//get account info from a row
	return REQUEST_SUCCESS;	
}

int SetRights(const char *strAccount,int devcnt,const char *strRights)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	MYSQL_RAII Raii(&mysql,&res);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[1024];
	int len = snprintf(strSQL,sizeof(strSQL),
						"update account set devcnt=%d,rights=UNHEX('%s') where account='%s'",devcnt,strRights,strAccount);
	if(len<1)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}		
	return REQUEST_SUCCESS;
}

/**
 *Modify password in database
 @param strAccount:the account wanted to modify password
 @param strNewPwd:the new password
 @return:o if success,or ther value if failed;
*/
int ModifyPwd(const char* strAccount,const char* strNewPwd)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	MYSQL_RAII Raii(&mysql,&res);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[1024];
	int len = snprintf(strSQL,sizeof(strSQL),
					"update account set password='%s' where account='%s'",strNewPwd,strAccount);
	if(len<1)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	return REQUEST_SUCCESS;
}

int ForgetPassword(const char *strAccount,const char *strEmail,std::string& strNewPwd,std::string& strNick)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	MYSQL_RAII Raii(&mysql,&res);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[1024];
	int len = snprintf(strSQL,sizeof(strSQL),
					"select * from account where account='%s' order by id desc",strAccount);
	if(len<1)
		return MYSQL_SNPRINTF_ERROR;		
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}					
	res = mysql_store_result(&mysql);
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result error:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
	int rows = mysql_num_rows(res);
	if(rows <1)
		return USER_NOT_EXIST;
	MYSQL_ROW row = mysql_fetch_row(res); 
	class AccountInfo info;
	GetAccountInfo(row,info);
	if(strcmp(info.email,strEmail) != 0)
		return EMAIL_ERROR;
	unsigned int dat = GetRandomData();//get a random data from 000000-999999
	//unsigned int dat = GetRandomData(100000,999999);//get a random data from 100000-999999
	char strPwd[7];
	snprintf(strPwd,sizeof(strPwd),"%06d",dat);//format the data to string
	std::string strMD5;
	GetMD5(strPwd,strMD5);//Get MD5 of the password
	len = snprintf(strSQL,sizeof(strSQL),
					"update account set password='%s' where account='%s'",
					strMD5.c_str(),strAccount);
	if(len<1)
		return MYSQL_SNPRINTF_ERROR;		
	rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query2 failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}	
	strNewPwd.append(strPwd);	
	strNick.append(info.nick);			
	return REQUEST_SUCCESS;
}

int CheckEmail(const char* strAccount,class AccountInfo &info)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	MYSQL_RAII Raii(&mysql,&res);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),
				"select * from account where account='%s' order by id desc",strAccount);
	if(len < 1)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}				
	res = mysql_store_result(&mysql);
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	GetAccountInfo(row,info);
	if(strlen(info.email) == 0)
		return EMAIL_EMPTY;
	return REQUEST_SUCCESS;
}


int AddEmail(const char* strAccount,const char* strEmail)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	MYSQL_RAII Raii(&mysql,&res);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),
				"update account set email='%s' where account='%s'",strEmail,strAccount);
	if(len < 1)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}	
	return REQUEST_SUCCESS;
}

int GetMultiParam(const char* strSerial,unsigned char* rights,
						int cnt,std::vector<class DeviceInfo>& infos)
{
	MYSQL mysql;
	MYSQL_RES* res = NULL;
	MYSQL_RAII Raii(&mysql,&res,MYSQL_DB_DEVINFO);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[255];
	int len =0 ;
	int rc = 0;
	MYSQL_ROW row;
	my_ulonglong row_num;
	for(int i=0;i<cnt;i++)
	{
		len = snprintf(strSQL,sizeof(strSQL),"select * from devinfo where serial='%s' and devidx=%d",
				strSerial,*(rights+i));
		if(len < 1)
			return MYSQL_SNPRINTF_ERROR;
		//LogD("strSQL=%s\n",strSQL);		
		rc = mysql_real_query(&mysql,strSQL,len);
		if(rc != 0)
		{
			LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
			return MYSQL_REAL_QUERY_ERROR;
		}
		res = mysql_store_result(&mysql);
		if(res == NULL)
		{
			LogE("%s(): mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
		 	return MYSQL_STORE_RESULT_ERROR;
		}		
		row_num = mysql_num_rows(res);
		if(row_num >= 1)
		{
			row = mysql_fetch_row(res);
			if(row != NULL)
			{				
				class DeviceInfo info;
				GetDeviceInfo(row,info);				
				infos.push_back(info);
			}
		}		
		mysql_free_result(res);// this is very important,because of many times of mysql_store_result();
	}
	res = NULL;//this is very important,because of many times of mysql_store_result();
	return REQUEST_SUCCESS;
}

int GetAllParam(const char* strSerial,std::vector<class DeviceInfo>& infos)
{
	MYSQL mysql;
	MYSQL_RES* res = NULL;
	MYSQL_RAII Raii(&mysql,&res,MYSQL_DB_DEVINFO);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[255];		
	MYSQL_ROW row;	
	int len = snprintf(strSQL,sizeof(strSQL),"select * from devinfo where serial='%s' order by devidx asc",
			strSerial);
	if(len < 1)
		return MYSQL_SNPRINTF_ERROR;
	//LogD("strSQL=%s\n",strSQL);		
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	res = mysql_store_result(&mysql);
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
	 	return MYSQL_STORE_RESULT_ERROR;
	}
	int rows = mysql_num_rows(res);
	if(rows < 1)
	{
		LogE("%s(): mysql_num_rows <1\n");
		return MYSQL_ROWS_NONE;
	}	
	infos.reserve(rows);
	for(int i=0;i<rows;i++)
	{
		row = mysql_fetch_row(res);
		if(row != NULL)
		{
			class DeviceInfo info;
			GetDeviceInfo(row,info);
			infos.push_back(info);
		}
	}	
	return REQUEST_SUCCESS;
}

/**
 *set a account to be admin
 @param strAccount:the account to be set admin
 @return:0 if successed,or -1 if failed
*/
int User_SetAdmin(const char *strAccount)
{
	return 0;
}

/**
 *set a account to be admin
 @param id:the if of the account which to be set admin
 @return:0 if successed,or -1 if failed
*/
int User_SetAdmin(const unsigned int id,const char *strSerial)
{
	
	return 0;
}

/**
 *Generate a radom password if the user forget password
 @param strAccount:the user account
 @param strEamil:the user email registered;
 @param pBuf:the buffer to store the radom password
 @param bufLen:max length of the buffer
 @return:0 if successed,or -1 if failed
*/
int User_ForgetPassword(const char *strAccount,const char *strEmail,char *pBuf,int bufLen)
{
	return 0;
}

/*
 *Modify the user password
 @param strAccount:the user account
 @param strNewPassword:the new password
 @return:0 if successd,or -1 failed
**/
int User_ModifyPassword(const char *strAccount,const char *strNewPassword)
{
	return 0;
}

/**
 *Get the rights of a account
 @param strAccount:the account
 @param pBuf:the buffer to store rights
 @param bufLen:the max length of the buffer
*/
int User_GetRights(const char *strAccount,char *pBuf,int bufLen)
{

	return 0;	
}

int Admin_AddDevice(std::string& strSerial)
{
	MYSQL mysql;
	MYSQL_RES* res = NULL;
	MYSQL_RAII Raii(&mysql,&res,MYSQL_DB_ACCOUNT);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;	
	std::string strSQL = "update account set devcnt=devcnt+1 where serial='"+strSerial+
					"' and admin=1";
	int rc = mysql_real_query(&mysql,strSQL.c_str(),strSQL.size());
	if(rc != 0)
	{
		LogE("%s():mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}	
	return REQUEST_SUCCESS;
}

/**
 *Set the rights of a account
 @param strAccount:the account
 @param strRights:the rights to be set
 @return: 0 if successed,-1 if failed
*/
int User_SetRights(const char *strAccount,const char *strRigths)
{
	
	return 0;	
}

int GetUserInfo(std::string& strUID,class AccountInfo& info)
{
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	//Use RAII
	MYSQL_RAII Raii(&mysql,&res);
	int ret = Raii.GetMysqlState();
	if(ret != MYSQL_OPEN_SUCCESS)
		return ret;		
		
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),"select * from account where id=%s",strUID.c_str());
	if(len <=0)
		return MYSQL_SNPRINTF_ERROR;	
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
//	LogD("mysql_real_query success.\n");	
	res = mysql_store_result(&mysql); //Release the result by RAII
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
//	LogD("mysql_store_result success,res=%x.\n",res);
	int rows = mysql_num_rows(res);
	//LogD("total rows is %d\n",rows);
	if(rows <= 0)		
		return USER_NOT_EXIST;
	MYSQL_ROW row = mysql_fetch_row(res);	
	GetAccountInfo(row,info);//get account info from database
	return REQUEST_SUCCESS;	
}

/*
int User_Register(const char * strAccount,const char *strPassword,const char *strNick,const char *strSerial)
{
	MYSQL mysql;
	int ret = DB_Open(&mysql);
	if( ret != MYSQL_OPEN_SUCCESS)	
		return ret;
	if(0 != DB_CheckSerialExist(strSerial))
		return USER_REGISTER_SERIAL_NOT_EXIST;
		
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),"select * from Account where account=%s order by id desc",strAccount);
	if(len <=0)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		printf("mysql_real_query failed:%s\n",mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	printf("mysql_real_query success.\n");
	
	return USER_REGISTER_SUCCESS;
}*/

int CreateUserInfoVector(std::vector<class AccountInfo>& vec)
{
	MYSQL mysql;
	MYSQL_RES* res = NULL;
	MYSQL_RAII Raii(&mysql,&res,MYSQL_DB_ACCOUNT);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[1024];
	int len = snprintf(strSQL,sizeof(strSQL),"%s","select * from account order by id asc");
	if(len < 1)
		return MYSQL_SNPRINTF_ERROR;
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s():mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	res = mysql_store_result(&mysql);
	if(res == NULL)
	{
		LogE("%s():mysql_store_result failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
	unsigned long num= static_cast<unsigned long>(mysql_num_rows(res));
	if(num < 1)
		return MYSQL_ROWS_NONE;//no row
	try{
		vec.reserve(static_cast<std::vector<class AccountInfo>::size_type>(num +100));
	}catch(const std::length_error& le)
	{
		LogE("%s():AccountInfo vector reserve exception,what()=%s\n",__func__,le.what());
		return LENGTH_ERROR_EXCEPTION;
	}
	//LogI("%s():Account num = %lu.\n",__func__,num);
	syslog(LOG_INFO_FACILITY, "%s():Account num = %lu.\n",__func__,num);
	MYSQL_ROW row;
	for(unsigned long i=0;i<num;i++)
	{
		row = mysql_fetch_row(res);
		if(row == NULL)
			break;
		class AccountInfo info;
		GetAccountInfo(row,info);		
		vec.push_back(info);
	}
	return REQUEST_SUCCESS;
}

