#include <stdio.h>
#include <string.h>
#include <string>
#include <mysql/mysql.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include "../../app/app_config.h"
#include "../../inc/device_info.h"
#include "../../inc/log.h"
#include "../../inc/error.h"
#include "../../db/db_error.h"
#include "../../db/db_raii.h"
#include "../../inc/mystring.h"
#include "../../db/db_info.h"
#include "../../inc/gateway_info.h"

int UpdateDeviceInfo(class DeviceInfo& info,std::string& strAction)
{	
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	MYSQL_RAII Raii(&mysql,&res,MYSQL_DB_DEVINFO);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),
				"select * from devinfo where serial='%s' and devidx=%d order by id desc",
				info.serial,info.devidx);
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	res = mysql_store_result(&mysql);
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
	std::string strReg;
	HexArrayToString(strReg,info.reg,info.rnum);	
	int rows = mysql_num_rows(res);		
	if(rows < 1)
	{		
		len = snprintf(strSQL,sizeof(strSQL),
				"insert into devinfo values('0','%s','%d','%s','%s','%d','%d','%d','%d','%d',UNHEX('%s'))",
				info.serial,info.devidx,info.address,
				info.name,info.type,info.online,
				info.rssi,info.rnum,info.enable,
				strReg.c_str());		
		rc = mysql_real_query(&mysql,strSQL,len);
		if(rc != 0)
		{
			LogE("%s():  mysql insert into devinfo failed:%s\n",__func__,mysql_error(&mysql));
			return MYSQL_INSERT_ERROR;
		}
		LogI("%s():  mysql insert into devinfo success.\n",__func__);
		return REQUEST_SUCCESS;		
	}
	//MYSQL_ROW row  = mysql_fetch_row(res);
	//class DeviceInfo oldInfo;
	//GetDeviceInfo(row,oldInfo);
	//mysql_free_result(res);
	//res = NULL;
	
	/*
	if(info.online == 0)
	{
		info.type = oldInfo.rssi;
		info.rssi = oldInfo.rssi;
		info.rnum = oldInfo.rnum;
		info.enable = oldInfo.enable;
		memcpy(info.reg,oldInfo.reg,oldInfo.rnum);
	}*/
	strReg.clear();	
	HexArrayToString(strReg,info.reg,info.rnum);	
	int lenAll = 0;
	lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"%s","update devinfo set ");	
	if(strAction == "GetParam")
	{
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,//"%s","update devinfo set enable=0 where id=4");		
						"address='%s',name='%s',",info.address,info.name);	
	}	
	/*			
	lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,//"%s","update devinfo set enable=0 where id=4");		
					"update devinfo set address='%s',name='%s',type=%d,"
					"online=%d,rssi=%d,rnum=%d,enable=%d,reg=UNHEX('%s')"
					" where serial='%s' and devidx=%d",
					info.address,info.name,info.type,
					info.online,	info.rssi,info.rnum,info.enable,strReg.c_str(),
					info.serial,info.devidx);	
	*/				
	if(info.online == 1)
	{
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,//"%s","update devinfo set enable=0 where id=4");		
					"type=%d,online=%d,rssi=%d,rnum=%d,enable=%d,reg=UNHEX('%s')",					
					info.type,info.online,info.rssi,info.rnum,info.enable,strReg.c_str());	
	}else
	{
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,					
					"online=%d",info.online);	
	}
	lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,	
					" where serial='%s' and devidx=%d",info.serial,info.devidx);	
	LogD("strSQL=%s\n",strSQL);
	rc = mysql_real_query(&mysql,strSQL,lenAll);
	if(rc != 0)
	{
		LogE("%s():  mysql update devinfo failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_INSERT_ERROR;
	}
	LogI("%s():  mysql update devinfo success.\n",__func__);
	return REQUEST_SUCCESS;		
}

