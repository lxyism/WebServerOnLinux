#pragma once
#include<string>
#include"LogStream.h"
#include<memory>
#include<vector>
#include<thread>
#include<mutex>
#include<condition_variable>

using namespace std;
class AsyncLogging{
    typedef Buffer<1024*1024*4> LargeBuffer;
    typedef std::unique_ptr<LargeBuffer> LargeBufferPtr;
    typedef std::vector<LargeBuffer> BufferPtrVec;

    class LogFile{
    private:
        int fd_;
    public:
        LogFile(std::string file_name);
        ~LogFile();
        void WriteLog(const char* data, size_t len);
        inline void Flush();
    };
private:
    const std::string log_file_name_;
    const unsigned int flush_interval_;
    LogFile log_file_;
    LargeBufferPtr current_buffer_;
    LargeBufferPtr alternate_buffer_;
    BufferPtrVec buffer_vec_;
    bool running_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable condition_;
public:
    AsyncLogging(std::string file_name, unsigned int flush_interval = 3):
        log_file_name_(file_name),
        flush_interval_(flush_interval),
        log_file_(log_file_name_),
        current_buffer_(new LargeBuffer),
        alternate_buffer_(new LargeBuffer),
        buffer_vec_(),
        running_(true),
        thread_(std::bind(&AsyncLogging::ThreadFunc, this)),
        mutex_(),
        condition_()
        {
            current_buffer_->InitZero();
            alternate_buffer_->InitZero();
            buffer_vec_.reserve(10);
        }
    void Append(const char* log_msg, int len);
    void Flush(){
        condition_.notify_all();
    }
    void ThreadFunc();
    ~AsyncLogging()
    {
        printf("记录日志成功!!!");
        running_ = false;
        thread_.join();
    }

};