#pragma once
//同步计数器
//既用于所有子线程等待主线程的同步计时，也可用于主线程等待子线程初始化完毕才开始工作。
//其中使用RAⅡ方法封装Mutex类。
#include"Condition.h"
#include"Mutex.h"
#include"noncopyable.h"

//CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后，外层的start才返回
class CountDownLatch : noncopyable{
    private:
    mutable Mutex mutex_;
    Condition condition_;
    int count_;
    
    public:
    explicit CountDownLatch(int count):mutex_(),condition_(mutex_),count_(count){}
    void wait(){
        MutexGuard lock(mutex_);
        while(count_>0)
            condition_.wait();
    }
    void countDown(){
        MutexGuard lock(mutex_);
        --count_;
        if(count_ == 0)
            condition_.notifyAll();
    }
};