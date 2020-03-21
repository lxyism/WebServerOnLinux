#pragma once
//用于listen、accept，并调用会点函数来处理新的连接
//accept之后，如果有新的连接到来，会调用handleRead()函数，在这个函数接收连接之后。
//在HandleRead()函数中每接受一个新连接，之后调用处理新连接的回调函数。
//客户不直接使用Acceptor,它封装在httpserver里面
#include"Channel.h"
#include"EventLoop.h"
#include"Socket.h"
#include"ThreadPool.h"
#include"HttpServer.h"
#include<vector>
#include<sys/socket.h>

template<typename T>
class Acceptor :public noncopyable{
public:
    typedef EventLoop::EventLoop_WPtr EventLoop_WPtr;
    typedef Channel::Channel_WPtr Channel_WPtr;
    typedef ThreadPool::ThreadPool_WPtr ThreadPool_WPtr;

    Acceptor(int port, EventLoop_WPtr wptr, ThreadPool_WPtr threadpool_wptr, HttpServer* server);
    void NewConnect();
    ~Acceptor(){
        if(fd_>0)
            close(fd_);
    } 
    void EnableRead(){
        channel_sptr_->EnableRead();
    }
private:
    int fd_;
    Channel_SPtr channel_sptr_;
    ThreadPool_WPtr threadpool_wptr_;
    HttpServer* httpserver_;
};

template <typename T>
Acceptor<T>::Acceptor(int port, EventLoop_WPtr wptr, ThreadPool_WPtr threadpool_wptr, HttpServer* server):
    fd_(socket_bind_listen(port)),      //客户端监听的listen_fd传到fd_
    channel_sptr_(new Channel(fd_, wptr)),
    threadpool_wptr_(threadpool_wptr),
    httpserver_(server)
    {
        channel_sptr_->SetReadCallback(std::bind(&Acceptor<T>::NewConnect, this));    //这儿的回调函数就是创建新的连接
        channel_sptr_->EnableRead();
    }


template <typename T>
void Acceptor<T>::NewConnect(){
    while(1){
        int connfd;
        //服务器端使用accept函数返回接收的端口connfd
        if( (connfd = accept(fd_, NULL, NULL)) <= 0)
        {
            if(errno = EINTR){//系统中断打断
                continue;   //继续读
            }
            else if(errno == EAGAIN){//套接字队列读取完
                break;//直接返回
            }else{
                LOG_ERROR<<" accept error！";
                break;
            }
        }
        else{
            std::unique_ptr<T> client(new T(connfd, httpserver_));
            auto thread_pool = threadpool_wptr.lock();
            if(thread_pool){
                thread_pool->AddNewConnect(client->channel());
            }
            httpserver_->AddConnection(client);
        }
    }
}