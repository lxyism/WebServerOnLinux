#pragma once
#include<ptrhead.h>
#include<queue>
#include<memory>
#include<functional>
#include"base/noncopyable.h"
#include"EventLoop.h"

class Thread:public noncopyable{
public:
    typedef std::function<void()> ThreadFunction;
private:
struct ThreadData{
    typedef std::function<void()> func;
    ThreadFunction thread_function_;
    ThreadData(ThreadFunction thread_function):
        thread_function_(thread_function){}
    };

    pthread_t thread_id_;
    ThreadData thread_data_;
    static void* ThreadRunning(void* p);
public:
    Thread(ThreadFunction func);
    
    pthread_t GetThreadId() const {
        return thread_id_;
    }
    void join(){
        pthread_join(thread_id_, NULL);
    }
};