//封装线程互斥量
#pragma once
#include<pthread.h>
#include"noncopyable.h"

class Mutex: noncopyable{
public:
    Mutex() {pthread_mutex_init(&p_mutex_, NULL);}
    ~Mutex(){
        pthread_mutex_lock(&p_mutex_);
        pthread_mutex_destory(&p_mutex_);
    }
    void Lock(){pthread_mutex_lock(&p_mutex_);}//上锁
    void UnLock(){pthread_mutex_unlock(&p_mutex_);}//解锁
    pthread_mutex_t& Get() {return p_mutex_;}

private:
    pthread_mutex_t p_mutex_;
private:
    //友元类不受访问权限影响
    friend class Condition;
};

class MutexGuard:public noncopyable{
public:
    explicit MutexLockGuard(Mutex &mutex) :mutex_(mutex) {mutex.Lock();}
    ~MutexLockGuard() {mutex_.Unlock();}
private:
    Mutex& mutex_;
};