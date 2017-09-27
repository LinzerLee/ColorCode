#ifndef APPENDER_H
#define APPENDER_H

#include <cstdio>
#include <string>

#include "../Define.hpp"

namespace tools
{
	namespace log
	{
		class Layout;
		_final class EXPORT_API Appender
		{
			public:
				/**
				 * Constructor
				 */
				Appender();
				Appender(Layout* layout, FILE* device=stderr);
				
				/**
				 * Destructor
				 */
				~Appender();
				
				/**
				 * Interface
				 */
				int getStateCode();
				void resetState();  
				bool resetDevice(FILE* device); 
				bool resetLayout(Layout* layout); 
				bool print(std::string msg= std::string());
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
				int state;		// Appender状态: 0:正常, -1:错误, 2:无输出设备, 3:设备写入错误
				Layout *layout;
				FILE *device; 
		};
	}
}
#endif
