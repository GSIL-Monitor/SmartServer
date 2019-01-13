#pragma once

#include <string>
using namespace std;

string GetStringValue(string &strCmd,string strTitle);
bool GetHeaderInfo(string &strAll,const char *strHeader,string & strInfo);
int HexArrayToString(std::string &strHex,const unsigned char *pbuf,int len);
int StringToHexArray(std::string& strHex,unsigned char *pbuf,int count);
int charset_convert_UTF8_TO_GB2312(char *in_buf, size_t in_left, char *out_buf, size_t out_left);
int charset_UTF8_TO_GB2312(std::string& strUTF8,std::string& strGB2312);
