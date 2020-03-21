#include"LogStream.h"
#include<assert.h>
#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<algorithm>
#include<limits>

using namespace std;
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

template<typename T>
size_t convert(char buf[], T value){
    T i = value;
    char* p = buf;
    do{
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    }while(i != 0)

    if(value < 0){
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template <typename T>
void LogStream::formatInteger(T v){
    //buffer容不下kMaxNumbericSize个字符的话就会被直接丢弃
    if(buffer_.Avail() >= kMaxNumbericSize){
        size_t len = convert(buffer_.Current(), v);
        buffer_.Add(len);
    }
}

LogStream& LogStream::operator<<(short v){
    *this<<static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v){
    *this<<static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v){
    formateInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int  v){
    formateInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v){
    formateInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long v){
    formateInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v){
    formateInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v){
    formateInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(double v){
    if(buffer_.Avail() >= kMaxNumberSize) {
        int len = snprintf(buffer_.Current(), kMaxNumberSize, "%.12g", v);
        buffer_.Add(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(long double v){
    if(buffer_.Avail() >= kMaxNumberSize) {
        int len = snprintf(buffer_.Current(), kMaxNumberSize, "%.12g", v);
        buffer_.Add(len);
    }
    return *this;
}