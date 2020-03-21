
#include "Logging.h"
#include <sys/time.h>
#include <assert.h>
#include "ThreadTid.h"
#include "AsyncLogging.h"

//默认向标准输出写
inline void DefaultOutput(const char& msg, int len){
	fwrite(msg, 1, len, stdout);
}

//向标准输出刷新
inline void DefaultFlush(){
	fflush(std::cout);
}

Logging::Level Logging::level_ = Logging::INFO;
Logging::OutputFunc Logging::output_func_ = DefaultOutput;
Logging::FlushFunc Logging::flush_func_ = DefaultFlush;

__thread time_t last_second;
__thread char time_str[26];

void Logging::LogFormat::FormatTime(){
	struct timeval tv;
	::gettimeofday(&tv, NULL);
	time_t second = static_cast<time_t>(tv.sec);
	int micro_second = static_cast<int>(tv.usec);
	if(second != last_second)//如果刷新时间不在同一秒内就重新获取时间
	{
		last_second = second;
		struct tm time;
		localtime_r(&second, &time);	//带r后缀的实在参数中返回结果的，不带的以返回值返回
		int len = snprintf(time_str, sizeof time_str, "%4d%02d%02d %02d:%02d:%02d:%06d ",
			time.tm_year+1900, time.tm_mon+1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, micro_second);
		//格式化年月日的输出格式
		assert(len == 25);
	}
	else{	//在同一秒内
		int len =snprintf(time_str+18,sizeof(time_str) -18, "%06d", micro_second );
		assert(len == 17);
	}
	log_stream_<<time_str;
}

void Logging::LogFormat::FormatTid(){
	const char *tid = getTidString();
	log_stream_<<tid<<" ";
}

void Logging::LogFormat::FormatLevel(){
	static char *level_str[6] = {"TRACE","DEBUG","INFO","WARN","ERROR","FATAL"};
	log_stream_<<level_str[level_];
}

void Logging::LogFormat::Finish(){
	log_stream_<<" - "<<filename_<<":"<<line_<<"\n";
}
