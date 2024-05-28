#include "TimerFd.h"
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
        cout << "MyTask  is running!!!" << endl;
    }
};

void test()
{
    MyTask task;
    TimerFd tfd(bind(&MyTask::process, &task), 1, 5);
    thread th(bind(&TimerFd::start, &tfd));//th就是需要执行任务的线程

    sleep(30);

    tfd.stop();
    th.join();
}

int main(int argc, char *argv[])
{
    test();
    return 0;
}

