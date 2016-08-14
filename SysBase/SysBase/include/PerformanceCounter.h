#pragma once

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CPerformanceCounter 性能计数器

    class CPerformanceCounter : public SysBase::CObject
    {
    public:

        CPerformanceCounter();

        virtual ~CPerformanceCounter();

        bool Init(const char* szMachineName);

        void Close();

        bool Add(const char* szFullCounterPath);

        void Remove(const char* szFullCounterPath);

        bool GetValue(const char* szFullCounterPath, double& dbValue);

        bool GetValue(map<string, double>& counterValueMap);

        void CollectData();

        //////////////////////////////////////////////////////////////////////////

        static bool IsCounterPathValid(const char* szFullCounterPath);

        static UINT32 BuildCounterPath(
        const char* szObjectName, 
        const char* szInstanceName, 
        const char* szObjectItemName,
        char* szCounterPathBufferOut, 
        UINT32 nBufferSize);

        static bool GetCounterObjectList(list<string>& counterObjectListOut, const char* szMachineName = NULL);

        static bool GetCounterObjectItemList(
            list<string>& counterObjectItemListOut, 
            list<string>& counterInstanceListOut, 
            const char* szObjectName, 
            const char* szMachineName = NULL);

        static bool GetCounterObjectItemList(
            list<string>& counterObjectItemListOut, 
            list<string>& counterInstanceListOut, 
            const string& szObjectName,
            const char* szMachineName = NULL);

    private:

        void* m_pImp;
    };

    //////////////////////////////////////////////////////////////////////////
    //CPerformanceCounterMonitor

    class CPerformanceCounterMonitor : public SysBase::IThreadProcHandler, SysBase::CObject
    {
    public:

        static CPerformanceCounterMonitor& Instance();

        CPerformanceCounterMonitor();

        virtual ~CPerformanceCounterMonitor();

        bool Start(UINT32 nIntervalTime = 1000, const char* szMachineName = NULL);

        void Stop();

        bool AddCounter(const char* szFullCounterPath);

        void RemoveCounter(const char* szFullCounterPath);

        bool GetCounterValue(const char* szFullCounterPath, double& dbValue);

        bool GetCounterValue(map<string, double>& counterValueMap);

    protected:

        virtual void Proc();

    private:

        static CPerformanceCounterMonitor s_CPerformanceCounterMonitor;

        bool m_bStop;

        UINT32 m_nIntervalTime;

        SysBase::CThread m_CThread;

        SysBase::CEvent m_CEvent;

        CPerformanceCounter m_CPerformanceCounter;
    };
}
