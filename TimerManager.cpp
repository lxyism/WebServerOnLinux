#include"TimerManager.h"
#include<sys/time.h>
#include<stdio.h>
#include<sys/timerfd.h>
#include<fcntl.h>

uint64 GetTimeOfNow(){
    struct timeval tm;
    ::gettimeofday(&tm, NULL);
    uint64 time_ms = tm.tv_sec*1000 + tm.tv_usec/1000;
    return time_ms;
}

void TimerManager::doTimeEvent(){
    uint64  now_ms = GetTimeOfNow();    //获取现在时间
    while(!timer_queue_.empty()){    //时间事件队列非空
        auto timer_sptr = timer_queue_.top().lock();    //去除时间堆上的最小值，加锁
        if(timer_sptr != nullptr && timer_sptr->isValid()){ 
            if(now_ms < timer_sptr->GetExpired())       //获取过期时间
                break;  
            timer_sptr->run();  
        }
        timer_queue_.pop();
    }
}

uint64 TimerManager::getNearestTime(){
    while(!timer_queue_.empty()){
        auto timer_sptr = timer_queue_.top().lock();
        if(timer_sptr != nullptr && timer_sptr->isValid()){
            return timer_sptr->GetExpired();
        }
    }
    return 0;
}