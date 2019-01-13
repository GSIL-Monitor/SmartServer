#pragma once

class SendACK_RAII{
public:
	SendACK_RAII(SOCKET_T sockfd)
	{
		m_sockfd = sockfd;
	}
	~SendACK_RAII()
	{
		const char *strRet="ACK";
		send(m_sockfd,strRet,strlen(strRet),0);
	}
private:
	SOCKET_T m_sockfd;
};


