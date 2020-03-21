//epoll类，主要实现事件的添加、修改、删除和事件获取
#pragma once
#include"base/Logger/Logging.h"
#include"Channel.h"
#include<map>
#include<vector>
#include<memory>

class Epoll:public noncopyable{
public:
    typedef Channel::Channel_WPtr Channel_WPtr;
    typedef Channel::Channel_SPtr Channel_SPtr;
    typedef unsigned long long uint64;

public:
    Epoll();
    ~Epoll();
    void AddNewChannel(Channel_WPtr channel_wptr);
    void UpdateChannel(Channel_WPtr channel_wptr);
    void DelChannel(Channel_WPtr channel_wptr);
    void HandleActiveEvents(uint64 ms);
private:
    static const int kMaxFd = 100000;  //每个epoll最多只处理100000连接
    static const int kActiveEventSize = 1000;
    int epoll_fd_;
    std::vector<struct epoll_event> active_events_;
    std::vector<Channel_WPtr> fd2Channel_;
    std::vector<Channel_WPtr> active_handle_;
    void EpollAdd(int fd, struct epoll_event &event);
    void EpollMod(int fd, struct epoll_event &event);
    void EpollDel(int fd);

};