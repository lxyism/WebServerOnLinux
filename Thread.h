#pragma once
#include<pthread.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<functional>
#include<memory>
#include<string>
#include"noncopyable.h"
#include"CountDownLatch.h"
using namespace std;

class Thread:noncopyable{
public:
    typedef std::function<void()> ThreadFunc;
public:
    //可以阻止不应该允许的经过转换构造函数进行的隐式转换的发生。声明为explicit的构造函数不能在隐式转换中使用。
    explicit Thread(const ThreadFunc&, const std::string& name = std::string());
    ~Thread();
    void  start();
    int join();
    bool started() const  {return started_;}
    pid_t tid() const { return tid_;}
    const std::string& name() const {return name_;}

private:
    void setDefaultName();//设置默认名字
    bool started_;
    bool joined_;
    pthread_t pthreadId_;//线程id
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;  //倒计时器
};
