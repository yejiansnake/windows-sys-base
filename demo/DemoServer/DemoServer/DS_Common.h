#pragma once

#define APP_VER 100001

//////////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4099)

//////////////////////////////////////////////////////////////////////////

#define LOG_KEY_SYSTEM 0
#define LOG_KEY_CLIENT 1
#define LOG_KEY_STATISTICS 2

#define MAX_IP_SIZE 50
#define MAX_IP_LEN 49
#define MAX_SEND_BUFFER_SIZE 10240

#define MAX_MD5_SIZE 51
#define MAX_MD5_LEN 50

//////////////////////////////////////////////////////////////////////////
//协议文件

#include "DS_Protocol_Common.h"

//////////////////////////////////////////////////////////////////////////

#include "../../../SysBase/SysBase/SysBase_Interface.h"

using namespace SysBase;

#ifdef _DEBUG
#ifdef X64
#pragma comment(lib, "../../../SysBase/lib/SysBase_d_64.lib")
#else
#pragma comment(lib, "../../../SysBase/lib/SysBase_d.lib")
#endif // !WIN32
#else
#ifdef X64
#pragma comment(lib, "../../../SysBase/lib/SysBase_64.lib")
#else
#pragma comment(lib, "../../../SysBase/lib/SysBase.lib")
#endif // !WIN32
#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////

struct TSocketInfo
{
    UINT32 nSocketID;
    UINT32 nIP;
    UINT16 nPort;
    UINT16 nRecv;
    UINT32 nDataSize;
};