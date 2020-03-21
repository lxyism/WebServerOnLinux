#pragma once
#include<assert.h>
#include<string.h>
#include<string>
#include"noncopyable.h"

class AsyncLogging;
const  int kSmallBuffer = 4000;
const  int kLargeBuffer = 4000*1000;

template <int SIZE>
class FixedBuffer:noncopyable{
    private:
    const char* end() const {
        return data_ + sizeof data_;
    }
    char data_[SIZE];
    char* cur_;

    public:
    FixedBuffer():cur_(data_){}
    ~FixedBuffer(){}

    void LogstreamAppend(const  char* buf, size_t len)
    {
        if(Avail() > static_cast<int>(len))
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    int(Avail() const {
        return static_cast<int>(end() - cur_);
    }
    const char* Data() const{
        return data_;
    }
    int Length() const  {
        return static_cast<int>(cur_ - data_);
    }

    char* Current(){
        return cur_;
    }
    void Add(size_t len){
        cur_ += len;
    }

    void Clear(){   //将当前指针重置为data_[]的首地址
        cur_ = data_;
    }
    void Initzero(){
        memset(data_, 0, sizeof data_);
    }
};

class LogStream : noncopyable{
    public:
    typedef FixedBuffer<kSmallBuffer> Buffer;

    private:
    void staticCheck();

    template<typename T>
    void formateInteger(T);

    Buffer buffer_;

    static const int kMaxNumberSize = 32;
    public:
    LogStream& operator<<(bool v){
        buffer_.LogstreamAppend(v ? "1" : "0");
        return *this;
    }
    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long );
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const char*);
    LogStream& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }
    LogStream& operator<<(double);
    LogStream& operator<<(long double);

    LogStream& operator<<(char v){
        buffer_.LogstreamAppend(&v, 1);
        return *this;
    }
    LogStream& operator<<(const char* str){
        if(str)
            buffer_.LogstreamAppend(str, sizeof(str));
        else
            buffer_.LogstreamAppend("(null)", 6);
        return *this;
    }
    LogStream& operator<<(const std::string& v){
        buffer_.LogstreamAppend(v.c_str(), v.size());
        return *this;
    }
    void LogstreamAppend(const char* data, int len){
        buffer_.LogstreamAppend(data, len);
    }
    const Buffer& getBuffer() const {
        return buffer_;
    }
    void resetBuffer(){
        buffer_.reset();
    }
};