int UpdateDeviceInfo(class DeviceInfo& info)
{	
	MYSQL mysql;
	MYSQL_RES *res = NULL;
	MYSQL_RAII Raii(&mysql,&res,MYSQL_DB_DEVINFO);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[255];
	int len = snprintf(strSQL,sizeof(strSQL),
				"select * from devinfo where serial='%s' and devidx=%d order by id desc",
				info.serial,info.devidx);
	int rc = mysql_real_query(&mysql,strSQL,len);
	if(rc != 0)
	{
		LogE("%s(): mysql_real_query failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_REAL_QUERY_ERROR;
	}
	res = mysql_store_result(&mysql);
	if(res == NULL)
	{
		LogE("%s(): mysql_store_result failed:%s",__func__,mysql_error(&mysql));
		return MYSQL_STORE_RESULT_ERROR;
	}
	std::string strReg;
	HexArrayToString(strReg,info.reg,info.rnum);	
	int rows = mysql_num_rows(res);		
	if(rows < 1)
	{
	/*		
		len = snprintf(strSQL,sizeof(strSQL),
				"insert into devinfo values('0','%s','%d','%s','%s','%d','%d','%d','%d','%d',UNHEX('%s'))",
				info.serial,info.devidx,info.address,
				info.name,info.type,info.online,
				info.rssi,info.rnum,info.enable,
				strReg.c_str());	
		*/	
		len = 0;
		len = snprintf(strSQL,sizeof(strSQL),	"insert into devinfo values('0','%s','%d',",
				info.serial,info.devidx);
		if(info.address_update)
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%s',",info.address);
		else	
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"%s","'',");
		if(info.name_update)
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%s',",info.name);
		else	
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"%s","'',");
		if(info.type_update)
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",info.type);
		else	
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",0);
		if(info.online_update)
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",info.online);
		else	
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",0);
		if(info.rssi_update)
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",info.rssi);
		else	
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",0);
		if(info.rnum_update)
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",info.rnum);
		else	
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",0);
		if(info.enable_update)
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",info.enable);
		else	
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"'%d',",0);
		if(info.reg_update)
		{
			std::string strReg;
			HexArrayToString(strReg,info.reg,info.rnum);
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"UNHEX('%s'))",strReg.c_str());
		}
		else	
			len += snprintf(strSQL+len,sizeof(strSQL)-len,"UNHEX('%s'))","00");
		LogD("strSQL= %s\r\n",strSQL);
		rc = mysql_real_query(&mysql,strSQL,len);
		if(rc != 0)
		{
			LogE("%s():  mysql insert into devinfo failed:%s\n",__func__,mysql_error(&mysql));
			return MYSQL_INSERT_ERROR;
		}
		LogI("%s():  mysql insert into devinfo success.\n",__func__);
		return REQUEST_SUCCESS;		
	}
	
	strReg.clear();	
	HexArrayToString(strReg,info.reg,info.rnum);	
	int lenAll = 0;
	lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"%s","update devinfo set ");		
	if(info.address_update)
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"address='%s',",info.address);
	if(info.name_update)
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"name='%s',",info.name);
	if(info.type_update)
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"type=%d,",info.type);
	if(info.online_update)
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"online=%d,",info.online);
	if(info.rssi_update)
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"rssi=%d,",info.rssi);
	if(info.rnum_update)
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"rnum=%d,",info.rnum);
	if(info.enable_update)
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"enable=%d,",info.enable);
	if(info.reg_update)
	{
		std::string strReg;
		HexArrayToString(strReg,info.reg,info.rnum);
		lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,"reg=UNHEX('%s')",strReg.c_str());
	}		
	if(strSQL[lenAll-1] == ',')
		lenAll --;//delete the last character	 in strSQL
	lenAll += snprintf(strSQL+lenAll,sizeof(strSQL)-lenAll,	
					" where serial='%s' and devidx=%d",info.serial,info.devidx);	
	LogD("strSQL=%s\r\n",strSQL);
	rc = mysql_real_query(&mysql,strSQL,lenAll);
	if(rc != 0)
	{
		LogE("%s():  mysql update devinfo failed:%s\n",__func__,mysql_error(&mysql));
		return MYSQL_INSERT_ERROR;
	}
	LogI("%s():  mysql update devinfo success.\n",__func__);
	return REQUEST_SUCCESS;		
}

int CreateGatewayInfoVector(std::vector<class GatewayInfo>& vec)
{
	MYSQL mysql;
	MYSQL_RES* res = NULL;
	MYSQL_RAII Raii(&mysql,&res,MYSQL_DB_GATEWAY);
	if(int ret = Raii.GetMysqlState() != MYSQL_OPEN_SUCCESS)
		return ret;
	char strSQL[1024];
	int len = snprintf(strSQL,sizeof(strSQL),"select * from gateway order by id asc");
	if( len < 1)
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
	unsigned long num = static_cast<unsigned long>( mysql_num_rows(res));
	if(num < 1)
		return MYSQL_ROWS_NONE;
	try{
		vec.reserve(static_cast<std::vector<class GatewayInfo>::size_type> (num+100));
	}catch(const std::length_error& le){
		LogE("%s():GatewayInfo vector reserve exception,what()=%s\n",__func__,le.what());
		return LENGTH_ERROR_EXCEPTION;
	}
	LogI("%s():gateway num = %lu.\n",__func__,num);
	MYSQL_ROW row;
	for(unsigned long i=0;i<num;i++)
	{
		row = mysql_fetch_row(res);
		if(row == NULL)
			break;
		class GatewayInfo info;
		GetGatewayInfo(row,info);
		vec.push_back(info);
	}
	return REQUEST_SUCCESS;
}



