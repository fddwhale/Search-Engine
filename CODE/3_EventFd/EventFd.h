#ifndef __EVENTFD_H__
#define __EVENTFD_H__

#include <functional>

using std::function;

class EventFd
{
    using EventFdCallback = function<void()>;
public:
    EventFd(EventFdCallback &&cb);
    ~EventFd();

    //运行与停止
    void start();
    void stop();

private:
    //创建用于通信的文件描述符
    int createEventFd();
    //A线程需要执行的read的操作
    void handleRead();
public:
    //用于唤醒线程的函数
    void wakeup();


private:
    int _evtfd;//进行通信的文件描述符
    EventFdCallback _cb;//通信之后需要执行的事件
    bool _isStarted;//标识运行与否


};

#endif
