#pragma once

#include "SysBase.h"
#include "SystemPerformanceInfo.h"
#include "SingleMgr.h"
#include "ThreadSync.h"
#include "Thread.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CProcUsageCollecter

    class CSystemPerformanceCollecter : public CSingletonT<CSystemPerformanceCollecter>, private SysBase::CObject
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_PARAM, 
            ERROR_CODE_SYSTEM,
            ERROR_CODE_HAS_START,
            ERROR_CODE_NO_START,
            ERROR_CODE_NO_ASSIGN,       //启动的模式不为收集指定进程
        };

        enum COLLECT_TYPE
        {
            COLLECT_TYPE_PROC_BASE = 0,           //基本信息[必有]（进程：Normal, Time, CpuUsage; 系统: CpuUsage, CpuIdle）
            COLLECT_TYPE_PROC_MEMORY = 1,         //进程内存信息
            COLLECT_TYPE_PROC_IO = 2,             //进程IO计数
            COLLECT_TYPE_PROC_RESOURCE = 4,       //进程资源
            COLLECT_TYPE_PROC_ADVANCED = 8,       //进程高级信息
            COLLECT_TYPE_SYS_RESOURCE = 16,       //系统资源(内存，句柄数等)
        };

        enum COLLECT_MODE
        {
            COLLECT_MODE_ALL = 0,       //收集所有进程信息
            COLLECT_MODE_ASSIGN,        //收集指定进程信息
        };

        //////////////////////////////////////////////////////////////////////////

        friend class CSingletonT<CSystemPerformanceCollecter>;

        virtual ~CSystemPerformanceCollecter();

        //////////////////////////////////////////////////////////////////////////

        ERROR_CODE Start(
            UINT32 collectTypes = COLLECT_TYPE_PROC_BASE, 
            COLLECT_MODE collectMode = COLLECT_MODE_ALL,
            UINT32 nIntervalTime = 1000,
            bool bSuspend = false);

        void Stop();

        ERROR_CODE Suspend();

        ERROR_CODE Resume();

        ERROR_CODE SetIntervalTime(UINT32 nIntervalTime);

        ERROR_CODE SetCollectTypes(UINT32 collectTypes);

        ERROR_CODE AddProcess(const wchar_t* szProcessName);

        ERROR_CODE RemoveProcess(const wchar_t* szProcessName);

        ERROR_CODE Get(ProcessPerformanceMap* pProcessPerformanceMap, 
            TSystemPerformance* pTSystemPerformance);

    private:

        CSystemPerformanceCollecter();

        void* m_pImp;
    };

}
