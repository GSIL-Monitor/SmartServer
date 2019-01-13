#pragma once

struct Base64Date6
{
unsigned int d4:6;
unsigned int d3:6;
unsigned int d2:6;
unsigned int d1:6;
};

char ConvertToBase64(char uc);
void  EncodeBase64(char*dbuf,char*buf128,int len);
