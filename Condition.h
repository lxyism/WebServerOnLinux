//用于封装线程的条件变量
#pragma once
#include"Mutex.h"
#include"noncopyable.h"
#include<pthread.h>

class Condition : noncopyable{
private:
    Mutex &mutex_;
    pthread_cond_t cond_;

public:
    Condition(Mutex& mutex):mutex_(mutex) {pthread_cond_init(&cond_, NULL);}
    ~Condition(){pthread_cond_destory(&cond_);}
    void Wait(){pthread_cond_wait(&cond_, &mutex_.Get());}
    void TimeWait(unsigned int wait_seconds)   //只支持秒级精度
    {
        struct timeval tm;
        ::gettimeofday(&tm, NULL);
        timespec sp;
        sp.tv_sec = (tm.tv_sec + wait_seconds);
        sp.tv_nsec = tm.tv_usec * 1000;
        pthread_cond_timedwait(&cond_, &mutex_.Get(), &sp);
    }
    void Notify(){pthread_cond_signal(&cond_);}
    void NotifyAll(){pthread_cond_broadcast(&cond_);}
};