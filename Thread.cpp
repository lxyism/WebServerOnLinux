#include<unistd.h>
#include"Thread.h"

Thread::Thread(ThreadFunction func):thread_data_(func){
    int ret;
    //pthread_create();
    if((ret = pthread_create(&thread_id_, NULL, ThreadRunning, &thread_data_)) != 0){
        perror("pthread_create error\n");
        exit(-1);
    }
}

void* Thread::ThreadRunning(void* p){
    ThreadData* therad_data = static_cast<ThreadData*>(p);
    therad_data->thread_function_();
}