#include "epoll.h"


void EpollServer::Start()
{
    //创建监听套接字
    _listenfd = socket(PF_INET,SOCK_STREAM,0);
    if(_listenfd< 0)
    {
        ErrorLog("socket create");
        return;
    }

    struct sockaddr_in addr;
    //初始化
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(_listenfd,(struct sockaddr*)&addr,sizeof(addr))< 0)
    {
        ErrorLog("bind ");
        return;
    }

    if(listen(_listenfd,100000)< 0)
    {
        ErrorLog("listen");
        return;
    }

    TraceLog("Epoll Server listen on %d",_port);


    //创建epoll；
    

    _eventfd = epoll_create(100000);
    if(_eventfd< 0)
    {
        ErrorLog("epoll_create");
        return;
    }

    //将——listenfd添加到监听事件中，监听连接事件
    
    SetNonblocking(_listenfd);
    //事件操作
    OPEvent(_listenfd,EPOLLIN,EPOLL_CTL_ADD);
    //进入事件循环
    Eventloop();
}

void EpollServer::Eventloop()
{
//最大监听事件数量
    struct epoll_event events[100000];
    while(1)
    {
    //表示有多少个事件数据传输
        int n = epoll_wait(_eventfd,events,100000,0);
        //处理当前到达的所有事件
        for(int i = 0;i< n;i++)
        {
            if(events[i].data.fd == _listenfd)//新的连接到达事件，对其进行处理
            {
                struct sockaddr client;
                socklen_t len;

                int connectfd = accept(_listenfd,&client,&len);

                if(connectfd< 0)
                {
                    ErrorLog("accept");
                }
                else
                {
                    TraceLog("new connect");
                }
                //处理连接事件
                ConnectEventHeadle(connectfd);
            }

            else if(events[i].events&EPOLLIN)//读事件到达并处理
            {
                //
                ReadEventHeadle(events[i].data.fd);
            }
            else if(events[i].events&EPOLLOUT)//写事件到达并处理
            {
                WriteEventHeadle(events[i].data.fd);
            }
            else
            {
                ErrorLog("event: %d",events[i].data.fd);
            }
        }
    }
}

void EpollServer::RemoveConnect(int fd)
{
	OPEvent(fd, EPOLLIN, EPOLL_CTL_DEL);
	map<int, Connect*>::iterator it = _fdConnectMap.find(fd);
	if (it != _fdConnectMap.end())
	{
		Connect* con = it->second;
		if (--con->_ref == 0)
		{
			delete con;
			_fdConnectMap.erase(it);
		}
	}
	else
	{
		assert(false);
	}

}
void EpollServer::SendInLoop(int fd, const char* buf, int len)
{
	int slen = send(fd, buf, len, 0);
	if (slen<0)
	{
		ErrorLog("send: %d", fd);
	}
	else if (slen<len)
	{
		TraceLog("recv %d byte,send %d byte", len, slen);
		map<int, Connect*>::iterator it = _fdConnectMap.find(fd);
		if (it != _fdConnectMap.end())
		{
			Connect* con = it->second;
			Channel* channel = &con->_clientchannel;
			if (fd == con->_clientchannel._fd)
			{
				channel = &con->_serverchannel;
			}

			int events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
			OPEvent(fd, events, EPOLL_CTL_MOD);
			channel->_buff.append(buf + slen);
		}
		else
		{
			assert(false);
		}
	}
}

void EpollServer::Forwarding(Channel* clientchannel, Channel* serverchannel, bool sendencry, bool recvdecrypt)
{

	char buf[4096];
	int rlen = recv(clientchannel->_fd, buf, 4096, 0);
	if (rlen<0)
	{
		ErrorLog("recv: %d", clientchannel->_fd);
	}
	else if (rlen == 0)
	{
		shutdown(serverchannel->_fd, SHUT_WR);
		RemoveConnect(clientchannel->_fd);
	}
	else
	{
		if (recvdecrypt)
		{
			Decrypt(buf, rlen);
		}
		if (sendencry)
		{
			Encry(buf, rlen);
		}
		buf[rlen] = '\0';
		//   int slen = send(serverchannel->_fd,buf,rlen ,0);

		// TraceLog("recv:%d->send:%d",rlen,slen);
		SendInLoop(serverchannel->_fd, buf, rlen);
	}
}

void EpollServer::WriteEventHeadle(int fd)
{
	TraceLog("connect evevt: %d", fd);
	map<int, Connect*>::iterator it = _fdConnectMap.find(fd);
	if (it != _fdConnectMap.end())
	{
		Connect* con = it->second;
		Channel* channel = &con->_clientchannel;
		if (fd = con->_serverchannel._fd)
		{

			channel = &con->_serverchannel;
		}
		string buff;
		buff.swap(channel->_buff);
		// buff+= channel->_buff;
		// channel->_buff.clear();
		SendInLoop(fd, channel->_buff.c_str(), channel->_buff.size());
	}
	else
	{
		assert(fd);
	}

}
