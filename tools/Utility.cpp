#define DLL_IMPL

#include<ctime>
#include<cstring> 

#include "Utility.h"
#include "./config/Parser.h"
#include "./log/Layout.h"
#include "./log/Appender.h"
#include "./log/Logger.h"

using namespace tools::log;
using namespace tools::config;

using std::map;
using std::string;
using std::vector;

// Utility
static map<string, string> params;
static Layout layout;
static Appender appender1, appender2;
static FILE *output=NULL;
static Logger logger;
static Parser parser;
static map<string, string> config;

// 日志loader
bool log_loader(map<string, vector<string> >&command_params)
{
        layout.resetPattern("[%X %x] %news\n");
        layout.resetParams(params);

		appender1.resetLayout(&layout);
		logger.addAppender(&appender1);

		if (command_params.count("LOG") == 0)
			return true;

        output = fopen(command_params["LOG"][0].c_str(), "a+");
		
        if(!output)
        {
                return false;
        }

        appender2.resetDevice(output);
        appender2.resetLayout(&layout);
        logger.addAppender(&appender2);

        return true;
}

// 日志unloader
bool log_unloader()
{
        fclose(output);

        return true;
}

// 配置loader
bool config_loader(map<string, vector<string> >&command_params)
{
        if(command_params.count("CFG")==0)
                return true;

        if(!parser.loadConfig(command_params["CFG"][0].c_str()))
        {
                return false;
        }

        map<string, vector<string> > config_file = parser.handle();
        map<string, vector<string> >::iterator it = config_file.begin();
        for(; it!=config_file.end(); ++it)
        {
                for(unsigned int j=0; j<it->second.size(); ++j)
                {
                        config[it->first] = it->second[j];
                }
        }

        return true;
}

// 配置unloader
bool config_unloader()
{
        return true;
}

// loader
bool loader(map<string, vector<string> >&command_params)
{
	if (!log_loader(command_params))
	{
		fprintf(stderr, "log load faild.\n");
		return false;
	}

	if (!config_loader(command_params))
	{
		fprintf(stderr, "config load faild.\n");
		return false;
	}
        
	return true;
}

// unloader
bool unloader()
{
    return log_unloader() && config_unloader();
}

static time_t begin_time, end_time;
static char nowtime[24];
static char buffer[1024];
static tm *lt;

// 开始计时
void StartTimer()
{
          begin_time = time(NULL);
          lt = localtime(&begin_time);
          memset(nowtime, 0, sizeof(nowtime));
          strftime(nowtime, 24, "%Y-%m-%d %H:%M:%S", lt);
		  Log("begin time : %s", nowtime);
}

// 设置计时点
void SetTimerPoint(double percentage)
{
          if(percentage < 0)
              percentage = 0.0f;

          if(percentage > 1)
              percentage = 1.0f;

          end_time = time(NULL);
          lt = localtime(&end_time);
          memset(nowtime, 0, sizeof(nowtime));
          strftime(nowtime, 24, "%Y-%m-%d %H:%M:%S", lt);
		  Log("point time : %s", nowtime);
		  Log("\t处理时长 : %ds", end_time - begin_time);
		  Log("\t完成进度 : %.2f%%", percentage * 100);
}

#include <cstdarg>
// 打印日志
void Log(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);
	logger.log(buffer);
}

// 配置读取
std::vector<std::string> Config(const std::string &key)
{
	return parser.config(key);
}

std::string ConfigFirst(const std::string &key)
{
	std::vector<std::string> &&cfg = Config(key);
	if (cfg.size() > 0)
		return cfg[0];
	else
		std::string();
}
