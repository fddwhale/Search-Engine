#include "TcpConnection.h"
#include "EventLoop.h"
#include <iostream>
#include <sstream>

using std::cout;
using std::endl;
using std::ostringstream;

TcpConnection::TcpConnection(int fd, EventLoop *loop)
: _loop(loop)
, _sockIO(fd)
, _sock(fd)
, _localAddr(getLocalAddr())
, _peerAddr(getPeerAddr())
{

}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::send(const string &msg)
{
    _sockIO.writen(msg.c_str(), msg.size());
}

//在TcpConnection中，将数据msg发送给EventLoop
//即使TcpConnection将线程池处理好之后的数据msg
//发送给EventLoop，但是EventLoop本身是没有发送
//数据的能力的，也就是即使拿到msg，也没有用，
//所以在此处，需要将数据msg以及发送数据能力
//的TcpConnection中的send函数一起打包发给
//EventLoop在，这样EventLoop才有能力将线程池
//处理好之后的数据发送给客户端
void TcpConnection::sendInLoop(const string &msg)
{
    if(_loop)
    {
        //线程池给EventLoop的“任务”就是处理好之后的数据msg
        //以及发送数据的能力send
        //void runInLoop(function<void()> &&cb)
        _loop->runInLoop(bind(&TcpConnection::send, this, msg));
    }

}

string TcpConnection::receive()
{
    char buff[65535] = {0};
    _sockIO.readLine(buff, sizeof(buff));

    return string(buff);
}

string TcpConnection::toString()
{
    ostringstream oss;
    oss << _localAddr.ip() << ":"
        << _localAddr.port() << "---->"
        << _peerAddr.ip() << ":"
        << _peerAddr.port();

    return oss.str();
}

//获取本端的网络地址信息
InetAddress TcpConnection::getLocalAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr );
    //获取本端地址的函数getsockname
    int ret = getsockname(_sock.fd(), (struct sockaddr *)&addr, &len);
    if(-1 == ret)
    {
        perror("getsockname");
    }

    return InetAddress(addr);
}

//获取对端的网络地址信息
InetAddress TcpConnection::getPeerAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr );
    //获取对端地址的函数getpeername
    int ret = getpeername(_sock.fd(), (struct sockaddr *)&addr, &len);
    if(-1 == ret)
    {
        perror("getpeername");
    }

    return InetAddress(addr);
}

//三个回调的注册
void TcpConnection::setNewConnectionCallback(const TcpConnectionCallback &cb)
{
    _onNewConnectionCb = cb;
}

void TcpConnection::setMessageCallback(const TcpConnectionCallback &cb)
{
    _onMessageCb = cb;
}

void TcpConnection::setCloseCallback(const TcpConnectionCallback &cb)
{
    _onCloseCb = cb;
}

//三个回调的执行
void TcpConnection::handleNewConnectionCallback()
{
    if(_onNewConnectionCb)
    {
        /* _onNewConnectionCb(shared_ptr<TcpConnection>(this)); */
        _onNewConnectionCb(shared_from_this());
    }
    else
    {
        cout << "_onNewConnectionCb == nullptr" << endl;
    }
}

void TcpConnection::handleMessageCallback()
{
    if(_onMessageCb)
    {
        _onMessageCb(shared_from_this());
    }
    else
    {
        cout << "_onMessageCb == nullptr" << endl;
    }
}

void TcpConnection::handleCloseCallback()
{
    if(_onCloseCb)
    {
        _onCloseCb(shared_from_this());
    }
    else
    {
        cout << "_onCloseCb == nullptr" << endl;
    }
}

bool TcpConnection::isClosed() const
{
    char buff[10] = {0};
    int ret = ::recv(_sock.fd(), buff, sizeof(buff), MSG_PEEK);

    return (0 == ret);
}
