#pragma once
#include<cstdlib>
#include<string>

size_t Readn(int fd, void* buff, size_t n);
size_t Readn(int fd, std::string &str);
size_t Writen(int fd, void *buffer, size_t n);
size_t Writen(int fd, std::string &str);

int setSocketNonBlocking(int fd);
void setSocketNoDelay(int fd);      //禁用Nagle算法
int socket_bind_listen(unsigned short port, bool nonBlocking = true);


