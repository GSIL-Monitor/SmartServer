#pragma once

#define account_length	32
#define password_length	32
#define nick_length		32
#define email_length		32
#define serial_length		24
#define rights_length		255
#define token_length		32

class AccountInfo{
public:
	unsigned int id;
	char account[account_length+1];
	char password[password_length+1];
	char nick[nick_length+1];		
	char email[email_length+1];	
	unsigned char admin;
	char serial[serial_length+1];	
	unsigned short devcnt;	
	unsigned char rowcnt;
	unsigned char enable;	
	unsigned char rights[255];
	char token[token_length+1];	
	unsigned int uid;
	
	SOCKET_T user_sock;
	
	
	AccountInfo()
	{
		//id =0;
		//memset(account,0,30);
		//memset(password,0,32);
		//memset(nick,0,30);
		
	}
};




