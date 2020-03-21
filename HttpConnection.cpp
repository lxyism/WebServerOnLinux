#include"Util.h"
#include"EventLoop.h"
#include"HttpServer.h"
#include"HttpConnection.h"
#include<sys/mman.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>

using namespace std;

const int HttpConnection::kLongLinkTime = 60;       //长连接时间60s
pthread_once_t FileType::once_control_ = PTHREAD_ONCE_INIT;
std::map<std::string, std::string> FileType::type_;

HttpConnection::HttpConnection(int fd, HttpServer* http_server):
    fd_(fd),
    channel_sptr_(new Channel(fd)),
    in_buf_(),
    out_buf_(),
    is_close_(false),
    http_server_(http_server)
    {
        channel_sptr_->SetReadCallback(std::bind(&HttpConnection::ReadHandle, this));
        channel_sptr_->SetWriteCallback(std::bind(&HttpConnection::WriteHandle, this));
        setSocketNonBlocking(fd),
        setSocketNodelay(fd);
        Init();
    }

HttpConnection::~HttpConnection(){
    if(fd_ > 0){
        close(fd_);
    }
}

void HttpConnection::Init(){
    http_state_ = STATE_WAIT_REQUEST;
    parse_state_ = STATE_PARSE_REQUEST_LINE;
    parse_request_line_state_ = STATE_PARSE_REQUEST_SUCCESS;
    parse_header_state_ = STATE_PARSE_HEADER_SUCCESS;
    parse_body_state_ = STATE_PARSE_BODY_SUCCESS;
    analysis_state_ = STATE_ANALYSIS_SUCCESS;

    method_ = METHOD_GET;
    file_name_.Clear();             //Clear()
    http_version_ = HTTP_10;
    header_.clear();
    have_body_ = false;
}   

//写操作
void HttpConnection::WriteHandle(){
    int write_size;
    if((write_size=Writen(fd_, out_buf_)) < 0){
        perror("writen error:");
        LOG_ERROR<<"writen error";
        exit(0);
    }
    else if(write_size < out_buf_.size()){
        //substr():从参数的地方截取剩下的内容，若有两个参数，则为这两参数中间部分
        out_buf_ = out_buf_.substr(write_size);  
    }
    else{
        if(!keep_alive_){
            ActiveClose();
        }else{  
            out_buf_.clear();
            channel_sptr_->DisableWrite();
        }
    }
}

void HttpConnection::ReadHandle(){
    LOG_DEBUG<<"into readhandle";
    if(http_state_ != STATE_WAIT_REQUEST){
        printf("error state1: \n");
        HttpError();
        return;
    }
    LOG_DEBUG<<"begin read";
    int read_num = Read(fd_, in_buf_);
    if(read_num == 0){
        PassiveClose();             //?
        return;
    }
    //解析请求行
    LOG_DEBUG<<"begin parse reques";
    http_state_ = STATE_PARSE_REQUEST;
    if(parse_state_ != STATE_PARSE_REQUEST_LINE){
        printf("error state2:\n");
        HttpError();
        return;
    }
    ParseRequestLine();
    if(parse_request_line_state_ != STATE_PARSE_REQUEST_SUCCESS){
        printf("parse request line error: %d\n", parse_request_line_state_);
        HttpError();
        return ;
    }
    //解析首部
    LOG_DEBUG<<"begin parse header";
    parse_state_ = STATE_PARSE_HEADER;
    ParseHeader();
    if(parse_header_state_ != STATE_PARSE_HEADER_SUCCESS){
        printf("parse header error:%d\n", parse_header_state_);
        HttpError();
        return;
    }

    if(have_body_){
        LOG_DEBUG<<"begin parse body";
        parse_state_ = STATE_PARSE_BODY;
        ParseBody();
        if(parse_body_state_ != STATE_PARSE_BODY_SUCCESS){
            printf("parse body error:%d\n",parse_body_state_);
            HttpError();
            return;
        }
    }
    LOG_DEBUG<<"analysis request";
    http_state_ = STATE_ANALYSIS_REQUEST;
    AnalysisRequest();
    if(analysis_state_ != STATE_ANALYSIS_SUCCESS){
        printf("analysis request error:\n");
        HttpError();
        return;
    }
    Init();
    return;
}

