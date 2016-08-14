#include "../SysBase_Interface.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CThreadMutexImp

    class CThreadMutexImp
    {
    public:

        CThreadMutexImp();

        virtual ~CThreadMutexImp();

        //功能:锁定
        virtual void Lock();

        //功能:解锁
        virtual void Unlock();

    private:

        CRITICAL_SECTION m_CriticalSection;
    };

    CThreadMutexImp::CThreadMutexImp()
    {
        InitializeCriticalSection(&m_CriticalSection);
    }

    CThreadMutexImp::~CThreadMutexImp()
    {
        DeleteCriticalSection(&m_CriticalSection);
    }

    void CThreadMutexImp::Lock()
    {
        EnterCriticalSection(&m_CriticalSection);
    }

    void CThreadMutexImp::Unlock()
    {
        LeaveCriticalSection(&m_CriticalSection);
    }


    //////////////////////////////////////////////////////////////////////////
    //CThreadMutex

    CThreadMutex::CThreadMutex()
    {
        m_pImp = new(std::nothrow) CThreadMutexImp();
    }

    CThreadMutex::~CThreadMutex()
    {
        if (m_pImp)
        {
            CThreadMutexImp* pCThreadMutexImp = (CThreadMutexImp*)m_pImp;

            delete pCThreadMutexImp;

            m_pImp = NULL;
        }
    }

    void CThreadMutex::Lock()
    {
        if (m_pImp)
        {
            CThreadMutexImp* pCThreadMutexImp = (CThreadMutexImp*)m_pImp;

            pCThreadMutexImp->Lock();
        }
    }

    void CThreadMutex::Unlock()
    {
        if (m_pImp)
        {
            CThreadMutexImp* pCThreadMutexImp = (CThreadMutexImp*)m_pImp;

            pCThreadMutexImp->Unlock();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //CSmartMutex

    CSmartMutex::CSmartMutex(CThreadMutex &aCThreadMutex)
    {
        m_pCThreadMutex = &aCThreadMutex;

        m_pCThreadMutex->Lock();
    }

    CSmartMutex::~CSmartMutex()
    {
        m_pCThreadMutex->Unlock();
    }

    //////////////////////////////////////////////////////////////////////////
    //CProcessMutexImp


    class CProcessMutexImp
    {
    public:

        CProcessMutexImp();

        virtual ~CProcessMutexImp();

        //功能:构造函数
        //参数:
        //      const char* pName IN 名称
        bool Open(const char* pName = NULL);

        void Release();

        void* GetHandle();

        bool Wait(UINT32 nWaitTime);

    private:

        HANDLE m_hSyncHandle;
    };

    CProcessMutexImp::CProcessMutexImp()
    {
        m_hSyncHandle = NULL;
    }

    CProcessMutexImp::~CProcessMutexImp()
    {
        this->Release();
    }

    bool CProcessMutexImp::Open(const char* pName)
    {
        m_hSyncHandle = CreateMutexA(NULL, false, pName);

        if (!m_hSyncHandle)
        {
            return false;
        }

        DWORD dwError = GetLastError();

        if (dwError == ERROR_ALREADY_EXISTS)	
        {
            this->Release();

            return false;
        }

        return true;
    }

    void CProcessMutexImp::Release()
    {
        if (m_hSyncHandle)
        {
            ReleaseMutex(m_hSyncHandle);
            CloseHandle(m_hSyncHandle);

            m_hSyncHandle = NULL;
        }
    }

    void* CProcessMutexImp::GetHandle()
    {
        return m_hSyncHandle;
    }

    bool CProcessMutexImp::Wait(UINT32 nWaitTime)
    {
        if (!m_hSyncHandle)
        {
            return false;
        }

        DWORD dwWaitResult = WaitForSingleObject(m_hSyncHandle, nWaitTime);

        switch (dwWaitResult)
        {
            case WAIT_OBJECT_0 :    //获取到权限
                {
                    return true;
                }
                break;
            case WAIT_TIMEOUT :     //另一个进程或线程正占有权限
                {
                    return false;
                }
                break;
            case WAIT_FAILED:
                {
                    return false;
                }
                break;
            default:
                {
                    return false;
                }
        }

        return false;
    }

	//////////////////////////////////////////////////////////////////////////
	//CProcessMutex

	CProcessMutex::CProcessMutex()
	{
		m_pImp = new(std::nothrow) CProcessMutexImp();
	}

	CProcessMutex::~CProcessMutex()
	{
		if (m_pImp)
        {
            CProcessMutexImp* pCProcessMutexImp = (CProcessMutexImp*)m_pImp;

            delete pCProcessMutexImp;

            m_pImp = NULL;
        }
	}

	bool CProcessMutex::Open(const char* pName)
	{
		if (!m_pImp)
        {
            return false;
        }

        CProcessMutexImp* pCProcessMutexImp = (CProcessMutexImp*)m_pImp;

        return pCProcessMutexImp->Open(pName);
	}

	void CProcessMutex::Release()
	{
        if (!m_pImp)
        {
            return;
        }

        CProcessMutexImp* pCProcessMutexImp = (CProcessMutexImp*)m_pImp;

        pCProcessMutexImp->Release();
	}

    void* CProcessMutex::GetHandle()
    {
        if (!m_pImp)
        {
            return NULL;
        }

        CProcessMutexImp* pCProcessMutexImp = (CProcessMutexImp*)m_pImp;

        return pCProcessMutexImp->GetHandle();
    }

    bool CProcessMutex::Wait(UINT32 nWaitTime)
    {
        if (!m_pImp)
        {
            return false;
        }

        CProcessMutexImp* pCProcessMutexImp = (CProcessMutexImp*)m_pImp;

        return pCProcessMutexImp->Wait(nWaitTime);
    }

    //////////////////////////////////////////////////////////////////////////
    //CSyncObject
    
    CSyncObject::CSyncObject()
    {
    }
    
    CSyncObject::~CSyncObject()
    {
    }

    bool CSyncObject::WaitSyncObjects(
        CSyncObject** pCSyncObjectArray, 
        UINT32 nCount, 
        bool bWaitAll, 
        UINT32 nMilliseconds,
        UINT32& nObjectIndex)
    {
        if (!pCSyncObjectArray
            || 0 == nCount
            || MAXIMUM_WAIT_OBJECTS < nCount)
        {
            throw;
        }

        DWORD dwIndex = 0;

        vector<HANDLE> handleArray;	

        handleArray.resize(nCount);

        for (dwIndex = 0; dwIndex < nCount; ++dwIndex)
        {
            handleArray[dwIndex] = (HANDLE)pCSyncObjectArray[dwIndex]->GetHandle();
        }

        dwIndex = WaitForMultipleObjects(nCount, handleArray.data(), bWaitAll, nMilliseconds) - WAIT_OBJECT_0;

        switch (dwIndex)
        {
        case WAIT_TIMEOUT:
            {
                return false;
            }
            break;
        case WAIT_FAILED:
            {
                return false;
            }
        }

        nObjectIndex = dwIndex;

        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    //CEventImp

    class CEventImp
    {
    public:

        CEventImp(  
            bool bManualReset = true,
            bool bInitialState = false,
            const char* pName = NULL);

        virtual ~CEventImp();

        friend class CSyncObject;

        void Reset();

        void Set();

        bool Wait(UINT32 nWaitTime);

        void* GetHandle();

    private:

        HANDLE m_hSyncHandle;
    };

    CEventImp::CEventImp(  
        bool bManualReset,
        bool bInitialState,
        const char* pName)
    {
        m_hSyncHandle = CreateEventA(NULL, bManualReset, bInitialState, pName);
    }

    CEventImp::~CEventImp()
    {
        if(m_hSyncHandle)
        {
            CloseHandle(m_hSyncHandle);

            m_hSyncHandle = NULL;
        }
    }

    void CEventImp::Reset()
    {
        if (m_hSyncHandle)
        {
            ResetEvent(m_hSyncHandle);
        }
    }

    void CEventImp::Set()
    {
        if (m_hSyncHandle)
        {
            SetEvent(m_hSyncHandle);
        }
    }

    bool CEventImp::Wait(UINT32 nWaitTime)
    {
        DWORD dwWaitResult = WaitForSingleObject(m_hSyncHandle, nWaitTime);

        switch (dwWaitResult)
        {
        case WAIT_OBJECT_0 :    //获取到权限
            {
                return true;
            }
            break;
        case WAIT_TIMEOUT :     //另一个进程或线程正占有权限
            {
                return false;
            }
            break;
        case WAIT_FAILED:
            {
                return false;
            }
            break;
        default:
            {
                return false;
            }
        }

        return false;
    }

    void* CEventImp::GetHandle()
    {
        return m_hSyncHandle;
    }

    //////////////////////////////////////////////////////////////////////////
    //CEvent 事件

    CEvent::CEvent(  
        bool bManualReset,
        bool bInitialState,
        const char* pName)
    {
        m_pImp = new(std::nothrow) CEventImp(bManualReset, bInitialState, pName);
    }

    CEvent::~CEvent()
    {
        if(m_pImp)
        {
            CEventImp* pCEventImp = (CEventImp*)m_pImp;

            delete pCEventImp;

            m_pImp = NULL;
        }
    }

    void CEvent::Reset()
    {
		if (!m_pImp)
		{
			return;
		}

        CEventImp* pCEventImp = (CEventImp*)m_pImp;

        pCEventImp->Reset();
    }

    void CEvent::Set()
    {
        if (!m_pImp)
        {
            return;
        }

        CEventImp* pCEventImp = (CEventImp*)m_pImp;

        pCEventImp->Set();
    }

    bool CEvent::Wait(UINT32 nWaitTime)
    {
        if (!m_pImp)
        {
            return false;
        }

        CEventImp* pCEventImp = (CEventImp*)m_pImp;

        return pCEventImp->Wait(nWaitTime);
    }

    void* CEvent::GetHandle()
    {
        if (!m_pImp)
        {
            return NULL;
        }

        CEventImp* pCEventImp = (CEventImp*)m_pImp;

        return pCEventImp->GetHandle();
    }

    //////////////////////////////////////////////////////////////////////////

    CAutoEvent::CAutoEvent(bool bInitialState,const char* lpName) : CEvent(false, bInitialState, lpName)
    {

    }

    CAutoEvent::~CAutoEvent()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //CSemaphoreImp

    class CSemaphoreImp
    {
    public:

        //功能:构造函数
        //参数:
        //      int nInitialCount IN 初始化数量
        //      int nlMaximumCount IN 最大值（最大容量）
        //       const char* pName IN 名称
        CSemaphoreImp(
            int nInitialCount = 0,
            int nlMaximumCount = 2147483647, 
            const char* pName = NULL);

        //功能:析构函数
        virtual ~CSemaphoreImp();

        //功能:增加信号量计数
        //参数:
        //      int nAddCount IN 增加的计数量
		void Add(int nAddCount = 1);

        bool Wait(UINT32 nWaitTime);

        void* GetHandle();

    private:

        HANDLE m_hSyncHandle;
    };


    CSemaphoreImp::CSemaphoreImp(
        int nInitialCount,
        int nlMaximumCount, 
        const char* pName)
    {
        m_hSyncHandle = CreateSemaphoreA(NULL, nInitialCount, nlMaximumCount, pName);
    }

    CSemaphoreImp::~CSemaphoreImp()
    {
        if (m_hSyncHandle)
        {
            CloseHandle(m_hSyncHandle);

            m_hSyncHandle = NULL;
        }
    }

    void CSemaphoreImp::Add(int nAddCount)
    {
        if (m_hSyncHandle)
        {
            LONG lPreviousCount = 0;

            ReleaseSemaphore(m_hSyncHandle, nAddCount, &lPreviousCount);
        }
    }

    bool CSemaphoreImp::Wait(UINT32 nWaitTime)
    {
        DWORD dwWaitResult = WaitForSingleObject(m_hSyncHandle, nWaitTime);

        switch (dwWaitResult)
        {
        case WAIT_OBJECT_0 :    //获取到权限
            {
                return true;
            }
            break;
        case WAIT_TIMEOUT :     //另一个进程或线程正占有权限
            {
                return false;
            }
            break;
        case WAIT_FAILED:
            {
                return false;
            }
            break;
        default:
            {
                return false;
            }
        }

        return false;
    }

    void* CSemaphoreImp::GetHandle()
    {
        return m_hSyncHandle;
    }

    //////////////////////////////////////////////////////////////////////////
    //CSemaphore 信号量

    CSemaphore::CSemaphore(
        int nInitialCount,
        int nlMaximumCount, 
        const char* pName)
    {
        m_pImp = new(std::nothrow) CSemaphoreImp(nInitialCount, nlMaximumCount, pName);
    }

    CSemaphore::~CSemaphore()
    {
        if (m_pImp)
        {
            CSemaphoreImp* pCSemaphoreImp = (CSemaphoreImp*)m_pImp;

            delete pCSemaphoreImp;

            m_pImp = NULL;
        }
    }

    void CSemaphore::Add(int nAddCount)
    {
		if (!m_pImp)
        {
            return;
        }

        CSemaphoreImp* pCSemaphoreImp = (CSemaphoreImp*)m_pImp;

        pCSemaphoreImp->Add(nAddCount);
    }

    bool CSemaphore::Wait(UINT32 nWaitTime)
    {
        if (!m_pImp)
        {
            return false;
        }

        CSemaphoreImp* pCSemaphoreImp = (CSemaphoreImp*)m_pImp;

        return pCSemaphoreImp->Wait(nWaitTime);
    }

    void* CSemaphore::GetHandle()
    {
        if (!m_pImp)
        {
            return NULL;
        }

        CSemaphoreImp* pCSemaphoreImp = (CSemaphoreImp*)m_pImp;

        return pCSemaphoreImp->GetHandle();
    }
}