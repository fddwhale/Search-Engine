#include "TcpServer.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "ThreadPool.h"
#include <unistd.h>
#include <iostream>
#include <functional>

using std::cout;
using std::endl;
using std::bind;

ThreadPool *gPool = nullptr;

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
        /* _msg; */
        //进行业务逻辑的处理之后，还是需要通过连接进行收发
        //处理好的之后就是新的_msg1
        //如果在线程池中将业务逻辑处理好之后，需要让线程池
        //中的线程将处理好之后的数据发送给EventLoop/Reactor
        //，那么就会涉及到线程池中的线程与EventLoop之间的
        //通信问题？
        //解决方案：在进程或者线程之间进行通信的方式可以
        //使用eventfd.
        _con->sendInLoop(_msg);
        //线程池处理好数据之后，需要将数据传递到EventLoop中
        //但是数据在传递的时候，只有TcpConnection才有传递
        //的特点，而在此处，是使用连接执行了sendInLoop函数，
        //那就表明TcpConnection要知道EventLoop的存在，才能
        //将数据发给EventLoop，所以就是一个类要知道另外一个
        //类的存在，可以使用关联关系(单向的关联关系)
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
    gPool->addTask(bind(&MyTask::process, task));
}

void onClose(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has closed!!!" << endl;
}

void test()
{
    ThreadPool pool(4, 10);
    pool.start();
    gPool = &pool;

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

