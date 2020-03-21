#pragma once
//定时操作的高度抽象，根据情况设置回调函数,可以根据不同情况设置每个timer超时的回调函数
#include<functional>
#include<memory>
#include"base/noncopyable.h"

class TimerManager;

class Timer:public noncopyable, public std::enable_shared_from_this<Timer>{
public:
    typedef unsigned long long uint64;
    typedef unsigned int uint32;
    typedef std::function<void()> TimerCallback;
    typedef std::weak_ptr<Timer> wptr;
    typedef std::shared_ptr<Timer> sptr;
private:
    uint64 expired_time_; //到期时间
    TimerCallback callback_func_;        //定时器回调函数
    const uint32 interval_time_;    //超时时间间隔，如果是一次性定时器，该值为0
    bool use_;          
    std::weak_ptr<TimerManager> timer_manager_;     //timer管理的指针
public:
    Timer(uint32 interval_time, bool every, func callback_fun, std::weak_ptr<TimerManager> tmwptr);
    void run() //时间到期的执行函数，会调用回调函数以及是否重新计时等操作
    {
        callback_func_();   
        if(interval_time_ != 0)
            Restart();
    }
    void SetInvalid()  //将定时器标记为无效
    {
        use_ = false;
    }
    void Restart();   //重新计时，只在interval_time_不为0才计时
    uint64 GetExpired(){
        return expired_time_;
    }
    bool isValid(){ //定时器是否有效
        return use_;
    }
};