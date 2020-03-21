
#pragma once
#include"LogStream.h"
#include<functional>

class AsyncLogging;

class Logger{
public:
    enum Level{
        TRACE,DEBUG,INFO,WARN,ERROR,FATAL
    };

    typedef std::function<void(const char* msg, int len)> OutputFunc;
    typedef std::function<void()> FlushFunc;

    class LogFormat{
    public:
        LogFormat(const char *filename, int line, Level level):
            filename_(filename),
            line_(line),
            level_(level)
            {
                FormatTime();
                FormatId();
                FormatLevel();
            }
        LogStream &GetLogStream(){
            return log_stream_;
        }
        void Finish();
    private:
        const char* filename_;
        int line_;
        Level level_;
        LogStream log_stream_;
        void FormatTime();
        void FormatId();
        void FormatLevel();
    };

public:
    Logger(const char *filename, int line, Level level):
        log_format_(filename, line, level),
        current_level_(level){}
    ~Logger(){
        Finish();
        auto &buffer = GetLogStream().getBuffer();
        output_func_(buffer.Data(), buffer.Length());
        if(current_level_ == FATAL || current_level_ ==DEBUG)
        {
            flush_func_();
        }
    }

    static Level GetLogLevel(){
        return level_;
    }
    static void SetLevel(Level level){
        level_ = level;
    }
    LogStream &GetLogStream(){
        return log_format_.GetLogStream();
    }
    static void SetOutput(OutputFunc func){
        output_func_ = func;
    }
    static void SetFlush(FlushFunc func){
        flush_func_ = func;
    }
private:
    void Finish(){
        log_format_.Finish();
    }
    LogFormat log_format_;
    Level current_level_;
    static Level level_;
    static OutputFunc output_func_;
    static FlushFunc flush_func_;
};

#define LOG_TRACE if(Logger::GetLogLevel() <= Logger::Level::TRACE)\
Logger(__FILE__,__LINE__,Logger::Level::TRACE).GetLogStream()
#define LOG_DEBUG if(Logger::GetLogLevel() <= Logger::Level::DEBUG)\
Logger(__FILE__,__LINE__,Logger::Level::DEBUG).GetLogStream()
#define LOG_INFO if(Logger::GetLogLevel() <= Logger::Level::INFO)\
Logger(__FILE__,__LINE__,Logger::Level::INFO).GetLogStream()
#define LOG_WARN if(Logger::GetLogLevel() <= Logger::Level::WARN)\
Logger(__FILE__,__LINE__,Logger::Level::WARN).GetLogStream()
#define LOG_ERROR if(Logger::GetLogLevel() <= Logger::Level::ERROR)\
Logger(__FILE__,__LINE__,Logger::Level::ERROR).GetLogStream()
#define LOG_FATAL if(Logger::GetLogLevel() <= Logger::Level::FATAL)\
Logger(__FILE__,__LINE__,Logger::Level::FATAL).GetLogStream()
