#define DLL_IMPL

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <cctype>
#include<cstdio>

#include "Parser.h"

using std::ifstream;
using std::istringstream;
using std::ostringstream;
using std::string;
using std::map;
using std::vector;
using tools::config::Parser;

/**
 * Static Interface
 */
map<string, vector<string> > Parser::CommandParams(int argc, char *argv[])
{
	map<string, vector<string> > commandparams;
	if(argc<2) 
		return commandparams;
		
	// 参数前面可以为参数,值或空
	// 值前面可以为参数或值 
	string key; 
	for(int i=1; i<argc; ++i)
	{	
		// 去除值前面为空的情况 
		if(1==i && '-'!=argv[i][0])
		{
			return commandparams;
		}
		
		if('-'==argv[i][0] && 1==strlen(argv[i]))
		{
			return commandparams;
		}
		
		if('-'==argv[i][0]) 
		{
			key = &argv[i][1];
			commandparams[key] = vector<string>();
		}
		else
		{
			commandparams[key].push_back(argv[i]); 
		} 	
	} 
	
	return commandparams;
}

/**
 * Constructor
 */
Parser::Parser()
{
	_init();
}

Parser::Parser(const char* cfgpath)
{
	_init();
	this->loadConfig(cfgpath);
}

/**
 * Destructor
 */
Parser::~Parser()
{
}

/**
 * Initialize
 */
bool Parser::_init()
{
	// 初始化状态
	state = 0;
	
	return true; 
}

/**
 * Utillity
 */
void Parser::setStateCode(int code)
{
	state = code;
}

/**
 * Interface
 */
int Parser::getStateCode()
{
	return state;
}

void Parser::resetState()
{
	this->setStateCode(0);
}

bool Parser::loadConfig(const char* cfgpath)
{
	ifstream is(cfgpath);
	if (!is)
	{
		this->setStateCode(1);
		return false;
	}

	filepath = string(cfgpath);
	ostringstream buffer;
	char ch;
	while(buffer && is.get(ch))
	{
		buffer.put(ch);
	}

	this->content = buffer.str();
	this->setStateCode(3);
	return true;
}

string Parser::getContent()
{
	return this->content;
}

map<string, vector<string> >Parser::handle()
{
	if(3==getStateCode())
	{
		istringstream iss(this->content);

		char line[256];
		memset(line, 0, sizeof(line));
		while(iss.getline(line, 256))
		{
			int len = strlen(line);
			bool flag=false;		// 字符串开关
			bool vflag=false;		// value开关

			string key=string(""), value=string("");
			vector<string> keys;
			vector<string> values;
			for(int i=0; i<len; ++i)
			{
				if('\"'==line[i])
				{
					flag = !flag;
					continue;
				}

				if('='==line[i])
				{
					vflag = !vflag;
					continue;
				}

				if(!flag && isspace(line[i]))
					continue;

				if(!flag && ('#'==line[i] || ';'==line[i]))
					break;

				// key
				if(!vflag && ','==line[i])
				{
					if(key!=string(""))
					{
						keys.push_back(key);
						key=string("");
					}
					continue;
				}

				if(vflag && ','==line[i])
				{
					if(value!=string(""))
					{
						values.push_back(value);
						value=string("");
					}
					continue;
				}

				if(!vflag)
				{
					key += line[i];
				}
				else
				{
					value += line[i];
				}
			}

			if(key!=string(""))
			{
				keys.push_back(key);
				key=string("");
			}

			if(value!=string(""))
			{
				values.push_back(value);
				value=string("");
			}

			for(unsigned int j=0; j<keys.size(); ++j)
			{
				this->cfg[keys[j]] = values;
			}

			memset(line, 0, sizeof(line));
		}
	}

	setStateCode(0);
	return this->cfg;
}

void Parser::config(const std::string &key, const std::vector<std::string> &vaule)
{
	this->cfg[key] = vaule;
}

std::vector<std::string> Parser::config(const std::string &key)
{
	return this->cfg[key];
}

void Parser::mergeConfig(map<string, vector<string> > config)
{
	map<string, vector<string> >::iterator it = config.begin();
	for(; it!=config.end(); ++it)
	{
		this->cfg[it->first] = it->second;
	}
}




