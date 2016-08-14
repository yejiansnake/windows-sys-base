#pragma once

#include "SysBase.h"

#define MAX_SYSBASE_USER_NAME_SIZE 41
#define MAX_SYSBASE_USER_NAME_LEN 40

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //一般信息

    struct TProcessNormal
    {
        UINT32  nProcessID;             //进程ID
        UINT32  nParentProcessID;       //父进程ID
        UINT32  nThreadCount;           //线程数
        INT32   nPriClassBase;          //基础优先级
        WCHAR   szExeFile[MAX_PATH];    //进程名称
    };

    //////////////////////////////////////////////////////////////////////////
    //内存信息

    struct TProcessMemory
    {
        UINT32  nPageFaultCount;              //Number of page faults
        UINT64  nPeakWorkingSetSize;          //峰值内存使用
        UINT64  nWorkingSetSize;              //当前内存使用
        UINT64  nQuotaPeakPagedPoolUsage;     //峰值页面缓冲池
        UINT64  nQuotaPagedPoolUsage;         //当前页面缓冲池
        UINT64  nQuotaPeakNonPagedPoolUsage;  //峰值分页面缓冲池
        UINT64  nQuotaNonPagedPoolUsage;      //当前非页面缓冲池
        UINT64  nPagefileUsage;               //当前虚拟内存使用
        UINT64  nPeakPagefileUsage;           //峰值虚拟内存使用
    };

    //////////////////////////////////////////////////////////////////////////
    //IO计数 

    struct TProcessIOCounter
    {
        UINT64  nReadOperationCount;        //读次数
        UINT64  nWriteOperationCount;       //写次数
        UINT64  nOtherOperationCount;       //其它读写次数
        UINT64  nReadTransferCount;         //读-字节数
        UINT64  nWriteTransferCount;        //写-字节数
        UINT64  nOtherTransferCount;        //其它读写-字节数
    };

    //////////////////////////////////////////////////////////////////////////
    //进程资源

    struct TProcessResource
    {
        UINT32  nHandleCount;               //句柄数
        UINT32  nGDICount;                  //GDI数
        UINT32  nUserObjectCount;           //用户对象数
    };

    //////////////////////////////////////////////////////////////////////////
    //高级信息
#define COMAND_MAX_PATH			1024
    struct TProcessAdvanced
    {
        UINT8   nPriority;                                  //优先级（0:无内容; 1:低; 2:低于标准; 3:普通; 4:高于标准; 5:高; 6:最高）
        bool    bIs64;                                      //是否为64位进程
        UINT32  nSessionID;                                 //Terminal Services session （终端服务会话编号）
        WCHAR   szImageFileName[MAX_PATH];                  //获取进程的文件名全程（包含路径）
		WCHAR   szCommandLine[COMAND_MAX_PATH];                  //获取进程的文件名全程（包含路径）
        WCHAR   szUserName[MAX_SYSBASE_USER_NAME_SIZE];     //用户名
    };

    //////////////////////////////////////////////////////////////////////////
    //时间信息

    struct TProcessTime
    {
        INT64  nCreateTime;                 //进程创建时间
        INT64  nExitTime;                   //进程退出时间
        INT64  nKernelTime;                 //进程核心时间
        INT64  nUserTime;                   //进程用户时间
    };

    //////////////////////////////////////////////////////////////////////////
    //进程信息

    struct TProcessInfo 
    {
        TProcessNormal aNormal;

        TProcessMemory aMemory;

        TProcessIOCounter aIOCounter;

        TProcessResource aResource;

        TProcessAdvanced aAdvanced;

        TProcessTime aTime;
    };
}
