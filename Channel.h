#pragma once
#include"base/Logger/Logging.h"
#include<memory>
#include<functional>
#include"base/noncopyable.h"

class EventLoop;
class Channel:public noncopyable, public std::enable_shared_from_this<Channel>{
public:
    typedef EventLoop::EventLoop_WPtr EventLoop_WPtr;
    typedef std::function<void()> WriteCallback;    //回调写函数
    typedef std::function<void()> ReadCallback;     //回调读函数
    typedef std::function<void()> ErrorCallback;    //回调错误函数
    typedef std::function<void()> CloseCallback;    //回调关闭函数
    typedef std::shared_ptr<Channel> Channel_SPtr;
    typedef std::weak_ptr<Channel> Channel_WPtr;
public:
    Channel(int fd):
        events_(0),
        revents_(0),
        fd_(fd),
        eventloop_wptr_(),
        readcallback_(),
        writecallback_(),
        errorcallback_(),
        closecallback_(),
        new_channel_(true){}

    Channel(int fd, EventLoop_WPtr wptr):
        events_(0),
        revents_(0),
        fd_(fd),
        eventloop_wptr_(wptr),
        readcallback_(),
        writecallback_(),
        errorcallback_(),
        closecallback_(),
        new_channel_(true){}

    void SetEventLoop(EventLoop_WPtr wptr){
        eventloop_wptr_ = wptr;
    }

    void SetReadCallback(ReadCallback cb){
        readcallback_ = cb;
    }
    void SetWriteCallback(WriteCallback cb){
        writecallback_ = cb;
    }
    void SetErrorCallback(ErrorCallback cb){
        errorcallback_ = cb;
    }
    void SetCloseCallback(CloseCallback cb){
        closecallback_ = cb;
    }
    void HandleWrite(){
        if(writecallback_)
            writecallback_();
        else
            LOG_WARN <<"Not exit writecallback_";
    }

    void HandleRead(){
        if(readcallback_)
            readcallback_();
        else
            LOG_WARN <<"Not exit readcallback_";
    }

    void HandleError(){
        if(errorcallback_)
            errorcallback_();
        else
            LOG_WARN <<"Not exit errorcallback_";
    }

    void HandleClose(){
        if(closecallback_)
            closecallback_();
        else
            LOG_WARN <<"Not exit closecallback_";
    }

    int fd() const {
        return fd_;
    }
    __uint32_t events() const {
        return events_;
    }
    __uint32_t revents() const {
        return revents_;
    }
    void  SetREvents(__uint32_t revents){
        revents_ = revents;
    }
    void EnableRead(){
        events_ |= kReadEvent;
        Update();
    }
    void EnableWrite(){
        events_ |= kWriteEvent;
        Update();
    }
    void EnableAll(){
        events_ |= (kReadEvent | kWriteEvent);
        Update();
    }
    void DisableRead(){
        events_ &= ~kReadEvent;
        Update();
    }
    void DisableWrite(){
        events_ &= ~kWriteEvent;
        Update();
    }
    void DisableAll(){
        events_ &= kNoneEvent;
        Update();
    }
    void Update();
    void HandleEvents();

private:
    static const int kReadEvent;
    static const int kWriteEvent;
    static const int kNoneEvent;
    __uint32_t events_;     //事件类型
    __uint32_t revents_;    //调用epoll之后返回的事件类型
    const int fd_;
    EventLoop_WPtr eventloop_wptr_;     //通过eventloop_wptr可以向EventLoop中添加当前Channel事件
    WriteCallback writecallback_;
    ReadCallback readcallback_;
    ErrorCallback errorcallback_;
    CloseCallback closecallback_;
    bool new_channel_;
};
