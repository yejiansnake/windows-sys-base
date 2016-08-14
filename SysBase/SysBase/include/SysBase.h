#pragma once

//////////////////////////////////////////////////////////////////////////
//版本号
#define SYSBASE_VERSION_NUM "3.0.3"

//////////////////////////////////////////////////////////////////////////
//忽略的警告

//这两个警告与STL和模板有关
#pragma warning(disable: 4275)
#pragma warning(disable: 4251)
#pragma warning(disable: 4091)

//////////////////////////////////////////////////////////////////////////

//#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
//#define _WIN32_WINNT 0x0502	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
//#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif

#define _CRT_RAND_S		// for rand_s().

//////////////////////////////////////////////////////////////////////////
//支持平台 WIN XP SP2, 2003


#include <new.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include <winsvc.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <Commdlg.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <deque>

using namespace std;

//Shell
#pragma comment (lib, "shlwapi")
#pragma comment(lib, "shell32.lib")

//用户管理
#include <Lmaccess.h>
#include <LMErr.h>
#include <LM.h>
#include <iphlpapi.h>
#pragma comment(lib, "Netapi32.lib")

//////////////////////////////////////////////////////////////////////////

typedef const BYTE* LPCBYTE;

#ifndef WIN32
#ifndef WCHAR
#define WCHAR wchar_t
#endif
#endif // !WIN32

typedef const char* PCCHAR;
typedef const wchar_t* PCWCHAR;

typedef const void* PCVOID;

//////////////////////////////////////////////////////////////////////////

//#define CREATE_AUTO_PTR(type, ptr) auto_ptr<type> aAutoPtr(ptr)
    
//创建唯一参数名

#define CREATE_UNIQUE_VAR_NAME_2(x,y) x##y
#define CREATE_UNIQUE_VAR_NAME_1(x,y) CREATE_UNIQUE_VAR_NAME_2(x,y)
#define CREATE_UNIQUE_VAR_NAME(x) CREATE_UNIQUE_VAR_NAME_1(x,__COUNTER__)

#define CREATE_AUTO_PTR(type, ptr) auto_ptr<type> CREATE_UNIQUE_VAR_NAME(_aAutoPtr)(ptr) 

#define HAS_OPSTION_RETURN(var, opstion) \
	if (var & opstion) \
{\
	return TRUE;\
}\
	return FALSE;

//////////////////////////////////////////////////////////////////////////

//线程停止前等待的时间
#define SYSBASE_STOP_WAIT_TIME 3000

//////////////////////////////////////////////////////////////////////////
//内存管理

#define NEW_NOTHROW new(std::nothrow)

#define _NEW(varName, type) \
	type* varName = new(std::nothrow) type();

#define _NEWA(varName, type, count) \
	type* varName = new(std::nothrow) type[count];

#define _NEW_RV(varName, type, returnValue) \
	type* varName = new(std::nothrow) type();\
	return returnValue;

#define _NEWA_RV(varName, type, count, returnValue) \
	type* varName = new(std::nothrow) type[count];\
	return returnValue;

#define _NEW_EX(varName, type) \
	varName = new(std::nothrow) type();

#define _NEWA_EX(varName, type, count) \
	varName = new(std::nothrow) type[count];

#define _NEW_EX_RV(varName, type, returnValue) \
	varName = new(std::nothrow) type();\
	return returnValue;

#define _NEWA_EX_RV(varName, type, count, returnValue) \
	varName = new(std::nothrow) type[count];\
	return returnValue;

#define _DELETE(p) \
	if (p)\
{\
	delete (p);\
	(p) = NULL;\
}

#define _DELETEA(p) \
	if (p)\
{\
	delete[] (p);\
	(p) = NULL;\
}

//////////////////////////////////////////////////////////////////////////

namespace SysBase
{
    //对象接口
    class CObject
    {
    public:
    
        CObject(){}
        
        virtual ~CObject(){};

    protected:

    private:

    };
}
