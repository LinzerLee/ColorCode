#ifndef LOGGER_H
#define LOGGER_H

#include <vector>

#include "../Define.hpp"

namespace tools
{
	namespace log
	{
		class Appender; 
		_final class EXPORT_API Logger
		{
			public:
				/**
				 * Constructor
				 */
				Logger();
				
				/**
				 * Destructor
				 */
				~Logger();
				
				/**
				 * Interface
				 */
				int getStateCode();
				void resetState();  
				int addAppender(Appender* appender); 
				bool removeAppender(unsigned int code); 
				bool removeAppender(Appender* appender);
				bool log(std::string msg= std::string());
			protected:
				/**
				 * Initialize
				 */
				bool _init();
				
				/**
				 * Utillity
				 */
				void setStateCode(int code);				
				 
			private:
				/**
				 * Property
				 */
				int state;		// Layout状态: 0:正常, -1:错误, 2:无输出设备, 3:设备写入错误
				std::vector<Appender*> appenders;
		};
	}
}
#endif
