#pragma once
#include"EventLoop.h"
#include"base/Logger/AsyncLoogging.h"
#include"HttpConnection.h"
#include"ThreadPool.h"
#include<vector>

template<typename T>
class Acceptor;
class HttpServer:public noncopyable{
public:
    typedef EventLoop::EventLoop_WPtr EventLoop_WPtr;
    typedef EventLoop::EventLoop_SPtr EventLoop_SPtr;
    typedef std::shared_ptr<ThreadPool> ThreadPool_SPtr;
    typedef std::weak_ptr<ThreadPool> ThreadPool_WPtr;
    typedef std::shared_ptr<Acceptor<HttpConnection>> Acceptor_SPtr;
    typedef std::unique_ptr<HttpConnection> HttpConnection_UPtr;

private:
    EventLoop_SPtr listen_event_loop_;
    ThreadPool_SPtr thread_pool_;
    AsyncLogging async_logging_;
    Acceptor_SPtr acceptor_sptr_;
    std::vector<HttpConnection_UPtr> http_vec_;     //unique_ptr释放对象用reset()函数
    static const int kMaxHttpConnection = 100000;

public:
    HttpServer(int port, int thread_num, std::string log_file_name, unsigned int flush_interval);
    void Start();       //开始处理线程
    void DelConnection(int fd);
    void AddConnection(HttpConnection_UPtr &connection);
};
