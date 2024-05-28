#include "TimerFd.h"
#include <unistd.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

TimerFd::TimerFd(TimerFdCallback &&cb, int initSec, int peridocSec)
: _tfd(createTimerFd())
, _initSec(initSec)
, _peridocSec(peridocSec)
, _cb(std::move(cb))//注册
, _isStarted(false)
{
}

TimerFd::~TimerFd()
{
    setTimerFd(0, 0);
    close(_tfd);
}

//运行与停止
void TimerFd::start()
{
    struct pollfd pfd;
    pfd.fd = _tfd;
    pfd.events = POLLIN;

    //设定了定时器
    setTimerFd(_initSec, _peridocSec);

    _isStarted = true;
    while(_isStarted)
    {
        int nready = poll(&pfd, 1, 3000);
        if(-1 == nready &&  errno == EINTR)
        {
            continue;
        }
        else if(-1 == nready)
        {
            cerr << "-1 == nready" << endl;
            return;
        }
        else if(0 == nready)
        {
            cout << ">>poll timeout!!!" << endl;
        }
        else
        {
            if(pfd.revents & POLLIN)
            {
                handleRead();//阻塞等待被唤醒
                if(_cb)
                {
                    _cb();//通信之后需要执行的任务
                }

            }
        }
    }
}

void TimerFd::stop()
{
    if(_isStarted)
    {
        _isStarted = false;
        setTimerFd(0, 0);
    }
}

//创建用于通信的文件描述符
int TimerFd::createTimerFd()
{
    int ret = timerfd_create(CLOCK_REALTIME, 0);
    if(ret < 0)
    {
        perror("createTimerFd");
        return ret;
    }

    return ret;
}

//A线程需要执行的read的操作
void TimerFd::handleRead()
{
    uint64_t one = 1;
    ssize_t ret = read(_tfd, &one, sizeof(uint64_t));
    if(ret != sizeof(uint64_t))
    {
        perror("read");
        return;
    }
}

void TimerFd::setTimerFd(int initSec, int peridocSec)
{
    struct itimerspec newValue;
    newValue.it_value.tv_sec = initSec;//起始的秒数
    newValue.it_value.tv_nsec = 0;

    newValue.it_interval.tv_sec = peridocSec;//周期时间
    newValue.it_interval.tv_nsec = 0;

    int ret = timerfd_settime(_tfd, 0, &newValue, nullptr);
    if(ret)
    {
        perror("timerfd_settime");
        return;
    }
}
