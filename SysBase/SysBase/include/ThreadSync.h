#pragma once

#include "SysBase.h"

using namespace std;

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //线程互斥

    class CThreadMutex : public SysBase::CObject
    {
    public:

        CThreadMutex();

        virtual ~CThreadMutex();

        //功能:锁定
        virtual void Lock();

        //功能:解锁
        virtual void Unlock();

    private:

        void* m_pImp;
    };

    //////////////////////////////////////////////////////////////////////////
    //CSmartMutex 智能互斥

    class CSmartMutex
    {
    public:

        CSmartMutex(CThreadMutex &aCThreadMutex);

        ~CSmartMutex();

    private:

        CThreadMutex* m_pCThreadMutex;

    };

    //快速创建一个智能互斥对象
#define CREATE_SMART_MUTEX(aCThreadMutex) CSmartMutex CREATE_UNIQUE_VAR_NAME(_aSmartMutex)(aCThreadMutex);

#define CREATE_SMART_MUTEX_PTR(pCThreadMutex) CSmartMutex CREATE_UNIQUE_VAR_NAME(_aSmartMutex)(*pCThreadMutex);

    //////////////////////////////////////////////////////////////////////////
    //同步对象基类

    class CSyncObject
    {
    public:

        CSyncObject();

        virtual ~CSyncObject();

        virtual	bool Wait(UINT32 nWaitTime) = 0;

        virtual void* GetHandle() = 0;

        static bool WaitSyncObjects(
            CSyncObject** pCSyncObjectArray, 
            UINT32 nCount, 
            bool bWaitAll, 
            UINT32 nMilliseconds,
            UINT32& nObjectIndex);
    
    };

    //////////////////////////////////////////////////////////////////////////
    //进程互斥

    class CProcessMutex : public CSyncObject
    {
    public:

        CProcessMutex();

        virtual ~CProcessMutex();

        //功能:构造函数
        //参数:
        //      const char* pName IN 名称
        bool Open(const char* pName = NULL);

        void Release();

        virtual void* GetHandle();

        virtual	bool Wait(UINT32 nWaitTime);

    private:

        void* m_pImp;
    };

    //////////////////////////////////////////////////////////////////////////
    //CEvent 事件

    class CEvent : public CSyncObject
    {
    public:

        //功能:构造函数
        //参数:
        //      bool bManualReset IN true 时需要 Reset() 函数重置状态，false 时当线程释放后自动置为初始状态
        //      bool bInitialState IN true 时为创建就初始化为等待状态，false 时为初始状态
        //      const char* lpName IN 名称
        CEvent(  
            bool bManualReset = true,
            bool bInitialState = false,
            const char* lpName = NULL);

        virtual ~CEvent();

        //功能:重置事件信号为初始状态
        void Reset();

        //功能:设置事件型号为等待状态
        void Set();

        virtual	bool Wait(UINT32 nWaitTime);

        virtual void* GetHandle();

    private:

        void* m_pImp;
    };

    class CAutoEvent : public CEvent
    {
    public:

        CAutoEvent(
            bool bInitialState = false,
            const char* lpName = NULL);

        virtual ~CAutoEvent();

    };

    //////////////////////////////////////////////////////////////////////////
    //CSemaphore 信号量

    class CSemaphore : public CSyncObject
    {
    public:

        //功能:构造函数
        //参数:
        //      int nInitialCount IN 初始化数量
        //      int nlMaximumCount IN 最大值（最大容量）
        //       const char* pName IN 名称
        CSemaphore(
            int nInitialCount = 0,
            int nlMaximumCount = 2147483647, 
            const char* pName = NULL);

        //功能:析构函数
        virtual ~CSemaphore();

        //功能:增加信号量计数
        //参数:
        //      int nAddCount IN 增加的计数量
        void Add(int nAddCount = 1);

        virtual	bool Wait(UINT32 nWaitTime);

        virtual void* GetHandle();

    private:

        void* m_pImp;
    };
}
