#include"ThreadTid.h"
#include<sys/syscall>
#include<unistd.h>
#include<stdio.h>
#include<assert.h>

__thread int tid = 0;
__thread char tid_str[32] = {0};
__thread int tid_str_length = 0;

void cacheTid(){
	tid = ::syscall(SYS_gettid);
	int len = snprintf(tid_str, sizeof tid_str, "%05d", tid);
	assert(len == 5);
}

int getTid(){
	if(!tid){
		cacheTid();
	}
	return tid;
}

const char *getTidString(){
	if(!tid)
		cacheTid();
	return tid_str;
}