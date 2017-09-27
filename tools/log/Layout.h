#ifndef LAYOUT_H
#define LAYOUT_H

#include <string>
#include <map>

#include "../Define.hpp"

namespace tools
{
	namespace log
	{
		_final class EXPORT_API Layout
		{
			public:
				/**
				 * Constructor
				 */
				Layout();
				Layout(std::string pattern, std::map<std::string, std::string> params/*=map<string, string>()*/);
				
				/**
				 * Destructor
				 */
				~Layout();
				
				/**
				 * Interface
				 */
				int getStateCode();
				void resetState(); 
				void resetPattern(std::string pattern);
				std::string getPattern();
				void resetParams(std::map<std::string, std::string> params);
				std::map<std::string, std::string> getParams();
				void resetParam(std::string key, std::string value);
				std::string getParam(std::string key);
				std::string parser(std::string msg= std::string());
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
				int state;		// Layout×´Ì¬: 0:Õý³£, -1:´íÎó
				std::map<std::string, std::string> params;
				std::map<std::string, std::string> default_params;
				std::string pattern;
		};
	}
}
#endif
