//此类为线程间消息通知类
#pragma once
#include<functional>
#include<memory>
#include<unistd.h>
#include"Util.h"
#include"sys/types.h"
#include<sys/eventfd.h>
#include"Channel.h"
#include"base/Logger/Logging.h"

int CreatEventFd();

class NotifyEvent{
public:
    typedef Channel::EventLoop_WPtr EventLoop_WPtr;
    typedef Channel::Channel_SPtr Channel_SPtr;
    typedef std::function<void()> NewConnectHandler;
    NotifyEvent(NewConnectHandler func):fd_(CreatEventFd()),
        channel_sptr_(new Channel(fd_)),
        new_connect_handler_(func)
        {
            channel_sptr_->SetReadFunc(std::bind(&NotifyEvent::NewConnection, this));
        }
    void SetEventLoop(EventLoop_WPtr wptr){
        channel_sptr_->SetEventLoop(wptr);
    }
    void EnableRead(){
        channel_sptr_->EnableRead();
    }
    void WriteHandle(){
        uint64_t one =1;
        size_t n = Writen(fd_, &one, sizeof(one));
        if(n != sizeof(one)){
            LOG_ERROR<<"NotifyEvent written error: ";
        }
    }
    void ReadHandle(){
        uint64_t one = 1;
        size_t n = Readn(fd_, &one, sizeof(one));
        if(n != sizeof(one)){
            LOG_ERROR<<"NotifyEvent readen error: "<<n<<"bytes instead of 8";
        }
    }

    void NewConnection(){
        LOG_DEBUG<<"NewConnection";
        ReadHandle();
        new_connect_handler_();
    }
private:
    int fd_;
    Channel_SPtr channel_sptr_;
    NewConnectHandler new_connect_handler_;
};