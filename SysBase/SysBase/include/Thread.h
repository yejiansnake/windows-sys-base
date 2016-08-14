#pragma once

#include "SysBase.h"
#include "Process.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //IThreadProcHandler CThread类线程执行细节

    class IThreadProcHandler
    {
    public:

        IThreadProcHandler(){};

        virtual ~IThreadProcHandler(){};

        virtual void Proc() = 0;

    protected:
    private:
    };

	//////////////////////////////////////////////////////////////////////////
	//CThread 线程类

    class CThread : public SysBase::CObject
	{
	public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_PARAM, 
            ERROR_CODE_SYSTEM,
            ERROR_CODE_HAS_INIT,
        };

        //////////////////////////////////////////////////////////////////////////

		CThread();

		virtual ~CThread();

		//功能:创建线程
		//参数:
		//      IThreadProcHandler* pIThreadProcHandler IN 线程执行逻辑
		//      bool bRun IN 为 ture 是线程逻辑马上执行，为 false 时则需要调用 Resume 才执行
		//异常
		ERROR_CODE Start(
			IThreadProcHandler* pIThreadProcHandler,
			bool bRun = true);//CREATE_SUSPENDED

		//功能:关闭线程
		void Close();

		//功能:启动线程
		//返回值: 启动成功与否
		bool Resume();

		//功能:中止线程
		//参数:
		//		UINT32 nWaitTime IN 等待线程时间
		//      UINT32 nExitCode IN 线程退出代码
		//返回值: 中止成功与否
		bool Terminate(UINT32 nWaitTime, UINT32 nExitCode);

		//功能:挂起线程
		//返回值: 挂起成功与否
		bool Suspend();

		//功能:获取线程ID
		//返回值: 线程ID
		UINT32 GetThreadID();

		//功能:获取线程退出代码
		//参数:
		//      UINT32* pnExitCode OUT 线程退出返回码
		//返回值: 中止成功与否
		bool GetExitCode(UINT32* pnExitCode);

		//////////////////////////////////////////////////////////////////////////
		//静态函数

		//功能:暂停线程指定的时间（线程函数使用）
		//参数:
		//      UINT32 nWaitTime IN 时间，单位为毫秒 
		//返回值: 中止成功与否
		static void SleepThread(UINT32 nWaitTime);

		//功能:退出当前线程（线程函数使用）
		//参数:
		//      UINT32 nExitCode IN 线程退出代码
		static void ExitCurrentThread(UINT32 nExitCode);

		//功能:获取当前线程ID（线程函数使用）
		//返回值: 线程ID
		static UINT32 GetCurrentThreadID();

	private:

		void* m_pImp;
	};

	//////////////////////////////////////////////////////////////////////////
	//CCustomThread 自定义线程类，该类只能通过继承来使用，必须实现Proc接口

    class CCustomThread : public IThreadProcHandler, public SysBase::CObject
	{
	public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_PARAM, 
            ERROR_CODE_SYSTEM,
            ERROR_CODE_HAS_INIT,
        };

        //////////////////////////////////////////////////////////////////////////

		CCustomThread();

		virtual ~CCustomThread();

		//功能:启动线程
		ERROR_CODE Start();

		//功能:停止线程
		//参数:
		//      UINT32 nWaitTime IN 等待多时秒后中止线程
		//      UINT32 nExitCode IN 线程退出代码
		void Stop(UINT32 nWaitTime = SYSBASE_STOP_WAIT_TIME, UINT32 nExitCode = 0);

		//功能:获取线程ID
		//返回值: 线程ID
		UINT32 GetThreadID();

	protected:

		//功能:线程处理过程
		//返回值: 线程返回值
		virtual void Proc() = 0;

	private:

		void* m_pImp;
	};

    //////////////////////////////////////////////////////////////////////////
    //CTimeThread 提供指定时间触发回调事件的独立线程功能

    class CTimeThread : private CCustomThread
    {
    public:
        
        //////////////////////////////////////////////////////////////////////////

        CTimeThread();

        virtual ~CTimeThread();

        //功能:启动线程
        //参数:
        //      UINT32 nWaitTime IN 定时的等待时间
        //      bool bRunFirst IN 线程启动后线调用一次OnTime再进入定时等待
        bool Start(UINT32 nWaitTime = 1000, bool bRunFirst = false);

        //功能:停止线程
        //参数:
        //      UINT32 nWaitTime IN 等待多时秒后中止线程
        //      UINT32 nExitCode IN 线程退出代码
        void Stop(UINT32 nWaitTime = SYSBASE_STOP_WAIT_TIME, UINT32 nExitCode = 0);

        void SetWaitTime(UINT32 nWaitTime);

        //功能:获取线程ID
        //返回值: 线程ID
        UINT32 GetThreadID();

    protected:

        //指定时间到达后调用该事件
        virtual void ProcOnTime() = 0;

    private:

        virtual void Proc();

        UINT32 m_nWaitTime;
        bool m_bRunFirst;

        bool m_bStop;
        CAutoEvent m_event;
    };

	//////////////////////////////////////////////////////////////////////////
	//CCustomMultiThread 自定义多线程类，该类只能通过继承来使用，必须实现Proc接口

	class CCustomMultiThread : public IThreadProcHandler, public SysBase::CObject
	{
	public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_PARAM, 
            ERROR_CODE_SYSTEM,
            ERROR_CODE_HAS_INIT,
        };

        //////////////////////////////////////////////////////////////////////////

		CCustomMultiThread();

		virtual ~CCustomMultiThread();

		//功能:启动创建线程池
		//参数:
		//      unsigned short sThreadCount IN 线程个数
		//异常
		virtual ERROR_CODE Start(unsigned short sThreadCount);

		//功能:等待多少毫秒后结束停止并且关闭所有线程
		//参数:
		//      UINT32 nWaitTime IN 多少毫秒,如果为 0 时马上结束所有线程
		virtual void Stop(UINT32 nWaitTime = SYSBASE_STOP_WAIT_TIME);

		//功能:获取线程编号集合
		//参数:
		//      unsigned short* psThreadCount OUT 线程编号个数
		//返回值:
		//		编号数组地址[如果没有则返回NULL]
		//异常
		UINT32* GetThreadIDs(unsigned short* psThreadCount);

	protected:

		//功能:线程处理过程
		//返回值: 线程返回值
		virtual void Proc() = 0;

	private:

		void* m_pImp;
	};

    //////////////////////////////////////////////////////////////////////////
    //TTaskThrreadInfo

    struct TTaskInfo
    {
        UINT32 nThreadID;
    };

	//////////////////////////////////////////////////////////////////////////
	//ITaskHandler 任务处理者对象

	class ITaskHandler
	{
	public:

		ITaskHandler();

		virtual ~ITaskHandler();

        //事件：线程处理
		virtual void Proc(void* pParam) = 0;

        //事件：接收到线程关闭消息通知（此时线程还未关闭）
		virtual void Stop(const TTaskInfo* pTTaskInfo) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	//ITaskHandlerFactory 任务工厂

	class ITaskHandlerFactory
	{
	public:

		ITaskHandlerFactory();

		virtual ~ITaskHandlerFactory();

		virtual ITaskHandler* Create() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	//CTaskMultiThread 多线程执行任务

	class CTaskMultiThread : public SysBase::CObject
	{
	public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_PARAM, 
            ERROR_CODE_SYSTEM,
            ERROR_CODE_HAS_INIT,
        };

        //////////////////////////////////////////////////////////////////////////

		//功能:初始化
		CTaskMultiThread();

		virtual ~CTaskMultiThread();

		//功能:启动创建线程池
		//参数:
		//	    ITaskHandlerFactory *pITaskHandlerFactory IN 任务工厂对象
		//      unsigned short sThreadCount IN 线程个数
		//      const void* lpParam IN 线程函数的参数
		//异常
		ERROR_CODE Start(ITaskHandlerFactory *pITaskHandlerFactory, unsigned short sThreadCount, void* lpParam = NULL);

		//功能:等待多少毫秒后结束停止并且关闭所有线程
		//参数:
		//      UINT32 nWaitTime IN 多少毫秒,如果为 0 时马上结束所有线程
		void Stop(UINT32 nWaitTime = SYSBASE_STOP_WAIT_TIME);

		//功能:获取线程ID数组
		//参数:
		//      vector<UINT32> *pThreadIDArray OUT 线程ID数组
		void GetThreadIDArray(vector<UINT32>* pThreadIDArray);

	private:

		void* m_pImp;
	};
}