void HttpConnection::HttpError(){
    char send_buff[4096];
    string body_buff, header_buff;
    string short_msg = "404 Not Found!";
    body_buff += "<html><title>哎~出错了</title>";
    body_buff += "<body bgcolor =\"ffffff\">";
    body_buff += short_msg;
    body_buff += "<hr><em> Web Server </em>\n</body></html>";

    header_buff += "HTTP/1.1" + short_msg + "\r\n";
    header_buff += "Content-Type: text/html\r\n";
    header_buff += "Connection: Close\r\n";
    header_buff += "Content-Length: "+ std::to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: LXYism's Web Server\r\n";
    header_buff += "\r\n";
    //错误处理不考虑Writen不完的情况
    sprintf(send_buff, "%s", header_buff.c_str());
    Writen(fd_, send_buff, strlen(send_buff));
    sprintf(send_buff, "%s", body_buff.c_str());
    Writen(fd_, send_buff, strlen(send_buff));
    Init();
}

//解析请求行
void HttpConnection::ParseRequestLine(){
    //取出第一行作为请求行
    auto pos = in_buf_.find('\n');
    if(pos == string::npos){
        parse_request_line_state_ = STATE_PARSE_ERROR;
        return;
    }
    string request_line = in_buf_.substr(0, pos + 1);
    in_buf_ = in_buf_.substr(pos + 1);

    //method
    auto pos_get = request_line.find("GET");
    auto pos_head = request_line.find("HEADER");

    if(pos_get != string::npos){
        method_ = METHOD_GET;
        pos = pos_get;
    }
    else if(pos_head != string::npos){
        method_ = METHOD_HEAD;
        pos = pos_head;
    }
    else{
        parse_request_line_state_ = STATE_UNSUPPORTED_METHOD;
        return;
    }

    //request file
    pos = request_line.find('/', pos);
    if(pos == string::npos){
        parse_request_line_state_ = STATE_UNSUPPORTED_URL;
        return;
    }
    else{
        auto _pos = request_line.find(' ', pos);
        if(pos == string::npos){
            parse_request_line_state_ = STATE_UNSUPPORTED_URL;
            return;
        }
        else{
            file_name_ = request_line.substr(pos+1, _pos-pos-1);
            auto __pos = file_name_.find('?');
            if(pos != string::npos){
                file_name_ = file_name_.substr(0, __pos);
            }
        }
        pos = _pos;
    }

    //HTTP version
    pos = request_line.find('/', pos);
    if(pos == string::npos){
        parse_request_line_state_ = STATE_UNSUPPORTED_HTTP_VERSION;
        return;
    }
    else{
        if(request_line.size()-pos < 6){
            parse_request_line_state_ = STATE_UNSUPPORTED_HTTP_VERSION;
            return;
        }
        else{
            string version = request_line.substr(pos + 1, 3);
            if(version == "1.0"){
                http_version_ = HTTP_10;
            }
            else if(version = "1.1"){
                http_version_ = HTTP_11;
            }
            else{
                parse_request_line_state_ = STATE_UNSUPPORTED_HTTP_VERSION;
                return;
            }
        }
    }
    parse_request_line_state_ = STATE_PARSE_REQUEST_SUCCESS;
}

void HttpConnection::ParseHeader(){
    string::size_type key_start = 0,key_end = 0, value_start = 0, value_end = 0;
    while(1){
        key_end = in_buf_.find(':', key_start);
        if(key_end == string::npos){
            if(in_buf_[key_start] == '\r' && in_buf_[key_start+1] = '\n'){
                in_buf_ = in_buf_.substr(key_start+2);
                if(in_buf_.size() > 0){
                    have_body_ = true;
                }
                parse_header_state_ = STATE_PARSE_HEADER_SUCCESS;
            }
            else{
                parse_header_state_ = STATE_PARSE_KEY_ERROR;
            }
            return ;
        }
        else{
            value_start = key_end + 2;
            value_end = in_buf_.find('\r', value_start);
            if(value_end == string::npos){
                parse_header_state_ = STATE_PARSE_VALUE_ERROR;
                break;
            }
            else{
                string key(in_buf_.begin() + key_start, in_buf_.begin() + key_end);
                if(key.size() == 0){
                    parse_header_state_ = STATE_PARSE_KEY_ERROR;
                    break;
                }
                string value(in_buf_.begin() + value_start, in_buf_.begin() + value_end);
                if(value.size() == 0){
                    parse_header_state_ = STATE_PARSE_VALUE_ERROR;
                    break;
                }
                header_[key] = value;
                key_start = value_end + 2;
            }
        }
    }
}

