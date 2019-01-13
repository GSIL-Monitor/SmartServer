#pragma once

class SendACK_RAII{
public:
	SendACK_RAII(SOCKET_T sockfd,std::string& strAction)
	{
		m_sockfd = sockfd;
		m_strAction = strAction;
	}
	~SendACK_RAII()
	{
		std::string strRet;
		if(m_strAction == "GetParam")
		{
			strRet="ACK";
			send(m_sockfd,strRet.c_str(),strRet.size(),0);
		}
		else if(m_strAction == "Heart")
		{
			strRet="Action=Heart"; 
			send(m_sockfd,strRet.c_str(),strRet.size(),0);
		}
	}
private:
	SOCKET_T m_sockfd;
	std::string m_strAction;
};


