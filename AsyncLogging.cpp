#include"AsyncLogging.h"
#include<sys/fcntl.h>
#include<unistd.h>
#include<chrono>//duration/time_point/clock
using namespace std;

AsyncLogging::LogFile::LogFile(std::string file_name){
    fd_ = ::open(file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND);
    if(fd_ < 0)
    {
        printf("Can't open log file!\n");
        exit(0);
    }
}

AsyncLogging::LogFile::~LogFile()
{
    if(fd_ > 0)
    {
        Flush();
        close(fd_);
    }
}

void AsyncLogging::LogFile::WriteLog(const char* data, size_t len){
    //阻塞write也需要考虑写入字节小于指定字节问题，原因：
    //1、物理存储介质空间不够；
    //2、被信号中断
    ssize_t writed_size = 0;
    while(writed_size != len){
        ssize_t ret = write(fd_, data+ writed_size, len - writed_size);
        if(ret<0)
        {
            printf("error: write file error\n");
            exit(0);
        }
        writed_size += ret;
    }
}

void AsyncLogging::LogFile::Flush()
{
    fsync(fd_);
}

void AsyncLogging::Append(const char* log_msg, int len){
    std::lock_guard<std::mutex> mutex_guard(mutex_);
    if(current_buffer_->Avail() > len)//正常情况下
        current_buffer_->LogstreamAppend(log_msg, len);
    else//当前buffer被填满，应当通知写线程及时写
    {
        buffer_vec_.push_back(std::move(current_buffer_));
        if(alternate_buffer_)//还有备用buffer
        {
            current_buffer_ = std::move(alternate_buffer_);//启用备用buffer
        }
        else//这种情况说明写日志频率很高，备用buffer也被写满，考虑扩容bufffer
        {
            current_buffer_.Clear(new LargeBuffer);
        }
        current_buffer_->LogstreamAppend(log_msg, len);
        condition_.notify_all();
    }
}

void AsyncLogging::ThreadFunc()//写日志线程
{
    //两个新buffer，用以交换日志buffer，减少临界区
    LargeBufferPtr new_buffer1(new LargeBuffer),new_buffer2(new LargeBuffer);
    new_buffer1->InitZero();
    new_buffer2->InitZero();
    BufferPtrVec write_buffer_vec;
    write_buffer_vec.reserve(10);
    //必须使用do while，防止主线程结束，日志线程直接结束
    do{
        {
            std::unique_lock<std::mutex> mutex_guard(mutex_);
            if(buffer_vec_.empty())//基于时间等待
            {
                std::chrono::seconds sec(3);
                condition_.wait_for(mutex_guard, sec);
            }
            buffer_vec_.push_back(std::move(current_buffer_));
            swap(write_buffer_vec, buffer_vec_);
            current_buffer_ = std::move(new_buffer1);
            if(!alternate_buffer_)
            {
                alternate_buffer_ std::move(new_buffer2);
            }
        }
        if(write_buffer_vec.size() > 10)
        {
            char warn_msg[100] = "log message is to many\n";
            log_file_.WriteLog(warn_msg, strlen(warn_msg));
        }

        for(auto &buffer: write_buffer_vec)
        {
            log_file_.WriteLog(buffer->data(), buffer->length());
        }
        if(write_buffer_vec.size() > 2)
            write_buffer_vec.resize(2);
        new_buffer1 = std::move(write_buffer_vec.back());
        write_buffer_vec.pop_back();
        new_buffer1->Clear();
        if(!new_buffer2)
        {
            new_buffer2 = std::move(write_buffer_vec.back());
            new_buffer2->Clear();
        }
        write_buffer_vec.Clear();
        log_file_.Flush();
    }while(running_);
}