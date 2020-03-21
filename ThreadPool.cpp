#include"ThreadPool.h"
#include<functional>

ThreadPool::ThreadPool(int thread_num):next_loop_(0){
    for(int i=0; i< thread_num; ++i){
        std::shared_ptr<EventLoop> event_ptr(new EventLoop);
        event_ptr->Init();
        std::unique_ptr<Thread> thread_uptr(new Thread(std::bind(&EventLoop::loop, event_ptr.get())));
        thread_uptr_vec_.push_back(std::move(thread_uptr));
        event_loop_sptr_vec_.push_back(std::move(event_ptr));
    }
}

ThreadPool::~ThreadPool(){
    printf("ThreadPool destruct\n");
    for(int i=0; i< event_loop_sptr_vec_.size();++i){
        event_loop_sptr_vec_[i].Quit();
        thread_uptr_vec_[i].join();
    }
}

void ThreadPool::AddNewConnect(Channel_WPtr channel_wptr){
    auto channel_sptr = channel_wptr.lock();
    if(channel_sptr){
        //把event_lopp_对象
        channel_sptr->SetEventLoop(event_loop_sptr_vec_[next_loop_]);
        channel_sptr->EnableRead();
        //round-robin
        event_loop_sptr_vec_[next_loop_++]->AddNewChannel(channel_wptr);
        //如果event_loop_sptr_vec满了，直接把next_loop_丢弃
        next_loop_ = (next_loop_ >= event_loop_sptr_vec_.size() ? 0 : next_loop_);
    }
}