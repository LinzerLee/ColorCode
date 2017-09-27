#define DLL_IMPL

#include <ctime>
#include <string>
#include <map>

#include "Layout.h"

using tools::log::Layout;
using std::string;
using std::map;

/**
 * Constructor
 */
Layout::Layout()
{
	_init();
	this->pattern = string("");
	this->params = map<string, string>();
}

Layout::Layout(string pattern, map<string, string> params/*=map<string, string>()*/)
{
	_init();
	this->pattern = pattern;
	this->params = params;
}

/**
 * Destructor
 */
Layout::~Layout()
{
}

/**
 * Initialize
 */
bool Layout::_init()
{
	// 初始化状态
	state = 0;
	// 默认参数
	default_params["a"] = "";		// 星期几的缩写
	default_params["A"] = "";		// 星期几的全名
	default_params["b"] = "";		// 月份名称的缩写
	default_params["B"] = "";		// 月份名称的全名
	default_params["d"] = "";		// 用数字表示本月的第几天 (范围为 00 至 31)--日期
	default_params["H"] = "";		// 用 24 小时制数字表示小时数 (范围为 00 至 23)
	default_params["I"] = "";		// 用 12 小时制数字表示小时数 (范围为 01 至 12)
	default_params["j"] = "";		// 以数字表示当年度的第几天 (范围为 001 至 366)
	default_params["m"] = "";		// 月份的数字 (范围由 1 至 12)
	default_params["c"] = "";		// 本地端日期时间较佳表示字符串
	default_params["M"] = "";		// 分钟
	default_params["p"] = "";		// 以 AM 或 PM 表示本地端时间
	default_params["S"] = "";		// 秒数
	default_params["U"] = "";		// 数字表示为本年度的第几周，第一个星期由第一个周日开始
	default_params["W"] = "";		// 数字表示为本年度的第几周，第一个星期由第一个周一开始
	default_params["w"] = "";		// 用数字表示本周的第几天 ( 0 为周日)
	default_params["x"] = "";		// 不含时间的日期表示法
	default_params["X"] = "";		// 不含日期的时间表示法
	default_params["y"] = "";		// 二位数字表示年份 (范围由 00 至 99)
	default_params["Y"] = "";		// 完整的年份数字表示，即四位数
	default_params["Z"] = "";		// 时区或名称缩写
	default_params["news"] = "";		// 说明
	
	return true; 
}

/**
 * Utillity
 */
void Layout::setStateCode(int code)
{
	state = code;
}

/**
 * Interface
 */
int Layout::getStateCode()
{
	return state;
}

void Layout::resetState()
{
	setStateCode(0);
}

void Layout::resetPattern(string pattern)
{
	this->pattern = pattern;
	this->resetState();
}

string Layout::getPattern()
{
	return this->pattern;
}

void Layout::resetParams(map<string, string> params)
{
	this->params = params;
	this->resetState();
}

map<string, string> Layout::getParams()
{
	return this->params;
}

void Layout::resetParam(string key, string value)
{
	this->params[key] = value;
	this->resetState();
}

string Layout::getParam(string key)
{
	return this->params[key];
}

string Layout::parser(string msg/*=string()*/)
{
	if(string("")==this->pattern)
	{
		return pattern;
	}

	string::size_type len = this->pattern.size();
	string::size_type pos = this->pattern.find('%');

	if(string::npos==pos)
	{
		return pattern;
	}

	string result;
	string::size_type lastpos=0, l=0;
	while(string::npos!=pos && pos<len)
	{
		result += pattern.substr(lastpos, pos-lastpos);
		if(pos+1<len)
		{
			if('%'==pattern[pos+1])
			{
				result += "%";
				lastpos = pos+2;
			}
			else
			{
				// 查找可替换字符
				// 1:在params中查找
				map<string, string>::iterator it = this->params.begin();
				for(; it!=this->params.end(); ++it)
				{
					l = it->first.size();
					if(pos+l>=len)
					{
						continue;
					}

					string tmp = pattern.substr(pos+1, l);
					// 找到可替换参数
					if(tmp==it->first)
					{
						result += it->second;
						lastpos = pos+l+1;
						break;
					}
				}

				// 2:params中未找到,取系统默认的
				if(it==this->params.end())
				{
					for(it=default_params.begin(); it!=default_params.end(); ++it)
					{
						l = it->first.size();
						if(pos+l>=len)
						{
							continue;
						}

						string tmp = pattern.substr(pos+1, l);

						// 找到可替换参数
						if(tmp==it->first)
						{
							if("news"==tmp)
							{
								result += msg;
							}
							else
							{
								time_t rawtime;
								time(&rawtime);
								char t[64];
								strftime(t, sizeof(t), string("%"+tmp).c_str(), localtime(&rawtime));
								result += t;
							}

							lastpos = pos+l+1;
							break;
						}
					}
				}

				// 3:都未找到
				if(it==this->default_params.end())
				{
					result += "%";
					lastpos = pos+1;
				}
			}

			pos = this->pattern.find('%', lastpos);
		}
		else
		{
			break;
		}
	}

	if(lastpos<len)
	{
		result += pattern.substr(lastpos);
	}
	return result;
}

