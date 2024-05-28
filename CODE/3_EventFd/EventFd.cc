#include "EventFd.h"
#include <unistd.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

EventFd::EventFd(EventFdCallback &&cb)
: _evtfd(createEventFd())
, _cb(std::move(cb))//注册
, _isStarted(false)
{
}

EventFd::~EventFd()
{
    close(_evtfd);
}

//运行与停止
void EventFd::start()
{
    struct pollfd pfd;
    pfd.fd = _evtfd;
    pfd.events = POLLIN;

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

void EventFd::stop()
{
    _isStarted = false;
}

//创建用于通信的文件描述符
int EventFd::createEventFd()
{
    int ret = eventfd(10, 0);
    if(ret < 0)
    {
        perror("eventfd");
        return ret;
    }

    return ret;
}

//A线程需要执行的read的操作
void EventFd::handleRead()
{
    uint64_t one = 1;
    ssize_t ret = read(_evtfd, &one, sizeof(uint64_t));
    if(ret != sizeof(uint64_t))
    {
        perror("read");
        return;
    }
}
//用于唤醒线程的函数
void EventFd::wakeup()
{
    uint64_t one = 1;
    ssize_t ret = write(_evtfd, &one, sizeof(uint64_t));
    if(ret != sizeof(uint64_t))
    {
        perror("write");
        return;
    }
}