//暂时不接受主体，直接将主体丢弃
void HttpConnection::ParseBody(){
    int content_length = -1;
    if(header_.find("Content-length" != header_.end())){
        content_length = std::stoi(header_["content-length"]);
    }
    else{
        parse_body_state_ = STATE_PARSE_BODY_ERROR;
        return;
    }
    if(content_length == in_buf_.size()){
        in_buf_.clear();
    }
    else{
        in_buf_ = in_buf_.substr(content_length);
    }
    parse_body_state_ = STATE_PARSE_BODY_ERROR;
}

void HttpConnection::AnalysisRequest(){
    if(method_ == METHOD_GET || method_ == METHOD_HEAD){
        string header;
        header += "HTTP/1.1 200 ok\r\n";
        if(header_.find("Connection") != header_.end()){
            header += string("Connection: Close\r\n");  //暂时不接受长连接
        }
        else{
            header += string("Connection: Close\r\n");
            keep_alive_ = false;
        }

        auto dot_pos = file_name_.find('.');
        string file_type;
        if(dot_pos == string::npos){
            file_type = FileType::GetType("default");
        }
        else{
            file_type = FileType::GetType(file_name_.substr(dot_pos));
        }

        if(file_name_ == "hello"){
            string content = "hello world";
            out_buf_ = string("HTTP/1.1 200 OK\r\nContent-type:text/plain\r\n") +"Content-Length: " + std::to_string(content.size())+"\r\n"+"\r\n"+content;
            WriteData();
            analysis_state_ = STATE_ANALYSIS_SUCCESS;
            return;
        }
        else{
            struct stat sbuf;
            if(stat(file_name_.c_str(), &sbuf) < 0){
                analysis_state_ = STATE_ANALYSIS_ERROR;
                return;
            }
            header += "Content-Type: " + file_type + "\r\n";
            header += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
            header += "Server: LXYism's Web Server\r\n";
            header += "\r\n";
            out_buf_ = header;

            if(method_ == METHOD_HEAD){
                analysis_state_ = STATE_ANALYSIS_SUCCESS;
                return;
            }

            int send_file_fd = open(file_name_.c_str(), O_RDONLY, 0);
            if(send_file_fd < 0){
                out_buf_.clear();
                analysis_state_ = STATE_ANALYSIS_ERROR;
                return;
            }
            void *mmapRet = mmap(NULL, sbuf, PROT_READ, MAP_PRIVATE, send_file_fd, 0);
            close(send_file_fd);
            if(mmapRet == (void*)(-1)){
                munmap(mmapRet, sbuf.st_size);
                out_buf_.clear();
                analysis_state_ = STATE_ANALYSIS_ERROR;
                return;
            }
            char *src_addr = static_cast<char *>(mmapRet);
            out_buf_ += string(src_addr, src_addr + sbuf.st_size);
            munmap(mmapRet, sbuf.st_size);
            WriteData();
            analysis_state_ = STATE_ANALYSIS_SUCCESS;
            return;
        }
    }
}

void HttpConnection::TimeHandle(){
    ActiveClose();
}

void HttpConnection::ActiveClose(){
    if(connect_state_ == STATE_CONNECTING){
        ::shutdown(fd, SHUT_WR);
        connect_state_ = STATE_DISCONNECTING;
    }
    else if(connect_state_ == STATE_DISCONNECTING){
        http_server_->DelConnection(fd_);
        connect_state_ = STATE_DISCONNECTED;
    }
}

void HttpConnection::PassiveClose(){
    if(connect_state_ == STATE_CONNECTING){
        connect_state_ = STATE_DISCONNECTING;
    }
    else if(connect_state_ == STATE_DISCONNECTING){
        http_server_->DelConnection(fd_);
        connect_state_ = STATE_DISCONNECTED;
    }
}

void HttpConnection::WriteData(){
    size_t write_size = 0;
    if((write_size = Writen(fd_, out_buf_)) < 0){
        LOG_ERROR<<"writen error";
        exit(0);
    }
    else if(write_size < out_buf_.size()){
        out_buf_ = out_buf_.substr(write_size);
        channel_sptr_->EnableWrite();
    }
    else{
        if(!keep_alive_){
            ActiveClose();
        }
        else{
            out_buf_.clear();
        }
    }
}