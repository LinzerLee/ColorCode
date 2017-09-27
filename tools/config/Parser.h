#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <map>
#include <vector>

#include "../Define.hpp"

namespace tools
{
	namespace config
	{
		_final class EXPORT_API Parser
		{
			public:
				/**
				 * Static Interface 
				 */
				static std::map<std::string, std::vector<std::string> > CommandParams(int argc,char *argv[]);
				
				/**
				 * Constructor
				 */
				Parser();
				Parser(const char*cfgpath);
				
				/**
				 * Destructor
				 */
				~Parser();
				
				/**
				 * Interface
				 */ 
				int getStateCode();
				void resetState();  
				bool loadConfig(const char*cfg);
				std::string getContent();
				std::map<std::string, std::vector<std::string>>handle();
				void config(const std::string &key, const std::vector<std::string> &vaule);
				std::vector<std::string> config(const std::string &key);
				void mergeConfig(std::map<std::string, std::vector<std::string>> config);
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
				// 解析器状态: 0:正常, 1:配置文件未正确加载 2:配置文件语法错误 3:待处理 
				int state;
				std::string filepath;
				std::string content;
				std::map<std::string, std::vector<std::string>> cfg;
		};
	}
}
#endif
