#include "../SysBase_Interface.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CThreadImp 线程类

    class CThreadImp
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        CThreadImp();

        virtual ~CThreadImp();

        CThread::ERROR_CODE Start(
            IThreadProcHandler* pIThreadProcHandler,
            bool bRun = true);

        void Close();

        bool Resume();

        bool Terminate(UINT32 nWaitTime, UINT32 nExitCode);

        bool Suspend();

        UINT32 GetThreadID();

        bool GetExitCode(UINT32* pnExitCode);

    private:

        static DWORD WINAPI ThreadProc(LPVOID lpParam);

        ///////////////////////////////
        //数据成员

        UINT32 m_dwThreadId;

        HANDLE m_hThread;

        IThreadProcHandler* m_pIThreadProcHandler;
    };

	//////////////////////////////////////////////////////////////////////////
	//CThreadImp

	CThreadImp::CThreadImp()
	{
		m_hThread = NULL;
		m_dwThreadId = 0;
	}

	CThreadImp::~CThreadImp()
	{
		this->Close();
	}

	CThread::ERROR_CODE CThreadImp::Start(
		IThreadProcHandler* pIThreadProcHandler,
		bool bRun)
	{
		if (m_hThread)
		{
			return CThread::ERROR_CODE_HAS_INIT;
		}

        if (!pIThreadProcHandler)
        {
            return CThread::ERROR_CODE_PARAM;
        }

        DWORD dwFlag = 0;

        if (!bRun)
        {
            dwFlag = CREATE_SUSPENDED;
        }

        m_pIThreadProcHandler = pIThreadProcHandler;

		DWORD dwThreadId = 0;

		HANDLE hThread = CreateThread(NULL, 0, ThreadProc, this, dwFlag, &dwThreadId);

		if (!hThread)
		{
			return CThread::ERROR_CODE_SYSTEM;
		}

		m_dwThreadId = dwThreadId;

		m_hThread = hThread;

        return CThread::ERROR_CODE_SUCCESS;
	}

	void CThreadImp::Close()
	{
		if (m_hThread)
		{
            TerminateThread(m_hThread, 0);

			CloseHandle(m_hThread);

			m_hThread = NULL;
			m_dwThreadId = 0;

            m_pIThreadProcHandler = NULL;
		}
	}

	bool CThreadImp::Resume()
	{
		if (ResumeThread(m_hThread) == -1)
		{
			return FALSE;
		}

		return TRUE;
	}

	bool CThreadImp::Terminate(UINT32 nWaitTime, UINT32 nExitCode)
	{
		WaitForSingleObject(m_hThread, nWaitTime);

        if (TerminateThread(m_hThread, nExitCode))
        {
            return true;
        }

		return false;
    }

	bool CThreadImp::Suspend()
	{
		if (SuspendThread(m_hThread) == -1)
		{
			return false;
		}

		return true;
	}

	UINT32 CThreadImp::GetThreadID()
	{
		return m_dwThreadId;
	}

	bool CThreadImp::GetExitCode(UINT32* pnExitCode)
	{
        DWORD dwCode = 0;

		if (GetExitCodeThread(m_hThread, &dwCode))
        {
            *pnExitCode = dwCode;

            return true;
        }

        return false;
	}

    DWORD WINAPI CThreadImp::ThreadProc(LPVOID lpParam)
    {
        CThreadImp* pCThread = (CThreadImp*)lpParam;

        pCThread->m_pIThreadProcHandler->Proc();

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //CThread

    CThread::CThread()
    {
        m_pImp = new(std::nothrow) CThreadImp();
    }

    CThread::~CThread()
    {
        if (m_pImp)
        {
            CThreadImp* pCThreadImp = (CThreadImp*)m_pImp;

            delete pCThreadImp;

            m_pImp = NULL;
        }
    }

    CThread::ERROR_CODE CThread::Start(
        IThreadProcHandler* pIThreadProcHandler,
        bool bRun)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CThreadImp* pCThreadImp = (CThreadImp*)m_pImp;

        return pCThreadImp->Start(pIThreadProcHandler, bRun);
    }

    void CThread::Close()
    {
        if (!m_pImp)
        {
            return;
        }

        CThreadImp* pCThreadImp = (CThreadImp*)m_pImp;

        return pCThreadImp->Close();
    }

    bool CThread::Resume()
    {
        if (!m_pImp)
        {
            return false;
        }

        CThreadImp* pCThreadImp = (CThreadImp*)m_pImp;

        return pCThreadImp->Resume();
    }

    bool CThread::Terminate(UINT32 nWaitTime, UINT32 nExitCode)
    {
        if (!m_pImp)
        {
            return false;
        }

        CThreadImp* pCThreadImp = (CThreadImp*)m_pImp;

        return pCThreadImp->Terminate(nWaitTime, nExitCode);
    }

    bool CThread::Suspend()
    {
        if (!m_pImp)
        {
            return false;
        }

        CThreadImp* pCThreadImp = (CThreadImp*)m_pImp;

        return pCThreadImp->Suspend();
    }

    UINT32 CThread::GetThreadID()
    {
        if (!m_pImp)
        {
            return 0;
        }

        CThreadImp* pCThreadImp = (CThreadImp*)m_pImp;

        return pCThreadImp->GetThreadID();
    }

    bool CThread::GetExitCode(UINT32* pnExitCode)
    {
        if (!m_pImp)
        {
            return false;
        }

        CThreadImp* pCThreadImp = (CThreadImp*)m_pImp;

        return pCThreadImp->GetExitCode(pnExitCode);
    }

	void CThread::SleepThread(UINT32 nWaitTime)
	{
		Sleep(nWaitTime);
	}

	void CThread::ExitCurrentThread(UINT32 nExitCode)
	{
		ExitThread(nExitCode);
	}

	UINT32 CThread::GetCurrentThreadID()
	{
		return GetCurrentThreadId();
	}

    //////////////////////////////////////////////////////////////////////////
    //CCustomThreadImp

    class CCustomThreadImp
    {
    public:

        CCustomThreadImp(IThreadProcHandler* pIThreadProcHandler);

        virtual ~CCustomThreadImp();

        //功能:启动线程
        CCustomThread::ERROR_CODE Start();

        //功能:停止线程
        //参数:
        //      UINT32 nWaitTime IN 等待多时秒后中止线程
        //      UINT32 nExitCode IN 线程退出代码
        void Stop(UINT32 nWaitTime = SYSBASE_STOP_WAIT_TIME, UINT32 nExitCode = 0);

        //功能:获取线程ID
        //返回值: 线程ID
        UINT32 GetThreadID();

    private:

        static DWORD WINAPI ThreadProc(LPVOID lpParam);

        DWORD m_dwThreadId;

        HANDLE m_hThread;

        IThreadProcHandler* m_pIThreadProcHandler;
    };

    CCustomThreadImp::CCustomThreadImp(IThreadProcHandler* pIThreadProcHandler)
    {
        m_pIThreadProcHandler = pIThreadProcHandler;

        m_hThread = NULL;

        m_dwThreadId = NULL;
    }

    CCustomThreadImp::~CCustomThreadImp()
    {
        this->Stop(SYSBASE_STOP_WAIT_TIME, 0);
    }

    CCustomThread::ERROR_CODE CCustomThreadImp::Start()
    {
        if (m_hThread)
        {
            return CCustomThread::ERROR_CODE_HAS_INIT;
        }

        m_hThread = CreateThread(NULL, 0, CCustomThreadImp::ThreadProc, this, 0, &m_dwThreadId);

        if (!m_hThread)
        {
            return CCustomThread::ERROR_CODE_SYSTEM;
        }

        return CCustomThread::ERROR_CODE_SUCCESS;
    }

    void CCustomThreadImp::Stop(UINT32 nWaitTime, UINT32 nExitCode)
    {
        if (!m_hThread)
        {
            return;
        }

        if (WaitForSingleObject(m_hThread, nWaitTime) == WAIT_TIMEOUT)
        {
            TerminateThread(m_hThread, nExitCode);
        }

        CloseHandle(m_hThread);

        m_hThread = NULL;

        m_dwThreadId = 0;

        m_pIThreadProcHandler = NULL;
    }

    UINT32 CCustomThreadImp::GetThreadID()
    {
        return m_dwThreadId;
    }

    DWORD WINAPI CCustomThreadImp::ThreadProc(LPVOID lpParam)
    {
        CCustomThreadImp* pCCustomThread = (CCustomThreadImp*)lpParam;

        pCCustomThread->m_pIThreadProcHandler->Proc();

        return 0;
    }

	//////////////////////////////////////////////////////////////////////////
	//CCustomThread

	CCustomThread::CCustomThread()
	{
        m_pImp = new(std::nothrow) CCustomThreadImp(this);
	}

	CCustomThread::~CCustomThread()
	{
		if (!m_pImp)
        {
            return;
        }

        CCustomThreadImp* pCCustomThreadImp = (CCustomThreadImp*)m_pImp;

        delete pCCustomThreadImp;

        m_pImp = NULL;
	}

	CCustomThread::ERROR_CODE CCustomThread::Start()
	{
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CCustomThreadImp* pCCustomThreadImp = (CCustomThreadImp*)m_pImp;

        return pCCustomThreadImp->Start();
	}

	void CCustomThread::Stop(UINT32 nWaitTime, UINT32 nExitCode)
	{
        if (!m_pImp)
        {
            return;
        }

        CCustomThreadImp* pCCustomThreadImp = (CCustomThreadImp*)m_pImp;

        pCCustomThreadImp->Stop(nWaitTime, nExitCode);
	}

	UINT32 CCustomThread::GetThreadID()
	{
        if (!m_pImp)
        {
            return 0;
        }

        CCustomThreadImp* pCCustomThreadImp = (CCustomThreadImp*)m_pImp;

        return pCCustomThreadImp->GetThreadID();
	}

    //////////////////////////////////////////////////////////////////////////

    CTimeThread::CTimeThread()
    {
        m_nWaitTime = 1000;
        m_bRunFirst = false;
        m_bStop = true;
    }

    CTimeThread::~CTimeThread()
    {
        this->Stop();
    }

    bool CTimeThread::Start(UINT32 nWaitTime, bool bRunFirst)
    {
        if (!m_bStop)
        {
            return true;
        }

        m_nWaitTime = nWaitTime;
        m_bRunFirst = bRunFirst;

        if (CCustomThread::ERROR_CODE_SUCCESS != CCustomThread::Start())
        {
            return false;
        }

        return true;
    }

    void CTimeThread::Stop(UINT32 nWaitTime, UINT32 nExitCode)
    {
        if (m_bStop)
        {
            return;
        }

        m_bStop = true;

        m_event.Set();

        CCustomThread::Stop(nWaitTime, nExitCode);
    }

    void CTimeThread::SetWaitTime(UINT32 nWaitTime)
    {
        m_nWaitTime = nWaitTime;
    }

    UINT32 CTimeThread::GetThreadID()
    {
        return CCustomThread::GetThreadID();
    }

    void CTimeThread::Proc()
    {
        m_bStop = false;
        m_event.Reset();

        if (m_bRunFirst)
        {
            this->ProcOnTime();
        }

        while (!m_event.Wait(m_nWaitTime))
        {
            this->ProcOnTime();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //CCustomMultiThreadImp

    class CCustomMultiThreadImp
    {
    public:

        CCustomMultiThreadImp(IThreadProcHandler* pIThreadProcHandler);

        virtual ~CCustomMultiThreadImp();

        //功能:启动创建线程池
        //参数:
        //      unsigned short sThreadCount IN 线程个数
        //异常
        virtual CCustomMultiThread::ERROR_CODE Start(unsigned short sThreadCount);

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

    private:

        static DWORD WINAPI ThreadProc(LPVOID lpParam);

        bool m_bStop;   //停止标志

        list<HANDLE> m_ThreadHandleList;

        vector<DWORD> m_ThreadIDs;

        IThreadProcHandler* m_pIThreadProcHandler;
    };

    CCustomMultiThreadImp::CCustomMultiThreadImp(IThreadProcHandler* pIThreadProcHandler)
    {
        m_pIThreadProcHandler = pIThreadProcHandler;

        m_bStop = TRUE;
    }

    CCustomMultiThreadImp::~CCustomMultiThreadImp()
    {
        this->Stop(SYSBASE_STOP_WAIT_TIME);
    }

    CCustomMultiThread::ERROR_CODE CCustomMultiThreadImp::Start(unsigned short sThreadCount)
    {
        if (!m_bStop)
        {
            return CCustomMultiThread::ERROR_CODE_HAS_INIT;
        }

        if (0 == sThreadCount)
        {
            return CCustomMultiThread::ERROR_CODE_PARAM;
        }

        UINT uIndex = 0;
        DWORD dwThreadID = 0;

        m_ThreadIDs.clear();
        m_ThreadIDs.resize(sThreadCount);

        for (; uIndex < sThreadCount; ++uIndex)
        {
            HANDLE hThread = CreateThread(NULL, 0, CCustomMultiThreadImp::ThreadProc, this, CREATE_SUSPENDED, &dwThreadID);

            if (!hThread)
            {
                break;
            }

            m_ThreadHandleList.push_back(hThread);

            m_ThreadIDs[uIndex] = dwThreadID;
        }

        //当创建的线程数不符合sThreadCount时为异常情况
        if (m_ThreadHandleList.size() == sThreadCount)
        {
            list<HANDLE>::iterator entity = m_ThreadHandleList.begin();
            list<HANDLE>::iterator endEntity = m_ThreadHandleList.end();

            for(; entity != endEntity; ++entity)
            {
                ResumeThread(*entity);
            }
        }
        else
        {
            list<HANDLE>::iterator entity = m_ThreadHandleList.begin();
            list<HANDLE>::iterator endEntity = m_ThreadHandleList.end();       

            for(; entity != endEntity; ++entity)
            {
                TerminateThread(*entity, 0);

                CloseHandle(*entity);
            }

            m_ThreadHandleList.clear();

            m_ThreadIDs.clear();

            return CCustomMultiThread::ERROR_CODE_SYSTEM;
        }

        m_bStop = false;

        return CCustomMultiThread::ERROR_CODE_SUCCESS;
    }

    void CCustomMultiThreadImp::Stop(UINT32 nWaitTime)
    {
        if (m_bStop)
        {
            return;
        }

        m_bStop = TRUE;

        vector<HANDLE> threadHandlerArray;
        threadHandlerArray.resize(m_ThreadHandleList.size());
        
        {
            list<HANDLE>::iterator entity = m_ThreadHandleList.begin();
            list<HANDLE>::iterator endEntity = m_ThreadHandleList.end();       

            int nIndex = 0;
            for(; entity != endEntity; ++entity)
            {
                threadHandlerArray[nIndex] = *entity;
                nIndex++;
            }
        }

        WaitForMultipleObjects((DWORD)threadHandlerArray.size(), &threadHandlerArray.front(), TRUE, nWaitTime);

        list<HANDLE>::iterator entity = m_ThreadHandleList.begin();
        list<HANDLE>::iterator endEntity = m_ThreadHandleList.end();       

        for(; entity != endEntity; ++entity)
        {
            TerminateThread(*entity, 0);

            CloseHandle(*entity);
        }

        m_ThreadHandleList.clear();

        m_ThreadIDs.clear();

        m_pIThreadProcHandler = NULL;
    }

    UINT32* CCustomMultiThreadImp::GetThreadIDs(unsigned short* psThreadCount)
    {
        if (!psThreadCount)
        {
            return NULL;
        }

        if (0 == m_ThreadIDs.size())
        {
            *psThreadCount = 0;

            return NULL;
        }

        *psThreadCount = (WORD)m_ThreadIDs.size();

        return (UINT32*)&m_ThreadIDs.front();
    }

    DWORD WINAPI CCustomMultiThreadImp::ThreadProc(LPVOID lpParam)
    {
        CCustomMultiThreadImp* pCCustomMultiThreadImp = (CCustomMultiThreadImp*)lpParam;

        pCCustomMultiThreadImp->m_pIThreadProcHandler->Proc();

        return 0;
    }

	//////////////////////////////////////////////////////////////////////////
	//CCustomMultiThread

	CCustomMultiThread::CCustomMultiThread()
	{
        m_pImp = new(std::nothrow) CCustomMultiThreadImp(this);
	}

	CCustomMultiThread::~CCustomMultiThread()
	{
		if (!m_pImp)
        {
            return;
        }

        CCustomMultiThreadImp* pCCustomMultiThreadImp = (CCustomMultiThreadImp*)m_pImp;

        delete pCCustomMultiThreadImp;

        m_pImp = NULL;
	}

    CCustomMultiThread::ERROR_CODE CCustomMultiThread::Start(unsigned short sThreadCount)
	{
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CCustomMultiThreadImp* pCCustomMultiThreadImp = (CCustomMultiThreadImp*)m_pImp;

        return pCCustomMultiThreadImp->Start(sThreadCount);
	}

	void CCustomMultiThread::Stop(UINT32 nWaitTime)
	{
        if (!m_pImp)
        {
            return;
        }

        CCustomMultiThreadImp* pCCustomMultiThreadImp = (CCustomMultiThreadImp*)m_pImp;

        return pCCustomMultiThreadImp->Stop(nWaitTime);
	}

	UINT32* CCustomMultiThread::GetThreadIDs(unsigned short* psThreadCount)
	{
        if (!m_pImp)
        {
            return NULL;
        }

        CCustomMultiThreadImp* pCCustomMultiThreadImp = (CCustomMultiThreadImp*)m_pImp;

        return pCCustomMultiThreadImp->GetThreadIDs(psThreadCount);
	}

	///////////////////////////////////////////////ITaskHandlerFactory///////////////////////////
	//ITaskHandler

	ITaskHandler::ITaskHandler()
	{

	}

	ITaskHandler::~ITaskHandler()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//ITaskFactory

	ITaskHandlerFactory::ITaskHandlerFactory()
	{

	}

	ITaskHandlerFactory::~ITaskHandlerFactory()
	{

	}

    //////////////////////////////////////////////////////////////////////////
    //CTaskMultiThreadImp

    class CTaskMultiThreadImp
    {
    public:

        //功能:初始化
        CTaskMultiThreadImp();

        virtual ~CTaskMultiThreadImp();

        //功能:启动创建线程池
        //参数:
        //	    ITaskHandlerFactory *pITaskHandlerFactory IN 任务工厂对象
        //      unsigned short sThreadCount IN 线程个数
        //      const void* lpParam IN 线程函数的参数
        //异常
        CTaskMultiThread::ERROR_CODE Start(ITaskHandlerFactory *pITaskHandlerFactory, unsigned short sThreadCount, void* lpParam = NULL);

        //功能:等待多少毫秒后结束停止并且关闭所有线程
        //参数:
        //      UINT32 nWaitTime IN 多少毫秒,如果为 0 时马上结束所有线程
        void Stop(UINT32 nWaitTime = SYSBASE_STOP_WAIT_TIME);

        //功能:获取线程ID数组
        //参数:
        //      vector<UINT32> *pThreadIDArray OUT 线程ID数组
        void GetThreadIDArray(vector<UINT32>* pThreadIDArray);

    private:

        static DWORD WINAPI Proc(LPVOID lpParam);

        LPVOID m_lpParameter;//线程的参数

        bool m_bStop;   //停止标志

        ITaskHandlerFactory* m_pITaskHandlerFactory;

        list<HANDLE> m_ThreadHandleList;

        typedef map<DWORD, ITaskHandler*> ThreadTaskMap;

        ThreadTaskMap m_ThreadTaskMap;
    };

    CTaskMultiThreadImp::CTaskMultiThreadImp()
    {
        m_bStop = true;

        m_pITaskHandlerFactory = NULL;

        m_lpParameter = NULL;
    }

    CTaskMultiThreadImp::~CTaskMultiThreadImp()
    {
        this->Stop(3000);
    }

    CTaskMultiThread::ERROR_CODE CTaskMultiThreadImp::Start(ITaskHandlerFactory* pITaskHandlerFactory, unsigned short sThreadCount, void* lpParam)
    {
        if (!m_bStop)
        {
            return CTaskMultiThread::ERROR_CODE_HAS_INIT;
        }

        if (!pITaskHandlerFactory)
        {
            return CTaskMultiThread::ERROR_CODE_PARAM;
        }

        if (0 == sThreadCount)
        {
            return CTaskMultiThread::ERROR_CODE_PARAM;
        }

        m_bStop = false;

        m_lpParameter = lpParam;

        m_pITaskHandlerFactory = pITaskHandlerFactory;

        UINT uIndex = 0;

        DWORD dwThreadID = 0;

        for (; uIndex < sThreadCount; ++uIndex)
        {
            HANDLE hThread = CreateThread(NULL, 0, CTaskMultiThreadImp::Proc, 
                this, CREATE_SUSPENDED, &dwThreadID);

            if (!hThread)
            {
                break;
            }

            ITaskHandler* pITaskHandler = m_pITaskHandlerFactory->Create();

            if (pITaskHandler)
            {
                m_ThreadTaskMap.insert(ThreadTaskMap::value_type(dwThreadID, pITaskHandler));
            }
            else
            {
                break;;
            }

            m_ThreadHandleList.push_back(hThread);
        }

        list<HANDLE>::iterator entity = m_ThreadHandleList.begin();
        list<HANDLE>::iterator endEntity = m_ThreadHandleList.end();       

        if (m_ThreadHandleList.size() == sThreadCount)
        {
            for(; entity != endEntity; ++entity)
            {
                ResumeThread(*entity);
            }
        }
        else    //当创建的线程数不符合sThreadCount时为异常情况
        {
            this->Stop(3000);

            return CTaskMultiThread::ERROR_CODE_SYSTEM;
        }

        return CTaskMultiThread::ERROR_CODE_SUCCESS;
    }

    void CTaskMultiThreadImp::Stop(UINT32 nWaitTime)
    {
        if (m_bStop)
        {
            return;
        }

        m_bStop = true;

        {
            ThreadTaskMap::iterator entity = m_ThreadTaskMap.begin();
            ThreadTaskMap::iterator endEntity = m_ThreadTaskMap.end();       

            for(; entity != endEntity; ++entity)
            {
                TTaskInfo aTTaskInfo = {0};
                aTTaskInfo.nThreadID = entity->first;

                entity->second->Stop(&aTTaskInfo);
            }
        }

        if (m_ThreadHandleList.size() > 0)
        {
            vector<HANDLE> threadHandlerArray;
            threadHandlerArray.resize(m_ThreadHandleList.size());

            {
                list<HANDLE>::iterator entity = m_ThreadHandleList.begin();
                list<HANDLE>::iterator endEntity = m_ThreadHandleList.end();       

                int nIndex = 0;
                for(; entity != endEntity; ++entity)
                {
                    threadHandlerArray[nIndex] = *entity;
                    nIndex++;
                }
            }

            WaitForMultipleObjects((DWORD)threadHandlerArray.size(), &threadHandlerArray.front(), TRUE, nWaitTime);

            list<HANDLE>::iterator entity = m_ThreadHandleList.begin();
            list<HANDLE>::iterator endEntity = m_ThreadHandleList.end();       

            for(; entity != endEntity; ++entity)
            {
                TerminateThread(*entity, 0);

                CloseHandle(*entity);
            }
        }

        m_ThreadHandleList.clear();

        {
            ThreadTaskMap::iterator entity = m_ThreadTaskMap.begin();
            ThreadTaskMap::iterator endEntity = m_ThreadTaskMap.end();       

            for(; entity != endEntity; ++entity)
            {
                if (entity->second)
                {
                    delete entity->second;
                    entity->second = NULL;
                }
            }
        }

        m_ThreadTaskMap.clear();
    }

    void CTaskMultiThreadImp::GetThreadIDArray(vector<UINT32>* pThreadIDArray)
    {
        if (pThreadIDArray)
        {
            pThreadIDArray->reserve(m_ThreadTaskMap.size());

            ThreadTaskMap::iterator entity = m_ThreadTaskMap.begin();
            ThreadTaskMap::iterator endEntity = m_ThreadTaskMap.end();       

            for(; entity != endEntity; ++entity)
            {
                pThreadIDArray->push_back(entity->first);
            }
        }
    }

    DWORD WINAPI CTaskMultiThreadImp::Proc(LPVOID lpParam)
    {
        CTaskMultiThreadImp* pCTaskMultiThreadImp = (CTaskMultiThreadImp*)lpParam;

        ThreadTaskMap::iterator entity = pCTaskMultiThreadImp->m_ThreadTaskMap.find(GetCurrentThreadId());
        ThreadTaskMap::iterator endEntity = pCTaskMultiThreadImp->m_ThreadTaskMap.end();

        if (entity != endEntity)
        {
            entity->second->Proc(pCTaskMultiThreadImp->m_lpParameter);
        }

        return 0;
    }

	//////////////////////////////////////////////////////////////////////////
	//CTaskMultiThread

	CTaskMultiThread::CTaskMultiThread()
	{
        m_pImp = new(std::nothrow) CTaskMultiThreadImp();
	}

	CTaskMultiThread::~CTaskMultiThread()
	{
		if (!m_pImp)
        {
            return;
        }

        CTaskMultiThreadImp* pCTaskMultiThreadImp = (CTaskMultiThreadImp*)m_pImp;

        delete pCTaskMultiThreadImp;

        m_pImp = NULL;
	}

	CTaskMultiThread::ERROR_CODE CTaskMultiThread::Start(ITaskHandlerFactory* pITaskHandlerFactory, unsigned short sThreadCount, void* lpParam)
	{
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CTaskMultiThreadImp* pCTaskMultiThreadImp = (CTaskMultiThreadImp*)m_pImp;

        return pCTaskMultiThreadImp->Start(pITaskHandlerFactory, sThreadCount, lpParam);
	}

	void CTaskMultiThread::Stop(UINT32 nWaitTime)
	{
        if (!m_pImp)
        {
            return;
        }

        CTaskMultiThreadImp* pCTaskMultiThreadImp = (CTaskMultiThreadImp*)m_pImp;

        pCTaskMultiThreadImp->Stop(nWaitTime);
	}

	void CTaskMultiThread::GetThreadIDArray(vector<UINT32>* pThreadIDArray)
	{
        if (!m_pImp)
        {
            return;
        }

        CTaskMultiThreadImp* pCTaskMultiThreadImp = (CTaskMultiThreadImp*)m_pImp;

        pCTaskMultiThreadImp->GetThreadIDArray(pThreadIDArray);
	}
}
