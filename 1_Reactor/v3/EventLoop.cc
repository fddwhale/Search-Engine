#include "EventLoop.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include <unistd.h>

#include <iostream>

using std::cerr;
using std::endl;
using std::cout;

EventLoop::EventLoop(Acceptor &acceptor)
: _epfd(createEpollFd())
, _evtList(1024)
, _isLooping(false)
, _acceptor(acceptor)
{
    //将listenfd放在红黑树上进行监听
    int listenfd = _acceptor.fd();
    addEpollReadFd(listenfd);
}

EventLoop::~EventLoop()
{
    close(_epfd);
}

//事件循环与否
void EventLoop::loop()
{
    _isLooping = true;
    while(_isLooping)
    {
        waitEpollFd();
    }
}

void EventLoop::unloop()
{
    _isLooping = false;
}

//封装了epoll_wait函数
void EventLoop::waitEpollFd()
{
    int nready = 0;
    do
    {
        nready = epoll_wait(_epfd, &*_evtList.begin(), _evtList.size(), 3000);
    }while((-1 == nready && errno == EINTR));

    if(-1 == nready)
    {
        cerr << "-1 == nready" << endl;
        return;
    }
    else if(0 == nready)
    {
        cout << ">>epoll_wait timeout!!!" << endl;
    }
    else
    {
        //如果监听文件描述的个数超过设置的1024的，不能再进行扩容
        if(nready == (int)_evtList.size())
        {
            _evtList.resize(2 * nready);
        }

        for(int idx = 0; idx < nready; ++idx)
        {
            //连接是listenfd
            int fd = _evtList[idx].data.fd;
            int listenfd = _acceptor.fd();
            if(fd == listenfd)
            {
                if(_evtList[idx].events & EPOLLIN)
                {
                    //处理新的连接
                    handleNewConnection();
                }
            }
            else//处理老的连接
            {
                if(_evtList[idx].events & EPOLLIN)
                {
                    handleMessage(fd);
                }

            }
        }
    }
}
//处理新的连接请求
void EventLoop::handleNewConnection()
{
    //如果connfd有正确返回结果，就表明三次握手建立成功，
    //就可以创建连接
    int connfd = _acceptor.accept();
    if(connfd < 0)
    {
        perror("handleNewConnection accept");
        return;
    }

    //将创建出来的文件描述符放在红黑树上进行监听
    addEpollReadFd(connfd);

    //创建新的连接
    /* shared_ptr<TcpConnection> con(new TcpConnection(connfd)); */
    TcpConnectionPtr con(new TcpConnection(connfd));

    //将三个数据成员（回调函数）传递给连接TcpConnection
    con->setNewConnectionCallback(_onNewConnectionCb);//连接建立
    con->setMessageCallback(_onMessageCb);//消息到达
    con->setCloseCallback(_onCloseCb);//连接断开

    //将键值对存放在map中
    /* _conns.insert({connfd, con}); */
    _conns[connfd] = con;

    //连接建立的时机到了，就可以进行回调执行
    con->handleNewConnectionCallback();
}

//处理老的连接
void EventLoop::handleMessage(int fd)
{
    auto it = _conns.find(fd);
    if(it != _conns.end())
    {
        //如何判断连接是不是断开呢
        bool flag = it->second->isClosed();
        if(flag)
        {
            //连接断开了
            it->second->handleCloseCallback();//处理连接断开的事件
            delEpollReadFd(fd);//将文件描述符从红黑树上摘除掉
            _conns.erase(it);//将文件描述符与连接的键值对从map中删除
        }
        else
        {
            //消息在正常的收发
            it->second->handleMessageCallback();//消息到达（文件描述符可读）
        }
    }
    else
    {
        cout << "该连接不存在" << endl;
        return;
    }
}

//创建epfd的函数
int EventLoop::createEpollFd()
{
    int fd = epoll_create(10);
    if(fd < 0)
    {
        perror("createEpollFd");
        return fd;
    }

    return fd;
}

//监听文件描述符
void EventLoop::addEpollReadFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &evt);
    if(ret < 0)
    {
        perror("addEpollReadFd");
        return;
    }
}

//删除监听文件描述符
void EventLoop::delEpollReadFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &evt);
    if(ret < 0)
    {
        perror("delEpollReadFd");
        return;
    }
}

void EventLoop::setNewConnectionCallback(TcpConnectionCallback &&cb)
{
    _onNewConnectionCb = std::move(cb);
}

void EventLoop::setMessageCallback(TcpConnectionCallback &&cb)
{
    _onMessageCb = std::move(cb);
}

void EventLoop::setCloseCallback(TcpConnectionCallback &&cb)
{
    _onCloseCb = std::move(cb);
}
