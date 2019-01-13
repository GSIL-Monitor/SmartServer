//#include <stdio.h>
#include <string>
#include <string.h>
#include <stdexcept>
#include "log.h"
#include <iconv.h>
//using namespace std;

std::string GetStringValue(std::string &strCmd,std::string strTitle)
{
	std::size_t indexL,indexR;
	strTitle.append("=");
	indexL = strCmd.find(strTitle);
	if(indexL == std::string::npos)
		return "";
	indexL = strCmd.find('=',indexL);
	if(indexL == std::string::npos)
		return "";
	indexR = strCmd.find('*',indexL);
	if(indexR == std::string::npos)
		return "";			
	try{
		std::string strValue = strCmd.substr(indexL+1,indexR-indexL-1);	
		return strValue;
	}catch(const std::out_of_range& oor){
		LogE("%s():out_of_rang exception,what=%s\n",__func__,oor.what());
	}catch(const std::bad_alloc& ba){
		LogE("%s():bad_alloc exception,what=%s\n",__func__,ba.what());
	}
	return "";	
}

bool GetHeaderInfo(std::string &strAll,const char *strHeader,std::string & strInfo)
{
	std::size_t indexL = strAll.find(strHeader);
	if(indexL == std::string::npos)
		return false;
	indexL = strAll.find(':',indexL);
	if(indexL == std::string::npos)
		return false;
	std::size_t indexR = strAll.find("\r\n",indexL);
	if(indexR == std::string::npos)
		return false;
	strInfo = strAll.substr(indexL+1,indexR-indexL-1);
	strInfo.erase(0,strInfo.find_first_not_of(' '));//Trim Left
	strInfo.erase(strInfo.find_last_not_of(' ')+1);//Trim Right
	return true;
}

int HexArrayToString(std::string &strHex,const unsigned char *pbuf,int len)
{
	char str[3];
	strHex.clear();
	strHex.reserve(len *2);
	for(int i=0;i<len;i++)
	{
		snprintf(str,sizeof(str),"%02X",*(pbuf+i));
		strHex.append(str);
	}
	return strHex.size();
}

int StringToHexArray(std::string& strHex,unsigned char *pbuf,int count)
{
	std::string str;
	std::size_t idx;
	try{
		for(int i=0;i<count;i++)
		{
			str = strHex.substr(i*2,2);
			*(pbuf+i) = stoi(str,&idx,16);
		}
	}catch(const std::out_of_range& oor){
		return -1;
	}catch(const std::bad_alloc& ba){
		return -2;
	}
	return 0;
}

int charset_convert(const char *from_charset, const char *to_charset,
                           char *in_buf, size_t in_left, char *out_buf, size_t out_left)
{
    iconv_t icd = (iconv_t)-1;
    size_t sRet = -1;
    char *pIn = in_buf;
    char *pOut = out_buf;
    size_t outLen = out_left;
 
    if (NULL == from_charset || NULL == to_charset || NULL == in_buf || 0 >= in_left || NULL == out_buf || 0 >= out_left)
    {
        return -1;
    }
 
    icd = iconv_open(to_charset, from_charset);
    if ((iconv_t)-1 == icd)
    {
        return -1;
    }
 
    sRet = iconv(icd, &pIn, &in_left, &pOut, &out_left);
    if ((size_t)-1 == sRet)
    {
        iconv_close(icd);
        return -1;
    }
 
    out_buf[outLen - out_left] = 0;
    iconv_close(icd);
    return (int)(outLen - out_left);
}

int charset_convert_UTF8_TO_GB2312(char *in_buf, size_t in_left, char *out_buf, size_t out_left)
{
    return charset_convert("UTF-8", "GB2312", in_buf, in_left, out_buf, out_left);
}
 
int charset_convert_GB2312_TO_UTF8(char *in_buf, size_t in_left, char *out_buf, size_t out_left)
{
    return charset_convert("GB2312-8", "UTF-8", in_buf, in_left, out_buf, out_left);
}

int charset_UTF8_TO_GB2312(std::string& strUTF8,std::string& strGB2312)
{
	//std::string from_name_utf8="广东建科";
	char *ptr_gb2312 = new (std::nothrow) char[strUTF8.size()+1];
	if(ptr_gb2312 == nullptr)
		return -1;	
	memset(ptr_gb2312,0,strUTF8.size()+1);
	charset_convert_UTF8_TO_GB2312((char*)strUTF8.c_str(),strUTF8.size(),
					ptr_gb2312,strUTF8.size()+1);	
	strGB2312.append(ptr_gb2312);
	delete[] ptr_gb2312;
	return 0;
}

