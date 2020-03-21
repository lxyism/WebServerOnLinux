#include"Epoll.h"
#include<assert.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>

Epoll::Epoll():
    active_events_(kActiveEventSize),
    fd2Channel_(kMaxFd)
    {
        if((epoll_fd_ = epoll_create(1)) < -1){
            LOG_ERROR<<"epoll_create error";
            exit(0);
        }
    }

Epoll::~Epoll(){
    close(epoll_fd_);
}

//添加新的channel事件
void Epoll::AddNewChannel(Channel_WPtr channel_wptr){
    auto channel_sptr = channel_wptr.lock();
    if(channel_sptr){
        int fd = channel_sptr->fd();
        assert(fd2Channel_[fd] == nullptr);
        fd2Channel_[fd] = channel_wptr;
        struct epoll_event event;
        //将channel和对应的fd设置到event结构体里面进行epolladd
        event.events = channel_sptr->events();
        event.data.fd = fd;
        //
        EpollAdd(fd, event);    
    }
}

void Epoll::UpdateChannel(Channel_WPtr channel_wptr){
    auto channel_sptr = channel_wptr.lock();
    if(channel_sptr){
        int fd = channel_sptr->fd();
        assert(fd2Channel_[fd] != nullptr);
        struct epoll_event event;
        event.events = channel_sptr->events();
        event.data.fd = fd;
        EpollMod(fd, event);
    }
}

void Epoll::DelChannel(Channel_WPtr channel_wptr){
    auto channel_sptr = channel_wptr.lock();
    if(channel_sptr){
        int fd = channel_sptr->fd();
        assert(fd2Channel_[fd] != nullptr);
        fd2Channel_[fd].reset();
        EpollDel(fd);
    }
}

void Epoll::EpollAdd(int fd, struct epoll_event &event){
    int ret;
    //epoll的事件注册函数，它不同与select()是在监听事件时告诉内核要监听什么类型的事件，而是在这里先注册要监听的事件类型。
    //EPOLL_CTL_ADD：注册新的fd到event中；
    //EPOLL_CTL_MOD：修改已经注册的fd的监听事件；
    //EPOLL_CTL_DEL：从event中删除一个fd；
    if((ret=epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event)) < 0){
        LOG_ERROR<<"epoll_ctl_add error: ";
        exit(0);
    }
}

void Epoll::EpollMod(int fd, struct epoll_event &event){
    int ret;   
    if((ret=epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event)) == -1){
        LOG_ERROR<<"epoll_ctl_mod error: ";
        exit(0);
    }
}

void Epoll::EpollDel(int fd){
    int ret;   
    if((ret=epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, NULL)) < 0){
        LOG_ERROR<<"epoll_ctl_del error: ";
        exit(0);
    }    
}

void Epoll::HandleActiveEvents(uint64 ms){
    int nready;
    //等待EPOLL事件的发生，相当于监听，至于相关的端口，需要在初始化EPOLL的时候绑定。
    //arg1：fd；arg2：回传代处理事件；arg3：最大处理事件；arg4：超时时间
    if((nready = epoll_wait(epoll_fd_, active_events_.data(), active_events_.size(), ms)) < 0){
        LOG_ERROR<<"epoll_wait error: ";
        exit(0);
    }
    for(int i = 0; i<nready;++i){
        auto channel_sptr = fd2Channel_[active_events_[i].data.fd].lock();
        if(channel_sptr){
            channel_sptr->SetREvents(active_events_[i].events); //设置返回事件
            channel_sptr->HandleEvents();   //Channel按照返回的EPOLL事件进行处理
        }
    }
}