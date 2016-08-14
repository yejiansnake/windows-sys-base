#include "../SysBase_Interface.h"

#include <psapi.h>
#include <Tlhelp32.h>

#pragma comment( lib, "PSAPI.LIB" )

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CProcUsageCollecterImp

    typedef map<UINT32, UINT64> ProcessTimeMap;

    class CSystemPerformanceCollecterImp : private CCustomThread
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        CSystemPerformanceCollecterImp();

        virtual ~CSystemPerformanceCollecterImp();

        //////////////////////////////////////////////////////////////////////////

        CSystemPerformanceCollecter::ERROR_CODE Start(
            UINT32 collectTypes,
            CSystemPerformanceCollecter::COLLECT_MODE collectMode,
            UINT32 nIntervalTime, 
            bool bSuspend);

        void Stop();

        CSystemPerformanceCollecter::ERROR_CODE Suspend();

        CSystemPerformanceCollecter::ERROR_CODE Resume();

        CSystemPerformanceCollecter::ERROR_CODE SetIntervalTime(UINT32 nIntervalTime);

        CSystemPerformanceCollecter::ERROR_CODE SetCollectTypes(UINT32 collectTypes);

        CSystemPerformanceCollecter::ERROR_CODE AddProcess(const wchar_t* szProcessName);

        CSystemPerformanceCollecter::ERROR_CODE RemoveProcess(const wchar_t* szProcessName);

        CSystemPerformanceCollecter::ERROR_CODE Get(ProcessPerformanceMap* pProcessPerformanceMap, 
            TSystemPerformance* pTSystemPerformance);

    protected:

        void CollectProcessUsage();

    private:

        virtual void Proc();

        ProcessPerformanceMap m_ProcessPerformanceMap;

        TSystemPerformance m_TSystemPerformance;

        SysBase::CEvent m_CEvent;

        CThreadMutex m_CThreadMutex;

        UINT32 m_nIntervalTime;

        bool m_bIsStop;

        UINT64 m_nLastSystemTotalTime;

        map<UINT32, UINT64> m_lastProcessTime;

        UINT64 m_nLastSystemIdleTime;

        bool m_bSuspend;

        UINT32 m_collectTypes;

        CSystemPerformanceCollecter::COLLECT_MODE m_collectMode;

        typedef map<wstring, char> ProcessNameMap;

        ProcessNameMap m_processNameMap;

        CThreadMutex m_processNameThreadMutex;
    };

    //////////////////////////////////////////////////////////////////////////
    //CProcUsageCollecterImp 实现

    CSystemPerformanceCollecterImp::CSystemPerformanceCollecterImp()
    {
        m_nIntervalTime = 1000;

        m_bIsStop = true;

        m_bSuspend = false;

        m_collectTypes = CSystemPerformanceCollecter::COLLECT_TYPE_PROC_BASE;

        m_collectMode = CSystemPerformanceCollecter::COLLECT_MODE_ALL;

        m_processNameMap.clear();
    }

    CSystemPerformanceCollecterImp::~CSystemPerformanceCollecterImp()
    {
        this->Stop();
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecterImp::Start(
        UINT32 collectTypes,
        CSystemPerformanceCollecter::COLLECT_MODE collectMode,
        UINT32 nIntervalTime, 
        bool bSuspend)
    {
        if (!m_bIsStop)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_HAS_START;
        }

        if (0 == nIntervalTime)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_PARAM;
        }

        m_nIntervalTime = nIntervalTime;

        m_bSuspend = bSuspend;

        m_collectTypes = collectTypes;

        m_collectMode = collectMode;

        m_processNameMap.clear();

        //////////////////////////////////////////////////////////////////////////
        //提升本进程的权限

        //if (!CProcessHelper::EnableDebugPrivilege(true))
        //{
        //    //return CSystemPerformanceCollecter::ERROR_CODE_SYSTEM;
        //}

        //////////////////////////////////////////////////////////////////////////

        if (CCustomThread::ERROR_CODE_SUCCESS != CCustomThread::Start())
        {
            return CSystemPerformanceCollecter::ERROR_CODE_SYSTEM;
        }

        return CSystemPerformanceCollecter::ERROR_CODE_SUCCESS;
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecterImp::SetIntervalTime(UINT32 nIntervalTime)
    {
        if (0 == nIntervalTime)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_PARAM;
        }

        if (0 == nIntervalTime)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_PARAM;
        }

        m_nIntervalTime = nIntervalTime;

        return CSystemPerformanceCollecter::ERROR_CODE_SUCCESS;
    }

    void CSystemPerformanceCollecterImp::Stop()
    {
        if (m_bIsStop)
        {
            return;
        }

        m_bIsStop = true;

        m_CEvent.Set();

        CCustomThread::Stop();
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecterImp::Suspend()
    {
        if (m_bIsStop)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_NO_START;
        }

        m_bSuspend = true;

        return CSystemPerformanceCollecter::ERROR_CODE_SUCCESS;
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecterImp::Resume()
    {
        if (m_bIsStop)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_NO_START;
        }

        m_bSuspend = false;

        return CSystemPerformanceCollecter::ERROR_CODE_SUCCESS;
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecterImp::AddProcess(const wchar_t* szProcessName)
    {
        if (!szProcessName)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_PARAM;
        }

        if (m_bIsStop)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_NO_START;
        }

        if (CSystemPerformanceCollecter::COLLECT_MODE_ASSIGN != m_collectMode)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_NO_ASSIGN;
        }

        CREATE_SMART_MUTEX(m_processNameThreadMutex);

        m_processNameMap[szProcessName] = 0;

        return CSystemPerformanceCollecter::ERROR_CODE_SUCCESS;
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecterImp::RemoveProcess(const wchar_t* szProcessName)
    {
        if (!szProcessName)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_PARAM;
        }

        if (m_bIsStop)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_NO_START;
        }

        if (CSystemPerformanceCollecter::COLLECT_MODE_ASSIGN != m_collectMode)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_NO_ASSIGN;
        }

        CREATE_SMART_MUTEX(m_processNameThreadMutex);

        m_processNameMap.erase(szProcessName);

        return CSystemPerformanceCollecter::ERROR_CODE_SUCCESS;
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecterImp::SetCollectTypes(UINT32 collectTypes)
    {
        if (m_bIsStop)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_NO_START;
        }

        m_collectTypes = collectTypes;

        return CSystemPerformanceCollecter::ERROR_CODE_SUCCESS;
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecterImp::Get(
        ProcessPerformanceMap* pProcessPerformanceMap, 
        TSystemPerformance* pTSystemPerformance)
    {
        if (m_bIsStop)
        {
            return CSystemPerformanceCollecter::ERROR_CODE_NO_START;
        }

        CREATE_SMART_MUTEX(m_CThreadMutex);

        if (pProcessPerformanceMap)
        {
            *pProcessPerformanceMap = m_ProcessPerformanceMap;
        }

        if (pTSystemPerformance)
        {
            *pTSystemPerformance = m_TSystemPerformance;
        }

        return CSystemPerformanceCollecter::ERROR_CODE_SUCCESS;
    }

    void CSystemPerformanceCollecterImp::Proc()
    {
        m_bIsStop = false;

        m_CEvent.Reset();

        m_nLastSystemIdleTime = 0;

        m_nLastSystemTotalTime = 0;

        m_lastProcessTime.clear();

        m_ProcessPerformanceMap.clear();

        ZeroMemory(&m_TSystemPerformance, sizeof(m_TSystemPerformance));

        while (!m_bIsStop)
        {
            if (m_bSuspend)
            {
                Sleep(1);

                continue;
            }

            if (m_CEvent.Wait(m_nIntervalTime))
            {
                return;
            }

            this->CollectProcessUsage();
        }
    }

    void CSystemPerformanceCollecterImp::CollectProcessUsage()
    {
        ProcessPerformanceMap aProcessPerformanceMap;
        TSystemPerformance aTSystemPerformance = {0};

        ProcessTimeMap curProcessTime;
        UINT64 nCurTotalTime = 0;
        UINT64 nCurSystemIdleTime = 0;

        //////////////////////////////////////////////////////////////////////////
        //获取系统运作时间

        FILETIME fSystemI = {0};
        FILETIME fSystemK = {0};
        FILETIME fSystemU = {0};

        GetSystemTimes(&fSystemI, &fSystemK, &fSystemU);

        UINT64 nSystemI = 0;
        UINT64 nSystemK = 0;
        UINT64 nSystemU = 0;

        memcpy(&nSystemI, &fSystemI, sizeof(UINT64));
        memcpy(&nSystemK, &fSystemK, sizeof(UINT64));
        memcpy(&nSystemU, &fSystemU, sizeof(UINT64));

        nCurTotalTime = nSystemK + nSystemU;

        double dbIntervalTotalTime =  (double)(nCurTotalTime - m_nLastSystemTotalTime);

        //////////////////////////////////////////////////////////////////////////
        //获取进程运作时间

        HANDLE hProcessSnap;
        PROCESSENTRY32W processEntry = {0};
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (hProcessSnap == INVALID_HANDLE_VALUE)
        {
            return;
        }

        if (!Process32FirstW(hProcessSnap, &processEntry))
        {
            CloseHandle(hProcessSnap);

            return;
        }

        do
        {
            if (0 == processEntry.th32ProcessID)
            {
                continue;
            }

            if (CSystemPerformanceCollecter::COLLECT_MODE_ASSIGN == m_collectMode)
            {
                CREATE_SMART_MUTEX(m_processNameThreadMutex);

                if (m_processNameMap.find(processEntry.szExeFile) == m_processNameMap.end())
                {
                    continue;
                }
            }

            CProcess aCProcess;

            if (CProcess::ERROR_CODE_SUCCESS != aCProcess.Open(processEntry.th32ProcessID,
                CProcess::OPEN_RIGHT_TYPE_QUERY_LIMITED))
            {
                //如果无法打开进程，则继续运行下一个
                continue;
            }

            //////////////////////////////////////////////////////////////////////////

            TProcessPerformance& aTProcessPerformance = aProcessPerformanceMap[processEntry.th32ProcessID];

            aCProcess.GetProcessTime(aTProcessPerformance.aTime);

            curProcessTime[processEntry.th32ProcessID] = aTProcessPerformance.aTime.nKernelTime 
                + aTProcessPerformance.aTime.nUserTime;

            UINT64 nLastProcessTime = m_lastProcessTime[processEntry.th32ProcessID];

            double dbIntervalProcessTime = (double)(aTProcessPerformance.aTime.nKernelTime 
                + aTProcessPerformance.aTime.nUserTime - nLastProcessTime);

            aTProcessPerformance.aCpuUsage.nCpuUsage 
                = (UINT8)(((dbIntervalProcessTime * 100) / dbIntervalTotalTime) + 0.5);

            //////////////////////////////////////////////////////////////////////////
            
            //一般信息
            aTProcessPerformance.aNormal.nProcessID = processEntry.th32ProcessID;
            aTProcessPerformance.aNormal.nParentProcessID = processEntry.th32ParentProcessID;
            aTProcessPerformance.aNormal.nThreadCount = processEntry.cntThreads;
            aTProcessPerformance.aNormal.nPriClassBase = processEntry.pcPriClassBase;
            memcpy(aTProcessPerformance.aNormal.szExeFile, processEntry.szExeFile, MAX_PATH);

            //内存
            if (m_collectTypes & CSystemPerformanceCollecter::COLLECT_TYPE_PROC_MEMORY)
            {
                aCProcess.GetProcessMemory(aTProcessPerformance.aMemory);
            }

            //IO计数
            if (m_collectTypes & CSystemPerformanceCollecter::COLLECT_TYPE_PROC_IO)
            {
                aCProcess.GetProcessIOCounter(aTProcessPerformance.aIOCounter);
            }

            //资源数
            if (m_collectTypes & CSystemPerformanceCollecter::COLLECT_TYPE_PROC_RESOURCE)
            {
                aCProcess.GetProcessResource(aTProcessPerformance.aResource);
            }

            //高级信息
            if (m_collectTypes & CSystemPerformanceCollecter::COLLECT_TYPE_PROC_ADVANCED)
            {
                aCProcess.GetProcessAdvanced(aTProcessPerformance.aAdvanced);
            }
        }
        while(Process32NextW(hProcessSnap, &processEntry));

        CloseHandle(hProcessSnap);

        //////////////////////////////////////////////////////////////////////////
        //获取性能参数

        PERFORMANCE_INFORMATION performanceInformation = {0};

        if (m_collectTypes & CSystemPerformanceCollecter::COLLECT_TYPE_SYS_RESOURCE)
        {
            if (GetPerformanceInfo(&performanceInformation, sizeof(PERFORMANCE_INFORMATION)))
            {
                aTSystemPerformance.aResource.nCommitTotal = performanceInformation.CommitTotal * performanceInformation.PageSize;
                aTSystemPerformance.aResource.nCommitLimit = performanceInformation.CommitLimit * performanceInformation.PageSize;
                aTSystemPerformance.aResource.nCommitPeak = performanceInformation.CommitPeak * performanceInformation.PageSize;
                aTSystemPerformance.aResource.nPhysicalTotal = performanceInformation.PhysicalTotal * performanceInformation.PageSize;
                aTSystemPerformance.aResource.nPhysicalAvailable = performanceInformation.PhysicalAvailable * performanceInformation.PageSize;
                aTSystemPerformance.aResource.nSystemCache = performanceInformation.SystemCache * performanceInformation.PageSize;
                aTSystemPerformance.aResource.nKernelTotal = performanceInformation.KernelTotal * performanceInformation.PageSize;
                aTSystemPerformance.aResource.nKernelPaged = performanceInformation.KernelPaged * performanceInformation.PageSize;
                aTSystemPerformance.aResource.nKernelNonpaged = performanceInformation.KernelNonpaged * performanceInformation.PageSize;
                aTSystemPerformance.aResource.nPageSize = performanceInformation.PageSize;
                aTSystemPerformance.aResource.nHandleCount = performanceInformation.HandleCount;
                aTSystemPerformance.aResource.nProcessCount = performanceInformation.ProcessCount;
                aTSystemPerformance.aResource.nThreadCount = performanceInformation.ThreadCount;
            }
        }

        //////////////////////////////////////////////////////////////////////////
        //计算CPU率

        double dbIntervalIdleTime = (double)(nSystemI - m_nLastSystemIdleTime);

        //计算系统的CPU消耗(找出系统的进程资源消耗与CPU消耗情况)

        m_lastProcessTime = curProcessTime;
        m_nLastSystemTotalTime = nCurTotalTime;

        m_nLastSystemIdleTime = nSystemI;

        aTSystemPerformance.aCupUsage.nSystemIdleCpuUsage = (UINT8)(dbIntervalIdleTime * 100 / dbIntervalTotalTime + 0.5);
        aTSystemPerformance.aCupUsage.nSystemCpuUsage = 100 - aTSystemPerformance.aCupUsage.nSystemIdleCpuUsage;

        //////////////////////////////////////////////////////////////////////////
        //数据保存到缓存

        CREATE_SMART_MUTEX(m_CThreadMutex);

        m_ProcessPerformanceMap = aProcessPerformanceMap;
        m_TSystemPerformance = aTSystemPerformance;
    }

    //////////////////////////////////////////////////////////////////////////
    //CProcUsageCollecter

    CSystemPerformanceCollecter::CSystemPerformanceCollecter()
    {
        m_pImp = new(std::nothrow) CSystemPerformanceCollecterImp();
    }

    CSystemPerformanceCollecter::~CSystemPerformanceCollecter()
    {
        if (!m_pImp)
        {
            return;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        delete pImp;

        m_pImp = NULL;
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecter::Start(
        UINT32 collectTypes, 
        COLLECT_MODE collectMode,
        UINT32 nIntervalTime,
        bool bSuspend)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        return pImp->Start(collectTypes, collectMode, nIntervalTime, bSuspend);
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecter::SetIntervalTime(UINT32 nIntervalTime)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        return pImp->SetIntervalTime(nIntervalTime);
    }

    void CSystemPerformanceCollecter::Stop()
    {
        if (!m_pImp)
        {
            return;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        pImp->Stop();
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecter::Suspend()
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        return pImp->Suspend();
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecter::Resume()
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        return pImp->Resume();
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecter::AddProcess(const wchar_t* szProcessName)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        return pImp->AddProcess(szProcessName);
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecter::RemoveProcess(const wchar_t* szProcessName)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        return pImp->RemoveProcess(szProcessName);
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecter::SetCollectTypes(UINT32 collectTypes)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        return pImp->SetCollectTypes(collectTypes);
    }

    CSystemPerformanceCollecter::ERROR_CODE CSystemPerformanceCollecter::Get(
        ProcessPerformanceMap* pProcessPerformanceMap, 
        TSystemPerformance* pTSystemPerformance)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSystemPerformanceCollecterImp* pImp = (CSystemPerformanceCollecterImp*)m_pImp;

        return pImp->Get(pProcessPerformanceMap, pTSystemPerformance);
    }
}