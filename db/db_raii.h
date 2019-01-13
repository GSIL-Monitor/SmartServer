/*
 * file:db_raii.h
 * Write by TaoXiaoChao
 * RAII for mysql connection
*/

#pragma once
#include <mysql/mysql.h>
//#include "../app/app_config.h"

class MYSQL_RAII
{
public:	
	MYSQL_RAII(MYSQL *mysql,MYSQL_RES **result = NULL,const char* strDBName = MYSQL_DB_ACCOUNT)
	{
		m_mysql = mysql;
		m_result = result;	
		//m_mysql_state = DB_Open(m_mysql);				
		//LogD("RAII construct:res=%x, mysql=%x\n",*m_result,mysql);
		
		if(NULL==mysql_init(mysql)){
			LogE("%s(): mysql_init failed:%s\n",__func__,mysql_error(mysql));
			m_mysql_state = MYSQL_INIT_ERROR;
			return;
		}
		//LogD("mysql_init success\n");
		if(NULL==mysql_real_connect(mysql,
					MYSQL_SERVER_IP,
					MYSQL_USER_ID,
					MYSQL_USER_PWD,
					strDBName,
					MYSQL_SERVER_PORT,
					NULL,0))
		{
			LogE("%s(): mysql_real_connect failed:%s\n",__func__,mysql_error(mysql));
			m_mysql_state = MYSQL_REAL_CONNECT_ERROR;
			return;
		}
		//LogD("mysql_real_connect success,database:%s\n",strDBName);
		m_mysql_state = MYSQL_OPEN_SUCCESS;
		if(0 != mysql_set_character_set(mysql,"gb2312"))
		{
			LogE("mysql set character failed:%s\n",mysql_error(mysql));
		}
	}
	~MYSQL_RAII()
	{
		//LogD("RAII deconstruct:res=%x, mysql=%x\n",*m_result,m_mysql);		
		if(*m_result != NULL)
		{			
			mysql_free_result(*m_result);
			//LogD("RAII deconstruct:release mysql result\n");
		}		
		if(m_mysql_state == MYSQL_OPEN_SUCCESS)
		{
			mysql_close(m_mysql);
			//LogD("RAII deconstruct:colse mysql\n");
		}
		//LogD("RAII deconstruct:res=%x, mysql=%x\n",*m_result,m_mysql);
		//fprintf(stderr,"RAII deconstruct:res=%x, mysql=%x\n",*m_result,m_mysql);
		//LogE("%s(): RAII deconstruct:res=%x, mysql=%x\n",__func__,*m_result,m_mysql);
	}
	int GetMysqlState()
	{
		return m_mysql_state;
	}
private:
	int m_mysql_state;	
	MYSQL *m_mysql;
	MYSQL_RES **m_result;
};
