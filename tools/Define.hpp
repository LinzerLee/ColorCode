/******************************************
 *
 *      NAME:   Define.def 
 *      DESC:   Some macro defination to
 *				enhance C plus plus semantic. 
 *      Author: Linzer Lee
 *      Date:   2016/06/07
 *		Version:V1.0.0
 *
 ******************************************/

#ifndef DEFINE_DEF
#define DEFINE_DEF

#if _MSC_VER 
#ifdef DLL_IMPL
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __declspec(dllimport)
#endif // DLL_IMPL
#else  
#define EXPORT_API
#endif // _MSC_VER

#define _abstract
#define _final 
#define _override virtual
#define _interface
#define _implements :public 
#define _extends	:public 

#endif /* DEFINE_DEF */ 
