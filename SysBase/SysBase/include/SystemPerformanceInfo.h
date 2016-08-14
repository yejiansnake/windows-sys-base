#pragma once

#include "SysBase.h"
#include "ProcessInfo.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //进程CPU使用率

    struct TProcessCpuUsage 
    {
        UINT8 nCpuUsage;        //进程CPU使用率
    };

    //////////////////////////////////////////////////////////////////////////
    //高级进程信息

    struct TProcessPerformance
    {
        TProcessNormal aNormal;

        TProcessMemory aMemory;

        TProcessIOCounter aIOCounter;

        TProcessResource aResource;

        TProcessAdvanced aAdvanced;

        TProcessTime aTime;

        TProcessCpuUsage aCpuUsage;
    };

    //KEY:进程ID; VALUE:进程信息
    typedef map<UINT32, TProcessPerformance> ProcessPerformanceMap;

    //////////////////////////////////////////////////////////////////////////
    //系统CPU使用率

    struct TSystemCpuUsage 
    {
        UINT8 nSystemCpuUsage;
        UINT8 nSystemIdleCpuUsage;
    };

    //////////////////////////////////////////////////////////////////////////
    //系统资源信息

    struct TSystemResource
    {
        UINT64 nCommitTotal;            //系统提交字节数
        UINT64 nCommitLimit;            //系统当前能提交的最大字节数
        UINT64 nCommitPeak;             //当前系统已提交的最大字节数
        UINT64 nPhysicalTotal;          //物理内存字节总数
        UINT64 nPhysicalAvailable;      //用户进程可用的物理内存字节数
        UINT64 nSystemCache;            //系统系统缓存内存字节数
        UINT64 nKernelTotal;            //系统核心池总共字节数
        UINT64 nKernelPaged;            //系统核心池已分页的字节数
        UINT64 nKernelNonpaged;         //系统核心池未分页的字节数
        UINT64 nPageSize;               //系统每页的大小（字节）
        UINT32 nHandleCount;            //句柄数
        UINT32 nProcessCount;           //进程数
        UINT32 nThreadCount;            //线程数
    };

    //////////////////////////////////////////////////////////////////////////
    //系统信息

    struct TSystemPerformance
    {
        TSystemResource aResource;

        TSystemCpuUsage aCupUsage;
    };
}
