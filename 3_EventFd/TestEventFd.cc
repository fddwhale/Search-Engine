#include "EventFd.h"
#include <unistd.h>
#include <iostream>
#include <functional>
#include <thread>

using std::cout;
using std::endl;
using std::bind;
using std::thread;

class MyTask
{
public:
    void process() 
    {
        //....
        cout << "MyTask  process is running!!!" << endl;
    }
};

void test()
{
    MyTask task;
    EventFd efd(bind(&MyTask::process, &task));
    /* efd.start(); */
    thread th(bind(&EventFd::start, &efd));//th就是需要执行任务的线程

    //为了让事件可以多被执行几次，并且主线程可以子线程执行之后，
    //多次唤醒，可以将wakeup对应的主线程写在while循环中，并且
    //每执行一次就睡眠
    int cnt = 10;
    while(cnt--)
    {
        efd.wakeup();//另外一个线程需要执行wakeup唤醒
        sleep(1);
    }

    efd.stop();
    th.join();
}

int main(int argc, char *argv[])
{
    test();
    return 0;
}

