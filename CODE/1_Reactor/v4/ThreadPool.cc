#include "ThreadPool.h"
#include <iostream>

using std::cout;
using std::endl;

ThreadPool::ThreadPool(size_t threadNum, size_t queSize)
: _threadNum(threadNum)
, _queSize(queSize)
, _taskQue(_queSize)
, _isExit(false)
{
    //为了防止vector频繁扩容的问题，预留空间
    _threads.reserve(_threadNum);
}

ThreadPool::~ThreadPool()
{
}

//线程池的启动与停止
void ThreadPool::start()
{
    //将所有的工作线程创建出来，同时也要启动起来
    for(size_t idx = 0; idx != _threadNum; ++idx)
    {
        _threads.push_back(thread(&ThreadPool::doTask, this));
    }
}

//Q1:线程池并没有保证任务执行完，就已经将线程池退出来了?
//A1:因为任务没有执行完，主线程就已经回收了子线程（工作线程）
//
//Q2：任务可以执行完，但是线程池又无法退出？
//A2: 线程池之所以没有退出，是因为工作线程与主线程的配合问题
//也就是主线程设置_isExit的时序问题，而导致了子线程处于睡眠
//状态，解决方法是：在回收之前，将所有可能睡眠的线程唤醒之后
//再进行回收

void ThreadPool::stop()
{
    //只要工作线程没有将任务执行完，就不能向下执行
    while(!_taskQue.empty())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    _isExit = true;

    //将所有等待在_notEmpty条件变量上的线程唤醒（工作线程唤醒）
    _taskQue.wakeup();

    //将所有的工作线程进行回收
    for(auto &th : _threads)
    {
        th.join();
    }
}

//任务的添加与获取
void ThreadPool::addTask(Task &&task)
{
    if(task)
    {
        _taskQue.push(std::move(task));
    }
}

Task ThreadPool::getTask()
{
    return _taskQue.pop();
}

//线程池交给工作线程执行的任务
void ThreadPool::doTask()
{
    while(!_isExit)
    {
        Task taskcb = getTask();
        if(taskcb)
        {
            //回调的时候就是将注册的process进行了执行
            taskcb();//回调的执行
            /* ptask->process();//体现多态 */
        }
        else
        {
            cout << "task == nullptr " << endl;
        }
    }
}
