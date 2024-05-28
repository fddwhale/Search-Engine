#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include <sys/epoll.h>
#include <vector>
#include <map>
#include <memory>
#include <functional>

using std::vector;
using std::map;
using std::shared_ptr;
using std::function;

class Acceptor;//前向声明
class TcpConnection;//前向声明

using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallback = function<void(const TcpConnectionPtr &)>;

class EventLoop
{
public:
    EventLoop(Acceptor &acceptor);
    ~EventLoop();

    //事件循环与否
    void loop();
    void unloop();

private:
    //封装了epoll_wait函数
    void waitEpollFd();
    //处理新的连接请求
    void handleNewConnection();
    //处理老的连接
    void handleMessage(int fd);

    //创建epfd的函数
    int createEpollFd();
    //监听文件描述符
    void addEpollReadFd(int fd);
    //删除监听文件描述符
    void delEpollReadFd(int fd);

public:
    void setNewConnectionCallback(TcpConnectionCallback &&cb);
    void setMessageCallback(TcpConnectionCallback &&cb);
    void setCloseCallback(TcpConnectionCallback &&cb);

private:
    int _epfd;//epoll_create创建的文件描述符
    vector<struct epoll_event> _evtList;//存放满足条件的文件描述符的结构体
    bool _isLooping;//标识循环是否在运行
    Acceptor &_acceptor;
    map<int, TcpConnectionPtr> _conns;//存放文件描述符与连接的键值对

    TcpConnectionCallback _onNewConnectionCb;//连接建立
    TcpConnectionCallback _onCloseCb;//连接断开
    TcpConnectionCallback _onMessageCb;//消息到达（文件描述可读）

};

#endif
