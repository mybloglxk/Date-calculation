
#ifndef __SOCKS5_H__
#define __SOCKS5_H__


#include "epoll.h"

class Socks5Server : public EpollServer
{
public:
	Socks5Server(int port)
    :EpollServer(port)
	{}


    
    int AuthHeadle(int fd);
    int EstablishmentHeadle(int fd);  
	virtual void ConnectEventHeadle(int connectfd);
	virtual void ReadEventHeadle(int connectfd);
	//virtual void WriteEventHeadle(int connectfd);

protected:

};

#endif //__SOCKS5_H__
