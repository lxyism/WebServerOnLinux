//此类实现http的请求和回应
#include"Timer.h"
#include"Channel.h"
#include<string>
#include<map>
#include<memory>
#include<pthread.h>

class HttpServer;

enum HttpState{
    STATE_WAIT_REQUEST;     //http等待请求状态
    STATE_PARSE_REQUEST;    //解析请求行状态
    STATE_ANALYSIS_REQUEST; //分析请求状态
};

enum ParseState{
    STATE_PARSE_REQUEST_LINE,      //请求行 
    STATE_PARSE_HEADER,            //头部
    STATE_PARSE_BODY               //实体
};

enum ParseRequestLineState{
    STATE_PARSE_REQUEST_SUCCESS,            //解析请求成功
    STATE_PARSE_ERROR,                      //解析出错
    STATE_UNSUPPORTED_METHOD,               //不支持的方法
    STATE_UNSUPPORTED_URL,                  //不支持的地址
    STATE_UNSUPPORTED_HTTP_VERSION          //不支持的http版本
};

enum ParseHeaderState{
    STATE_PARSE_HEADER_SUCCESS,             //解析头部成功
    STATE_PARSE_KEY_ERROR,                  //解析key出错
    STATE_PARSE_VALUE_ERROR                 //解析value出错
};

enum Method{
    METHOD_GET,         //get操作        
    METHOD_HEAD,        //head操作       
    METHOD_POST,        //post操作  
    METHOD_PUT,         //put操作  
    METHOD_DELETE       //delete操作  
};

enum HttpVersion{
    HTTP_10,        //1.0
    HTTP_11        //1.1
};

enum ParseBodyState{
    STATE_PARSE_BODY_ERROR,         //解析body出错
    STATE_PARSE_BODY_SUCCESS        //解析body成功
};

enum AnalysisState{
    STATE_ANALYSIS_ERROR,       //analysis出错
    STATE_ANALYSIS_SUCCESS      //analysis成功
};

enum ConnectState{
    STATE_CONNECTING,       //正在连接状态
    STATE_DISCONNECTING,    //正在断开连接状态
    STATE_DISCONNECTED      //已经断开连接状态
};

class FileType:public noncopyable{
public:
    static std::string GetType(const std::string s){
        pthread_once(&once_control_, FileType::Init);
        if(type_.find(s) == type_.end()){
            return type_["default"];
        }else{
            return type_[s];
        }
    }
private:
    static std::map<std::string, std::string> type_;
    static pthread_once_t once_control_;    //一次性初始化变量，一般用于线程键
    FileType(){}
    static void Init(){
        type_.[".html"] = "text/html";
        type_.[".htm"] = "text/html";
        type_.[".default"] = "text/html";
    }
};

class HttpConnection:public noncopyable{
public:
    //HttpConnection使用shared_ptr来管理的类，因为它的生命周期模糊。
    typedef Channel::Channel_SPtr Channel_SPtr;
public:
    HttpConnection(int fd, HttpServer *http_server);
    ~HttpConnection();
    void WriteHandle();
    void ReadHandle();
    void ActiveClose();
    void PassiveClose();
    void TimeHandle(){
        Channel_SPtr channel(){
            return channel_sptr_;
        }
        const int fd(){
            return fd_;
        }
    }
private:
    static const int kLongLinkTime; //单位为秒
    
    const int fd_;
    Channel_SPtr channel_sptr_;
    std::string in_buf_;
    std::string out_buf_;

    HttpState http_state_;
    ParseState parse_state_;
    ParseRequestLineState parse_request_line_state_;
    ParseHeaderState parse_header_state_;
    ParseBodyState parse_body_state_;
    AnalysisState analysis_state_;
    ConnectState connect_state_;

    Method method_;
    std::string file_name_;
    HttpVersion http_version_;
    std::map<std::string, std::string> header_;
    typedef std::shared_ptr<Timer> Timer_SPtr;
    Timer_SPtr timer_sptr_;

    bool have_body_;
    bool is_close_;
    bool keep_alive_;

    HttpServer *http_server_;

    void ParseRequestLine();
    void ParseHeader();
    void ParseBody();
    void AnalysisRequest();
    void Init();
    void HttpError();
    void WriteData();
};