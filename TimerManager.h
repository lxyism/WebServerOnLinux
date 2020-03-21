#pragma once
#include<vector>
#include<queue>
#include"base/noncopable.h"
#include"Timer.h"
#include"Channel.h"
#include"EventLoop.h"

//能够快速根据当前时间找到已到期的定时器，也要高效的添加和删除Timer，
//可以用二叉搜索树、map或set，我们用的是小根堆，Timerqueue关注最早
//到期的定时器，getExpired返回所有的超时定时器列表，使用low_bound
//返回第一个>=超时定时器的迭代器。

typedef unsigned long long uint64;
typedef unsigned int uint32;

uint64 GetTimeOfNow();
class TimerManager:public noncopyable{
public:
    typedef Channel::Channel_SPtr Channel_SPtr;
    typedef Channel::EventLoop_WPtr EventLoop_WPtr;

private:
    typedef std::weak_ptr<Timer> timer_wptr;
    class cmp{
    public:
        //优先级队列的默认比较函数为<，默认为大根堆
        bool operator()(const timer_wptr &t1, const timer_wptr &t2){
            if(t1.lock() != nullptr && t2.lock())
                //GetExpired()返回的是到期时间，返回true表示：左边的优先级小于右边，所以是小根堆
                return t1.lock()->GetExpired() > t2.lock()->GetExpired();
            else
                return (t1.lock() == nullptr) ? false : true;
        }
    };
    std::priority_queue<timer_wptr, std::vector<timer_wptr>, cmp> timer_queue_; //按照到期时间来确定优先级
public:
    //将当前的时间加到时间堆里面
    void addTimer(timer_wptr timer){
        timer_queue_.push(timer);
    }
    void doTimeEvent();
    uint64 getNearestTime();    //获取最紧发生事件的时间，返回时间为绝对时间，单位ms，如果返回0，说明没有时间事件
};