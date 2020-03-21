//主要负责线程的创建、销毁、和任务分发
#pragma once
#include"Channel.h"
#include"EventLoop.h"
#include"Thread.h"
#include"base/noncopyable.h"
#include<memory>
#include<vector>

class ThreadPool:public noncopyable{
public:
    typedef std::shared_ptr<ThreadPool> ThreadPool_SPtr;
    typedef std::weak_ptr<ThreadPool> ThreadPool_WPtr;
    typedef EventLoop::EventLoop_WPtr EventLoop_WPtr;
    typedef Channel::Channel_WPtr Channel_WPtr;
private:
    //Thread的一个vector数组
    std::vector<std::unique_ptr<Thread>> thread_uptr_vec_;
    //eventloop对象的一个数组
    std::vector<std::shared_ptr<EventLoop>> event_loop_sptr_vec_;
    int next_loop_; //下一个事件
public:
    ThreadPool(int thread_num = 10);
    ~ThreadPool();
    void AddNewConnect(Channel_WPtr channel_wptr);
};