#include "Util.h"
#include<errno.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<signal.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>

// void perror(const char* msg){
//     printf("%s",errno);
//     exist(-1);
// }
const int LISTENQ = 1000;
const int MAX_BUFF = 4096;
size_t Readn(int fd, void *buff, size_t n){
    auto nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    char *ptr = (char *)buff;
    while(nleft > 0){
        if( (nread = read(fd, ptr, nleft)) < 0){    
            if(errno == EINTR)  //被系统中断打断
                nread = 0;  //继续读
            else if(errno == EAGAIN){   //套接缓存区被读空，提示再试一次,非阻塞操作的时候，缓冲区也无内容可读
                return readSum;
            }else{  //未知情况，视为出错
                return -1;
            }
        }else if(nread == 0)    //对端关闭不处理
            break;
        readSum += nread;
        nleft -= nread;
        ptr += nread;
    }
    return readSum;
}

size_t Readn(int fd, std::string &str){
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while(true){
        char buff[MAX_BUFF];
        if((nread = read(fd, buff, MAX_BUFF)) < 0){
            if(errno == EINTR)  //被系统中断打断
                continue;   //继续读
            else if(errno == EAGAIN){   //套接字缓存区被读空
                return readSum;
            }else { //未知情况，视为出错
                perror("read error");
                return -1;
            }
        }else if(nread == 0){   //对端关闭，不处理
            return readSum;     //注意如果读到EOF，会返回0，且下次还会返回0，不会出现EAGAIN
        }
        readSum += nread;
        str += std::string(buff, buff+nread);
    }
    return readSum;
}

size_t Writen(int fd, void *buff, size_t n){
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    char* ptr = static_cast<char *>(buff);
    while(nleft > 0){
        if((nwritten = write(fd, ptr, nleft)) <= ){
            if(nwritten < 0){
                if(errno == EINTR){ //被系统中断打断
                    nwritten = 0;   //继续写
                    continue;
                }
                else if(errno == EAGAIN){   //缓存区被写满
                    break;
                }else{  //未知情况，视为出错
                    return -1;  
                }
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}

size_t Writen(int fd, std::string& str){
    size_t nleft = str.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = str.c_str();
    while(nleft > 0){
        if((nwritten = write(fd, ptr, nleft)) <= 0){
            if(nwritten < 0){
                if(errno == EINTR){//系统中断
                    nwritten = 0;   //继续写
                    continue;
                }else if(errno == EAGAIN)   //缓存区写满
                    break;
                else    //未知情况
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}

//把socket、bind、listen绑定为一个函数，然后返回一个处于监听状态的listen_fd
int socket_bind_listen(unsigned short port, bool NonBlocking){
    //检查port值，取正确区间范围
    if(port < 0|| port > 65535)
        return -1;
    //创建socket（IPv4 + TCP，返回监听描述符）
    int listen_fd = 0;
    if((listen_fd = socket(IF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket error\n");
        return -1;
    }
    //设置服务器IP和PORT，监听描述符绑定
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    int ret;
    if((ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) < 0){
        perror("bind error\n");
        return -1;
    }
    //开始监听，最大等待队列长度为LISTENQ
    if((ret = listen(listen_fd, LISTENQ)) < 0){
        perror("listen error\n");
        return -1;
    }
    //无效监听描述符
    if(NonBlocking){
        setSocketNonBlocking(listen_fd);
    }
    return listen_fd;
}

void setSocketNonBlocking(int fd)
{
    //fcntl系统调用可以用来对已打开的文件描述符进行各种控制操作以改变已打开文件的的各种属性
    int val = fcntl(fd,F_GETFL,0);      //F_GETFL:获取文件状态标志
    fcntl(fd,F_SETFL,val|O_NONBLOCK);   //F_SETFL：设置文件状态标志
}

//禁用Nagle算法
//Nagle算法可以再一定程度上避免网络阻塞；禁用nagle算法，可以避免发包出现延迟，这对于编写低延迟
//的网络服务很重要，TCP_NODELAY选项可以禁用Nagle算法
//Nagle算法其实就是先把一些要发送的数据存到一个缓冲区，然后缓冲区满了之后一并发送，但是对于实时
//网络服务来说，这是不行的。
void setSocketNodelay(int fd){
    int ret;
    int optval = fd ? 1 : 0;
    if((ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval)) < 0){
        perror("set no delay error\n");
    }
}


