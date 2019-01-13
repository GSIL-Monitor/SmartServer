#pragma once


#define REQUEST_SUCCESS					0


//Error code defined in mysql area
#define MYSQL_OPEN_SUCCESS				0
#define MYSQL_INIT_ERROR					-1
#define MYSQL_REAL_CONNECT_ERROR		-2
#define MYSQL_SNPRINTF_ERROR			-3
#define MYSQL_REAL_QUERY_ERROR			-4
#define MYSQL_STORE_RESULT_ERROR		-5
#define MYSQL_ROWS_NONE					-6 // number of rows is 0
#define MYSQL_INSERT_ERROR				-7
#define MYSQL_UPDATE_ERROR				-8
#define MYSQL_DELETE_ERROR				-9


//Error code defined about user login
#define USER_NOT_EXIST					-7
#define PASSWORD_ERROR					-8
#define ACCOUNT_DISABLE					-9


//Error code defined about user register
#define ACCOUNT_EXIST							-10		//account exist
//#define USER_REGISTER_SERIAL_NOT_EXIST	-10		//serial not exist
//#define USER_REGISTER_ADMIN_EXIST			-11		//admin exist
//#define USER_REGISTER_DB_ERROR				-12		//insert into database error//

#define USER_IS_ADMIN							-11
#define USER_IS_MEMBER						-12
#define USER_BELONG_OTHER					-13

#define EMAIL_ERROR							-14 //
#define EMAIL_EMPTY							-15

