#include"EventLoop.h"
#include"TimerManager.h"
#include<functional>


IgnoreSigPipe  initObj; 

EventLoop::EventLoop():
    epoll_(),
    is_quit_(false),
    mutex_(),
    new_channel_queue_(),
    loop_(new EventLoop),
    notify_event_(std::bind(&EventLoop::NewConnectHandle, this)),
    timer_manager_(new TimerManager){}

//EventLoop对象的初始化
void EventLoop::Init(){
    notify_event_.SetEventLoop(shared_from_this()); //通过设定Channel来对eventloop进行初始化
    notify_event_.EnableRead();     //并使这个channel可以读EnableRead
}

//事件循环，该函数不能跨线程使用
//只能在创建该对象的线程中使用
void EventLoop::Loop(){
    while(!is_quit_){
        uint64 nearest_time = timer_manager_->getNearestTime(); //用小根堆的返回时间来确定处理的线程
        if(nearest_time == 0){
            nearest_time = -1;  //默认等待永久时间
        }else
            nearest_time = nearest_time - GetTimeOfNow();
        //获取活跃套接字
        epoll_.HandleActiveEvents(nearest_time);
        //处理时间事件
        timer_manager_->doTimeEvent();
    }
    printf("close server\n");
}

void EventLoop::NewConnectHandle()
{
	LOG_DEBUG << "NewConnectHandle";
    MutexGuard mutex_guard(mutex_);
    int n = new_channel_queue_.size();
    for(int i = 0;i < n;i++)
    {
		Channel_WPtr new_channel = new_channel_queue_.front();  //最开始的channel对象
		new_channel_queue_.pop();           //弹出来
		auto channel_sptr = new_channel.lock();     //升级锁
		if(channel_sptr)
			channel_sptr->EnableRead();     //使之可读
    }
}

void EventLoop::AddNewChannel(Channel_WPtr channel){
    LOG_DEBUG<<"addNewChannel";
    MutexGuard mutex_guard(mutex_);
    new_channel_queue_.push(channel);
    WakeUp();
}

std::weak_ptr<TimerManager> EventLoop::GetTimerManager(){
    return std::weak_ptr<TimerManager>(timer_manager_);
}

void EventLoop::AddTimer(std::weak_ptr<Timer> timer){
    timer_manager_->addTimer(timer);
}
