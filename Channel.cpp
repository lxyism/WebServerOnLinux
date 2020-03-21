#include"Channel.h"
#include"EventLoop.h"

const int Channel::kReadEvent = EPOLLIN | EPOOLPRI | EPOLLET;
const int Channel::kWriteEvent = EPOLLOUT | EPOLLET;
const int Channel::kNoneEvent = 0;

void Channel::Update(){
    auto sptr = eventloop_wptr_.lock();
    if(sptr){
        auto this_sptr = shared_from_this();
        sptr->UpdateChannel(this_sptr, new_channel_);
        new_channel_ = false;
    }
}

void Channel::HandleEvents(){
    if(revents_ & (EPOLLERR | EPOLLHUP))    //本端出错
    {
        LOG_INFO<<"EPOLLERR | EPOLLHUP";
        HandleError();
    }
    if(revents_ & (EPOLLIN | EPOLLPRI)) //新连接或数据到来
    {
        LOG_INFO<<"EPOLL | EPOLLPRI";
        HandleRead();
    }
    if(revents_ & EPOLLOUT) //可写
    {
        LOG_INFO<<"EPOLLOUT";
        HandleWrite();
    }
}