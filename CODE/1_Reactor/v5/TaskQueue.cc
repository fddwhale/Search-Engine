#include "TaskQueue.h"

TaskQueue::TaskQueue(size_t queSize)
: _queSize(queSize)
, _que()
, _mutex()
, _notFull()
, _notEmpty()
, _flag(true)
{

}

TaskQueue::~TaskQueue()
{

}

//生产数据
void TaskQueue::push(ElemType &&value)
{
    unique_lock<mutex> autoLock(_mutex);

    while(full())
    {
        _notFull.wait(autoLock);
    }
    _que.push(std::move(value));
    _notEmpty.notify_one();
}

//获取数据
ElemType TaskQueue::pop()
{
    unique_lock<mutex> autoLock(_mutex);
    while(empty() && _flag)
    {
        _notEmpty.wait(autoLock);
    }

    if(_flag)
    {
        ElemType tmp = _que.front();
        _que.pop();
        _notFull.notify_one();

        return tmp;
    }
    else
    {
        return nullptr;
    }
}

bool TaskQueue::full() const
{
    return _que.size() == _queSize;
}

bool TaskQueue::empty() const
{
    return _que.size() == 0;
}

//将所有等在在_notEmpty条件变量上的线程唤醒
void TaskQueue::wakeup()
{
    _flag = false;
    _notEmpty.notify_all();
}
