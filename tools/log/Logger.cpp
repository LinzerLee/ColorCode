#define DLL_IMPL

#include "Appender.h"
#include "Logger.h"

#include <string>

using tools::log::Appender;
using tools::log::Logger;

using std::string;
using std::vector;

/**
 * Constructor
 */
Logger::Logger()
{
	_init();
}

/**
 * Destructor
 */
Logger::~Logger()
{
}

/**
 * Initialize
 */
bool Logger::_init()
{
	// ³õÊ¼»¯×´Ì¬
	state = 0;
	
	return true; 
}

/**
 * Utillity
 */
void Logger::setStateCode(int code)
{
	state = code;
}

/**
 * Interface
 */
int Logger::getStateCode()
{
	return state;
}

void Logger::resetState()
{
	this->setStateCode(0);
}

int Logger::addAppender(Appender* appender)
{
	this->appenders.push_back(appender);

	return this->appenders.size()-1;
}

bool Logger::removeAppender(unsigned int code)
{
	if(code<0 || code>=this->appenders.size())
	{
		return false;
	}

	this->appenders[code] = NULL;
	
	return true; 
}

bool Logger::removeAppender(Appender* appender)
{
	for(unsigned int i=0; i<this->appenders.size(); ++i)
	{
		if(this->appenders[i]==appender)
		{
			this->appenders[i] = NULL;
			return true;
		}
	}

	return false;
}

bool Logger::log(string msg/*=string()*/)
{
	vector<Appender*>::iterator it = this->appenders.begin();
	for(; it!=this->appenders.end(); ++it)
	{
		if(NULL!=*it) 
		{
			(*it)->print(msg);
		}	
	}
	
	return true; 
}
