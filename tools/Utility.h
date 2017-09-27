#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include "Define.hpp"

// 日志loader
EXPORT_API bool log_loader(std::map<std::string, std::vector<std::string>>&command_params);

// 日志unloader
EXPORT_API bool log_unloader();

// 配置loader
EXPORT_API bool config_loader(std::map<std::string, std::vector<std::string>>&command_params);

// 配置unloader
EXPORT_API bool config_unloader();

// loader
EXPORT_API bool loader(std::map<std::string, std::vector<std::string> >&command_params);

// unloader
EXPORT_API bool unloader();

template <class Type>
Type stringToNum(const std::string& str)
{
		std::istringstream iss(str);
        Type num;
        iss >> num;
        return num;
}

template<typename T>
void write_file(const char *filename, const T &text)
{
	std::ofstream fout;
	fout.open(filename, std::ios::app);
	if (fout.is_open())
	{
		fout << text;
	}
}

// 开始计时
EXPORT_API void StartTimer();

// 设置计时点
EXPORT_API void SetTimerPoint(double percentage);

// 打印日志
EXPORT_API void Log(const char *fmt, ...);

// 配置读取
EXPORT_API std::vector<std::string> Config(const std::string &key);
EXPORT_API std::string ConfigFirst(const std::string &key);