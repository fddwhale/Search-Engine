#ifndef __TIMERFd_H__
#define __TIMERFd_H__

#include <functional>

using std::function;

class TimerFd
{
    using TimerFdCallback = function<void()>;
public:
    TimerFd(TimerFdCallback &&cb, int initSec, int peridocSec);
    ~TimerFd();

    //运行与停止
    void start();
    void stop();

private:
    //创建用于通信的文件描述符
    int createTimerFd();
    //A线程需要执行的read的操作
    void handleRead();
    //设置定时器
    void setTimerFd(int initSec, int peridocSec);

private:
    int _tfd;//进行通信的文件描述符
    int _initSec;//起始时间
    int _peridocSec;//周期时间
    TimerFdCallback _cb;//通信之后需要执行的事件
    bool _isStarted;//标识运行与否


};

#endif
