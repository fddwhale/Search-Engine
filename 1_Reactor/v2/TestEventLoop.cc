#include "EventLoop.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include <iostream>
#include <unistd.h>

using std::cout;
using std::endl;

//连接建立的时候到底做什么事呢?
void onNewConnection(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has connected!!!" << endl;
}

void onMessage(const TcpConnectionPtr &con)
{
    string msg = con->receive();
    cout << ">>recv from client " << msg << endl;
    //可以进行处理msg，也就是进行正常的业务逻辑的处理
    //....
    con->send(msg);
}

void onClose(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has closed!!!" << endl;
}

void test()
{
    Acceptor acceptor("127.0.0.1", 8888);
    acceptor.ready();//此时处于监听状态
                     //
    EventLoop loop(acceptor);
    loop.setNewConnectionCallback(std::move(onNewConnection));
    loop.setMessageCallback(std::move(onMessage));
    loop.setCloseCallback(std::move(onClose));
    loop.loop();
}

int main(int argc, char **argv)
{
    test();
    return 0;
}

