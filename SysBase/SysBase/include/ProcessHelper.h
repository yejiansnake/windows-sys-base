#pragma once

#include "SysBase.h"

namespace SysBase
{
    class CProcessHelper : public SysBase::CObject
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        enum PROCESS_INFO_TYPE
        {
            PROCESS_INFO_TYPE_NORMAL = 0,           //正常信息（必有）
            PROCESS_INFO_TYPE_MEMORY = 1,           //内存信息
            PROCESS_INFO_TYPE_IO = 2,               //IO计数
            PROCESS_INFO_TYPE_RESOURCE = 4,         //资源
            PROCESS_INFO_TYPE_ADVANCED = 8,         //高级信息
            PROCESS_INFO_TYPE_TIME = 16,            //时间信息
        };
	
        //////////////////////////////////////////////////////////////////////////

        static bool GetAllProcessInfo(
            list<TProcessInfo>& processInfoList,
            UINT32 nProcessInfoTypes = PROCESS_INFO_TYPE_NORMAL);

        static bool GetProcessInfo(
            UINT32 nProcessID, 
            TProcessInfo& processInfo, 
            UINT32 nProcessInfoTypes = PROCESS_INFO_TYPE_NORMAL);

        static bool GetProcessIDListByName(const char* szProcessName, list<UINT32>& processIDList);

        static bool GetProcessIDListByNameW(const WCHAR* szProcessName, list<UINT32>& processIDList);

        //获取系统进程数（包含2个系统默认进程：系统进程 与 系统idle进程）
        static UINT32 GetProcessCount();

        static bool EnableDebugPrivilege(bool bEnable);

        //////////////////////////////////////////////////////////////////////////

    private:

        CProcessHelper();

        virtual ~CProcessHelper();

        static CProcessHelper s_CProcessHelper;
    };
}
