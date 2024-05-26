#include "TcpServer.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include <iostream>
#include <unistd.h>

using std::cout;
using std::endl;

class MyTask
{
public:
    MyTask(const string &msg, const TcpConnectionPtr &con)
    : _msg(msg)
    , _con(con)
    {

    }
    void process()
    {
        _msg;
        //进行业务逻辑的处理之后，还是需要通过连接进行收发
        //处理好的之后就是新的_msg1
        //如果在线程池中将业务逻辑处理好之后，需要让线程池
        //中的线程将处理好之后的数据发送给EventLoop/Reactor
        //，那么就会涉及到线程池中的线程与EventLoop之间的
        //通信问题？
        //解决方案：在进程或者线程之间进行通信的方式可以
        //使用eventfd.
        _con->sendInLoop(_msg1)
    }
private:
    string _msg;
    TcpConnectionPtr _con;
};

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
    //如果业务逻辑的处理比较复杂，那么就只能在此处执行
    MyTask task(msg, con);
    threadPool.addTask(bind(&MyTask::process, task);
    con->send(msg);
}

void onClose(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has closed!!!" << endl;
}

void test()
{
    TcpServer server("127.0.0.1", 8888);
    server.setAllCallback(std::move(onNewConnection)
                          , std::move(onMessage)
                          , std::move(onClose));
    server.start();
}

int main(int argc, char **argv)
{
    test();
    return 0;
}

