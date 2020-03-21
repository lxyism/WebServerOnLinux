#include"NotifyEvent.h"

int CreatEventFd(){
    int fd = evenfd(0, EFD_NONBLOCK);
    if(fd < 0){
        LOG_ERROR<<"create evenfd failed";
        exit(0);
    }
    return fd;
}