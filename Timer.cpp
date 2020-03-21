#include"Timer.h"
#include"TimerManager.h"

Timer::Timer(uint32 interval_time, bool every, func callback_func, std::weak_ptr<TimerManager> tmwptr):
    expired_time_(GetTimeOfNow()+interval_time),
    callback_func_(callback_func),
    use_(true),
    interval_time_((every = true) ? interval_time : 0),
    timer_manager_(tmwptr)
    {}

void Timer::Restart(){
    expired_time_ += interval_time_;    //过期时间 = 前一个过期时间+间隔时间
    auto timer_sptr = timer_manager_.lock();    
    if(timer_sptr != nullptr){
        timer_sptr->addTimer(timer_manager_(shared_from_this())); 
    } 
}
