#include"HttpServer.h"
#include"Util.h"
#include"ListenSocket.h"
#include"base/Logger/Logging.h"
#include"HttpConnection.h"
#include<memory>

using namespace std;
using namespace placeholders;

HttpServer::HttpServer(int port, int thread_num, std::string log_file_name, unsigned int flush_interval):
    listen_event_loop_(new EventLoop),              //event_loop对象：main_Reactor
    thread_pool_(new ThreadPool(thread_num)),       //thread_pool对象：sub_Reactor
    async_logging_(log_file_name, flush_interval),  
    //Acceptor<HttpConnection>:创建的是一个httpconnection的acceptor对象
    acceptor_sptr_(new Acceptor<HttpConnection>(port, EventLoop_WPtr(listen_event_loop_), ThreadPool_WPtr(thread_pool_),this)),
    http_vec_(kMaxHttpConnection)       //最大为10w连接数
    {
        Logger::SetLevel(Logger::Level::DEBUG);     //设定默认的日志等级
        Logger::SetOutput(std::bind(&AsyncLogging::Append, &async_logging_, _1, _2));   //设定输出函数
        Logger::SetFlush(std::bind(&AsyncLogging::Flush, &async_logging_));         //设定刷新函数
        listen_event_loop_->Init();     //通过绑定到eventloop对象上的channel来初始化，初始化状态为：可读
        LOG_DEBUG<<"SIZE:"<<http_vec_.size();   //日志输出一下http连接数
    }
void HttpServer::Start(){
    listen_event_loop_->Loop();
}

void HttpServer::DelConnection(int fd){
    http_vec_[fd].reset();      //unique_ptr释放对象reset()函数，然后重置http_vec_[fd]的值.
}

//把因为新到的连接而创建的connection添加到http_vec_里面
void HttpServer::AddConnection(HttpConnection_UPtr &connection){
    LOG_DEBUG<<http_vec_.size()<<" "<<connection->fd()<<" "<<kMaxHttpConnection;
    http_vec_[connection->fd()] = std::move(connection);
}