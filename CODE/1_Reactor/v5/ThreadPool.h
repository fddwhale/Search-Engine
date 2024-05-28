#ifndef __THREADPOLL_H__
#define __THREADPOLL_H__

#include "TaskQueue.h"
#include <vector>
#include <thread>
#include <functional>

using std::vector;
using std::thread;
using std::function;

using Task = function<void()>;

class ThreadPool
{
public:
    ThreadPool(size_t threadNum, size_t queSize);
    ~ThreadPool();

    //线程池的启动与停止
    void start();
    void stop();

    //任务的添加与获取
    void addTask(Task &&task);
private:
    Task getTask();
    //线程池交给工作线程执行的任务
    void doTask();

private:
    size_t _threadNum;//子线程的数目
    size_t _queSize;//任务队列的大小
    vector<thread> _threads;//存放子线程的容器
    TaskQueue _taskQue;//存放任务的数据结构
    bool _isExit;//标识线程池是否退出的标志
};

#endif
