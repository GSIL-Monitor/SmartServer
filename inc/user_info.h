#pragma once

class UserInfo{
public:
	unsigned int id;
	char account[32];
	char password[32];
	char nick[32];		
	char email[32];	
	unsigned char admin;
	char serial[24];	
	unsigned short devcnt;	
	unsigned char rowcnt;
	unsigned char enable;	
	unsigned char rights[255];
	char token[33];	
	unsigned int uid;
	
};




