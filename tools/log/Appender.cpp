#define DLL_IMPL

#include "Appender.h"
#include "Layout.h"

#include <cstdio>
#include <string>

using tools::log::Layout;
using tools::log::Appender;

using std::string;

/**
 * Constructor
 */
Appender::Appender()
{
	_init();
}

Appender::Appender(Layout* layout, FILE* device/*=stderr*/)
{
	_init();
	this->layout = layout;
	this->device = device;
}

/**
 * Destructor
 */
Appender::~Appender()
{
}

/**
 * Initialize
 */
bool Appender::_init()
{
	// 初始化状态
	state = 0;
	// 默认输出设备
	device = stderr;
	// 布局
	layout = NULL; 
	
	return true; 
}

/**
 * Utillity
 */
void Appender::setStateCode(int code)
{
	state = code;
}

/**
 * Interface
 */
int Appender::getStateCode()
{
	return state;
}

void Appender::resetState()
{
	this->setStateCode(0); 
}

bool Appender::resetDevice(FILE* device)
{
	this->device = device;
	this->resetState();
	
	return true; 
}

bool Appender::resetLayout(Layout* layout)
{
	this->layout = layout;
	this->resetState();
	
	return true; 
}

bool Appender::print(string msg/*=string()*/)
{
	if(NULL==layout || EOF==fputs(layout->parser(msg).c_str(), this->device))
	{
		this->setStateCode(3);
		return false;
	}
	else
	{
		return true;
	}
}

