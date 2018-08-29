

#ifndef __TRANSFER_H__
#define __TRANSFER_H__
#include "common.h"
#include "epoll.h"
class TransferServer : public EpollServer
{
public:
    TransferServer(int selfport,const char* socks5IP,int socks5port)
    :EpollServer(selfport)
    {
        memset(&_socks5addr, 0, sizeof(struct sockaddr_in));
        _socks5addr.sin_family = AF_INET;
        _socks5addr.sin_port = htons(socks5port);
        _socks5addr.sin_addr.s_addr = inet_addr(socks5IP);
    }

    virtual void ConnectEventHandle(int connnectfd);
    virtual void ReadEventHandle(int connectfd);
  //  virtual void WriteEventHandle(int connectfd);
protected:
    struct sockaddr_in _socks5addr;
};


#endif //__TRANSFER_H__
