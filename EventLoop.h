//EventLoop类为epoll事件循环类，每次循环需要处理套接字事件、时间事件
#pragma once
#include"Channel.h"
#include"Epoll.h"
#include"NotifyEvent.h"
#include"base/Mutex.h"
#include"Timer.h"
#include<queue>
#include<signal.h>

class TimerManager;
class IgnoreSigPipe 
{ 
public: 
    IgnoreSigPipe() 
    { 
        ::signal(SIGPIPE,SIG_IGN); 
    } 
}; 

//one loop one thread，即一个线程只有一个EventLoop，所以定义一个__thread类型的指针变量，
//初始化为NULL，当有EventLoop对象生成的时候，将this赋值给此变量，用以保证one loop one thread
class EventLoop:public noncopyable, public std::enable_shared_from_this<EventLoop>
{
public:
    typedef std::shared_ptr<EventLoop> EventLoop_SPtr;
    typedef std::weak_ptr<EventLoop> EventLoop_WPtr;
    typedef Channel::Channel_WPtr Channel_WPtr;
    typedef Channel:Channel_SPtr Channel_SPtr;
    typedef std::function<void()> Func;
public:
    EventLoop();
    void Init();
    void Loop();
    void NewConnectHandle();
    void AddNewChannel(Channel_WPtr channel_wptr);
    //唤醒处理写事件
    inline void WakeUp(){
        notify_event_.WriteHandle();
    }
    //析构的时候开始处理写事件
    inline void Quit(){
        //printf("quit\n");
        //LOG_INFO<<"Quit EventLoop";
        is_quit_ = true;
        WakeUp();
    }

    void UpdateChannel(Channel_WPtr channel_wptr, bool new_channel){
        if(new_channel){
            epoll_.AddNewChannel(channel_wptr);
        }else{
            epoll_.UpdateChannel(channel_wptr);
        }
    }

    void DelChannelEvent(Channel_WPtr channel_wptr){
        epoll_.DelChannel(channel_wptr);
    }

    std::weak_ptr<TimerManager> GetTimerManager();
    void AddTimer(std::weak_ptr<Timer> timer);
    
private:
    //EventLoop loop_;
    Epoll epoll_;
    bool is_quit_;
    Mutex mutex_;
    std::queue<Channel_WPtr> new_channel_queue_;
    NotifyEvent notify_event_;
    std::shared_ptr<TimerManager> timer_manager_;
};