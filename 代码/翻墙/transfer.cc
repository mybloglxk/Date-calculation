#include "transfer.h"


void ConnectEventHandle(int connectfd)
{

	int serverfd=socket(AF_INET, SOCK_STREAM, 0);
    if(connect(serverfd,(struct sockaddr*)_socks5addr,sizeof(_socks5addr))<0)
    {
        ErrorLog("connect socks5 faild");
        return;
    }
    SetNonblocking(connectfd);
    OPEvevnt(connectfd,EPOLLIN,EPOLL_CTL_ADD);

	SetNonblocking(serverfd);
	OPEvent(serverfd, EPOLLIN, EPOLL_CTL_ADD);

    Connect* con = new Connect;
    con->_state = FORWARDING;

    con->_clientChannel._fd = connectfd;
    con->_ref++;
	_fdConnectMap[connectfd] = con;

    con->_serverchannel.fd = serverfd;
    con->_ref++;
	_fdConnectMap[serverfd] = con;
}
void ReadEventHandle(int fd)
{
    map<int,Connect*>::iterator it = _fdConnectMap.find(connectfd);
    if(it!=_fdConnectMap.end())
    {
        Connect* con = it->second;
        Channel* clientchannel = con->_clientchannel;
        Channel* serverchannel = con->_serverchannel;
		bool sendencry = true, recvdecrypt = false;
        if(connectfd == con->_serverchannel._fd)
        {
            swap(clientchannel,serverchannel);
			swap(sendencry, recvdecrypt);
			
        }
        
		

		Forwarding(clientchannel, serverchannel, sendencry, recvdecrypt);

    }
    else
    {
        assert(false);
    }

}



int main()
{
    TransferServer transfer;
    transfer.Start(8000,"49.51.51.188",8001);
}
