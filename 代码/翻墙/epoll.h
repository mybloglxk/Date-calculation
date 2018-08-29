
#ifndef __EPOLL_H__
#define __EPOLL_H__

#include "common.h"
#include "encry.h"
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE,SIG_IGN);
    }
};

static IgnoreSigPipe initPIPE_IGN;

class EpollServer
{
    public:
        EpollServer(int port = 8000)
            :_port(port)
             ,_listenfd(-1)
             ,_eventfd(-1)
    {}


        void Start();
        void Eventloop();


        void OPEvent(int fd,int events,int op)
        {


            struct epoll_event event;
            event.events = events;
            event.data.fd = fd;
            if(epoll_ctl(_eventfd,op,fd,&event)< 0)
            {
                ErrorLog("epoll ctl:%d+how:%d",fd,op);
            }
        }

        void SetNonblocking(int fd)
        {
            int flags,s;
            flags = fcntl(fd,F_GETFL,0);
            if(flags== -1)
            {
                ErrorLog("SetNonblocking F_GETFL:");
            }

            flags |= O_NONBLOCK;
            s = fcntl(fd,F_SETFL,flags);
            if(s== -1)
            {
                ErrorLog("SetNonblocking F_SETFL");
            }

        }

        enum Socks5State
        {
            AUTH,//身份认证
            ESTABLISHMENT,//建立连接
            FORWARDING, //转发

        };
        //通道连接描述符
        struct Channel
        {
            int _fd;
       
            string _buff;  //些缓冲
            Channel()
                :_fd(-1)
            {}
        };
    public:
        //标识数据来自何方，以便后续确定数据来自哪个通道，对其进行相应的响应
        struct Connect
        {
            //连接的状态
            Socks5State _state;
            //两个通道
            Channel _clientchannel;
            Channel _serverchannel;

            int _ref;

            Connect()
                :_state(AUTH)
                 ,_ref(0)
            {}
        };


   //     void Forwarding(Channel* clientchannel,Channel* serverchannel);
        void RemoveConnect(int fd);     
		void SendInLoop(int fd, const char* buf, int len);
		void Forwarding(Channel* clientchannel, Channel* serverchannel,bool sendencry,bool recvdecrypt);
        //处理连接事件
        virtual void ConnectEventHeadle(int connectfd)=0;
        //处理读事件
        virtual void ReadEventHeadle(int connectfd)=0;
        //处理写事件
        virtual void WriteEventHeadle(int connectfd);

        ~EpollServer()
        {
            if(_listenfd!= -1)
            {
                close(_listenfd);
            }
        }
    protected:
        int _port;
        int _listenfd;
        int _eventfd;

        map<int, Connect*> _fdConnectMap;//fd映射连接的map,第一个参数为文件描述符，第二个参数确定是哪个通道
};

#endif //__EPOLL_H__


