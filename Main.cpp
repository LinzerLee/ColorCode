#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

#ifdef _WIN32
#include "dirent.h"
#else
#include <direct.h>
#include <unistd.h>
#endif

#include "tools/Define.hpp"
#include "tools/Utility.h"
#include "tools/config/Parser.h"

#include "error.h"
#include "basetype.h" 
#include "bitmap.h" 
#include "colorcode.h"

using namespace std;
using namespace tools::config;

int usage()
{
	printf("usage: ColorCode [Options] [Params]\n");
	printf("Options:\n");
	printf("-H	--help		Print this message and exit\n");
	printf("Params:\n");
	printf("-INPUT	--doc set directory		Recognition *.BMP files from directory\n");
	printf("-CFG	--config file		Config file path\n");
	printf("-LOG	--log file		Log file path\n");

    return (EXIT_FAILURE);
}
/********************************************************************
 * Main
 ********************************************************************/
string GetColorCode(CONST CHAR *strFile) {
	Bitmap bitmap;
    UINT uiErrCode = ERROR_SUCCESS;
    ColorCodeBuffer ccb;
	uiErrCode = LoadBitmapFromFile(strFile, &bitmap); 
    InitColorCodeBuffer(&bitmap, &ccb);
    ReleaseBitmap(&bitmap);
    
    uiErrCode = ERROR_SUCCESS!=uiErrCode ? uiErrCode : EnqueueProcessStepQueue(PT_SIMILARITY); 
    uiErrCode = ERROR_SUCCESS!=uiErrCode ? uiErrCode : EnqueueProcessStepQueue(PT_SCAN_BORDER);
    uiErrCode = ERROR_SUCCESS!=uiErrCode ? uiErrCode : EnqueueProcessStepQueue(PT_STRENGTHEN_PARTITION);
    uiErrCode = ERROR_SUCCESS!=uiErrCode ? uiErrCode : EnqueueProcessStepQueue(PT_RECOGNITION_COLORCODE);
    
    uiErrCode = ERROR_SUCCESS!=uiErrCode ? uiErrCode : ProcessData(&ccb); 
//    PrintReserved(&ccb); 
	// ConvertCCB2Bitmap(&ccb, &bitmap);
	// SaveBitmapToFile("bak.bmp", &bitmap); 
    // ReleaseBitmap(&bitmap);
    string code;
    
    if(ERROR_SUCCESS==uiErrCode) {
    	
    	for(UINT i=0; i<ccb.bRowSize; ++i) {
		    for(UINT j=0; j<ccb.bColSize; ++j) {
		    	code += ColorCodeToString(ccb.abCodePoint[i][j]-1);
			} 
		} 
	}
	uiErrCode = ERROR_SUCCESS==uiErrCode ? uiErrCode : ReleaseColorCodeBuffer(&ccb); 
     
	return code;
} 

string ParseColorCode(CONST CHAR *code) {
	Parser parser;
	if(parser.loadConfig("./db/data.db")) {
		parser.handle();  
		vector<string> result = parser.config(code); 
	    if(result.size()>0) {
	    	return parser.config(code)[0];
		} 
	} 
	
	return ""; 
} 

INT main(INT argc, CHAR *argv[])
{
	map<string, vector<string> > command_params = Parser::CommandParams(argc, argv);
	// H选项检测
	if (command_params.count("H") > 0)
	{
		usage();
		printf("Exit Code : %d.", EXIT_SUCCESS);
		return EXIT_SUCCESS;
	}
	
	if (argc<2) {
		return usage();
	}
	
	// Utility组件加载器
	printf("Lodding Utility component...\n");
	if (!loader(command_params))
	{
		printf("Lode Utility component incomplete.\n");
		printf("Error Code : %d\n", EXIT_FAILURE);
		return EXIT_FAILURE;
	}
	
	Log("[ColorCode]");
	Log("Lode Utility component complete.");
	StartTimer();
	
	// S选项检测--相似度阈值 
	if (command_params.count("S") > 0)
	{
		char* error_point = NULL;
		SetSimilarityThreshold(strtod(command_params["S"][0].c_str(), &error_point)); 
	}
	
	DIR* pdir = NULL;
	dirent* ptr = NULL;
	char path[512];
	string output_file, input_dir;
	
	if (command_params["INPUT"].size() == 1)
	{
		input_dir = command_params["INPUT"][0];
		auto pos = input_dir.size() - 1;

		if ('\\' == input_dir[pos] || '/' == input_dir[pos])
			input_dir.erase(pos, 1);
	}
	
	// 打开INPUT目录 
	if (!(pdir = opendir(input_dir.c_str())))
	{
		Log("打开目录失败 : %s", input_dir.c_str());
		Log("Exit Code : %d.", EXIT_FAILURE);
		return EXIT_FAILURE;
	}
	
	// 循环取出待识别的图片 
	while ((ptr = readdir(pdir)) != 0)
	{
		if('.'==ptr->d_name[0]) {
			continue;
		}
		sprintf(path, "%s/%s", input_dir.c_str(), ptr->d_name);
		string code = GetColorCode(path);
		string info = ParseColorCode(code.c_str()); 
		if(""!=info) {
			Log("%s recognition result : [%s] %s", ptr->d_name, code.c_str(), info.c_str());
		    info = "start " + info; 
		    system(info.c_str());
		} else {
			Log("Sorry, %s can't recognition[%s].", ptr->d_name, code.c_str()); 
		} 
	}

	closedir(pdir);
     
    return 0; 
}
