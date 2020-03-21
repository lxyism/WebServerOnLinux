#pragma once
#include <stdint.h>//定义了几种扩展的整数类型和宏:intN_t,

namespace CurrentThread{
    //internal
    extern __thread int t_cachedTid;//需要引用全局变量，所以必须声明全局变量
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char* t_threadName;
    void cacheTid();
    inline int tid(){
        //允许程序员将最有可能执行的分支告诉编译器,t_cacheTid == 1可能性很大
        if(__builtin_expect(t_cachedTid == 0, 0)){
            cacheTid();
        }
        return t_cachedTid;
    }
    inline const char* tidString() //for logging
    {
        return t_tidString;
    }
    inline int tidStringLength()
    {
        return t_tidStringLength;
    }
    inline const char* name()
    {
        return t_threadName;
    }
}