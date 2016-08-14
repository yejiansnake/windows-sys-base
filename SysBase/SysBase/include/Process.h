#pragma once

#include "SysBase.h"
#include "ProcessInfo.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CProcess 进程类

	class CProcess
	{
	public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_NOT_CREATE_OR_OPEN,
            ERROR_CODE_PARAM,
            ERROR_CODE_SYSTEM,
            ERROR_CODE_HAS_INIT,
            ERROR_CODE_NO_EXIST,
            ERROR_CODE_TIME_OUT,
            ERROR_CODE_PROC_NOT_FIND,
        };

        enum OPEN_RIGHT_TYPE
        {
            OPEN_RIGHT_TYPE_ALL = 0,            //所有权限
            OPEN_RIGHT_TYPE_QUERY_LIMITED,      //最少进程信息查询权限
            OPEN_RIGHT_TYPE_QUERY_ALL,          //所有进程信息查询权限
        };

        //////////////////////////////////////////////////////////////////////////

		CProcess();

		virtual ~CProcess();

		//功能:创建进程
		//参数:
		//      char* lpCommandLine IN 启动进程的命令行
		//      bool bRun IN ture 时立即启动; false 时线程创建后暂停直到调用 Resume 方法时才启动
        //      bool bShowWindow IN ture 时显示窗口; false 时不显示
		ERROR_CODE Create(char* lpCommandLine, bool bRun = true, bool bShowWindow = true);

        //功能:创建进程
        //参数:
        //      char* lpCommandLine IN 启动进程的命令行
        //      bool bRun IN ture 时立即启动; false 时线程创建后暂停直到调用 Resume 方法时才启动
        //      bool bShowWindow IN ture 时显示窗口; false 时不显示
        ERROR_CODE Create(WCHAR* lpCommandLine, bool bRun = true, bool bShowWindow = true);

        //功能：恢复进程运行
        ERROR_CODE Resume();

		//功能:打开一个已有的进程
		//参数:
		//      UINT32 nProcessID IN 进程ID
        //      OPEN_RIGHT_TYPE openRightType IN 访问权限
		ERROR_CODE Open(UINT32 nProcessID, OPEN_RIGHT_TYPE openRightType = OPEN_RIGHT_TYPE_ALL);

		//功能:根据进程名获取已经打开的进程
		//参数:
		//      const char* szProcessName IN 进程名
		//异常
		ERROR_CODE OpenByName(const char* szProcessName, OPEN_RIGHT_TYPE openRightType = OPEN_RIGHT_TYPE_ALL);

        //功能:打开当前进程(需要释放)
        ERROR_CODE OpenCurrent();

		//功能:关闭线程
		void Close();

        //功能:中止进程
        //参数:
        //      UINT32 nExitCode IN 进程退出代码
        ERROR_CODE Terminate(UINT32 nExitCode);
        
        //功能：等待进程是否退出
        ERROR_CODE Wait(UINT32 nWaitTime);

		//功能:获取进程返回码
		//参数:
		//      UINT32* pnExitCode IN 进程返回的代码
		//返回值: 线程实例指针
		ERROR_CODE GetExitCode(UINT32* pnExitCode);

        //功能：尽可能的释放不再使用的内存
        ERROR_CODE ReleaseMemory();

		//功能:获取进程ID
		//返回值: 进程ID
		ERROR_CODE GetProcessID(UINT32& nProcessID);

        ERROR_CODE GetProcessNormal(TProcessNormal& aTProcessNormal);

        ERROR_CODE GetProcessMemory(TProcessMemory& aTProcessMemory);

        ERROR_CODE GetProcessIOCounter(TProcessIOCounter& aTProcessIOCounter);

        ERROR_CODE GetProcessResource(TProcessResource& aTProcessResource);

        ERROR_CODE GetProcessAdvanced(TProcessAdvanced& aTProcessAdvanced);

        ERROR_CODE GetProcessTime(TProcessTime& aTProcessTime);
        
		//////////////////////////////////////////////////////////////////////////
		//静态函数

		//功能:获取当前进程命令行
		//返回值: 进程命令行
		static const char* GetCurrentCommandLine();

        static UINT32 GetCurProcessID();

        static bool KillProcess(UINT32 nProcessID, UINT32 nExitCode = 0);

        //功能:创建独立进程（创建后不再关联该子进程）
        //参数:
        //      char* lpCommandLine IN 启动进程的命令行
        //      bool bShowWindow IN ture 时显示窗口; false 时不显示
        static bool RunProcesss(const char* szCmdLine,UINT32& nProcessID, const char* strCurrentDir,bool bShowWindow = true);

        //功能:创建独立进程（创建后不再关联该子进程）
        //参数:
        //      char* lpCommandLine IN 启动进程的命令行
        //      bool bShowWindow IN ture 时显示窗口; false 时不显示
        static bool RunProcesss(const WCHAR* szCmdLine, UINT32& nProcessID,const WCHAR* strCurrentDir,bool bShowWindow = true);

	protected:

		void* m_pImp;
	};
}