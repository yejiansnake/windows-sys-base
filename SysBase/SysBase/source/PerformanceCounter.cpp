#include "../SysBase_Interface.h"

#include <Pdh.h>
#include <PdhMsg.h>

#pragma comment(lib, "Pdh.lib")

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CPerformanceCounterImp

    class CPerformanceCounterImp
    {
    public:

        CPerformanceCounterImp();

        virtual ~CPerformanceCounterImp();

        bool Init(const char* szMachineName = NULL);

        void Close();

        bool Add(const char* szFullCounterPath);

        void Remove(const char* szFullCounterPath);

        bool GetValue(const char* szFullCounterPath, double& dbValue);

        bool GetValue(map<string, double>& counterValueMap);

        void CollectData();

    private:

        HQUERY m_hQuery ;       //计数器对象句柄

        typedef map<string, HCOUNTER> CounterMap;

        CounterMap m_CounterMap;    //计数器句柄集合

        CThreadMutex m_CThreadMutex;
    };

    //////////////////////////////////////////////////////////////////////////
    //CPerformanceCounterImp 实现

    CPerformanceCounterImp::CPerformanceCounterImp()
    {
        m_hQuery = NULL;
    }

    CPerformanceCounterImp::~CPerformanceCounterImp()
    {
        this->Close();
    }

    bool CPerformanceCounterImp::Init(const char* szMachineName)
    {
        if (ERROR_SUCCESS != PdhConnectMachineA(szMachineName))
        {
            return false;
        }

        if (ERROR_SUCCESS != PdhOpenQueryA(NULL, 0, &m_hQuery))
        {
            return false;
        }

        return true;
    }

    void CPerformanceCounterImp::Close()
    {
        if (m_hQuery)
        {
            PdhCloseQuery(m_hQuery);

            m_hQuery = NULL;

            m_CounterMap.clear();
        }
    }

    bool CPerformanceCounterImp::Add(const char* szFullCounterPath)
    {
        if (!m_hQuery || !szFullCounterPath)
        {
            return false;
        }

        CREATE_SMART_MUTEX(m_CThreadMutex);

        if (m_CounterMap.find(szFullCounterPath) != m_CounterMap.end())
        {
            return true;
        }

        HCOUNTER hCounter = NULL;

        if (ERROR_SUCCESS != PdhAddCounterA(m_hQuery, szFullCounterPath, 0, &hCounter))
        {
            return false;
        }

        m_CounterMap.insert(CounterMap::value_type(szFullCounterPath, hCounter));

        return true;
    }

    void CPerformanceCounterImp::Remove(const char* szFullCounterPath)
    {
        if (!m_hQuery || !szFullCounterPath)
        {
            return;
        }

        CREATE_SMART_MUTEX(m_CThreadMutex);

        CounterMap::iterator entity = m_CounterMap.find(szFullCounterPath);

        if (entity == m_CounterMap.end())
        {
            return;
        }

        PdhRemoveCounter(entity->second);

        m_CounterMap.erase(entity);
    }

    bool CPerformanceCounterImp::GetValue(const char* szFullCounterPath, double& dbValue)
    {
        if (!m_hQuery || !szFullCounterPath)
        {
            return false;
        }

        CREATE_SMART_MUTEX(m_CThreadMutex);

        CounterMap::iterator entity = m_CounterMap.find(szFullCounterPath);

        if (entity == m_CounterMap.end())
        {
            return false;
        }

        DWORD dwCounterType = 0;
        PDH_FMT_COUNTERVALUE DisplayValue = {0};          //计数器值  

        if (ERROR_SUCCESS != PdhGetFormattedCounterValue(entity->second,   
            PDH_FMT_DOUBLE, &dwCounterType, &DisplayValue))
        {
            dbValue = 0.00;
        }
        else
        {
            dbValue = DisplayValue.doubleValue;  
        }

        return true;
    }

    bool CPerformanceCounterImp::GetValue(map<string, double>& counterValueMap)
    {
        counterValueMap.clear();

        if (!m_hQuery)
        {
            return false;
        }

        CREATE_SMART_MUTEX(m_CThreadMutex);

        CounterMap::iterator entity = m_CounterMap.begin();
        CounterMap::iterator endEntity = m_CounterMap.end();

        for (; entity != endEntity; ++entity)
        {
            DWORD dwCounterType = 0;
            PDH_FMT_COUNTERVALUE DisplayValue = {0}; 

            if (ERROR_SUCCESS != PdhGetFormattedCounterValue(entity->second,   
                PDH_FMT_DOUBLE, &dwCounterType, &DisplayValue))
            {
                counterValueMap[entity->first] = 0.00;
            }
            else
            {
                counterValueMap[entity->first] = DisplayValue.doubleValue; 
            }
        }

        return true;
    }

    void CPerformanceCounterImp::CollectData()
    {
        if (!m_hQuery)
        {
            return;
        }

        PdhCollectQueryData(m_hQuery);
    }

    //////////////////////////////////////////////////////////////////////////
    //CPerformanceCounter

    CPerformanceCounter::CPerformanceCounter()
    {
        m_pImp = new(std::nothrow) CPerformanceCounterImp();
    }

    CPerformanceCounter::~CPerformanceCounter()
    {
        if (m_pImp)
        {
            CPerformanceCounterImp* pImp = (CPerformanceCounterImp*)m_pImp;

            delete pImp;

            m_pImp = NULL;
        }
    }

    bool CPerformanceCounter::Init(const char* szMachineName)
    {
        if (!m_pImp)
        {
            return false;
        }

        CPerformanceCounterImp* pImp = (CPerformanceCounterImp*)m_pImp;

        return pImp->Init(szMachineName);
    }

    void CPerformanceCounter::Close()
    {
        if (!m_pImp)
        {
            return;
        }

        CPerformanceCounterImp* pImp = (CPerformanceCounterImp*)m_pImp;

        pImp->Close();
    }

    bool CPerformanceCounter::Add(const char* szFullCounterPath)
    {
        if (!m_pImp)
        {
            return false;
        }

        CPerformanceCounterImp* pImp = (CPerformanceCounterImp*)m_pImp;

        return pImp->Add(szFullCounterPath);
    }

    void CPerformanceCounter::Remove(const char* szFullCounterPath)
    {
        if (!m_pImp)
        {
            return;
        }

        CPerformanceCounterImp* pImp = (CPerformanceCounterImp*)m_pImp;
        
        return pImp->Remove(szFullCounterPath);
    }

    bool CPerformanceCounter::GetValue(const char* szFullCounterPath, double& dbValue)
    {
        if (!m_pImp)
        {
            return false;
        }

        CPerformanceCounterImp* pImp = (CPerformanceCounterImp*)m_pImp;

        return pImp->GetValue(szFullCounterPath, dbValue);
    }

    bool CPerformanceCounter::GetValue(map<string, double>& counterValueMap)
    {
        if (!m_pImp)
        {
            return false;
        }

        CPerformanceCounterImp* pImp = (CPerformanceCounterImp*)m_pImp;

        return pImp->GetValue(counterValueMap);
    }

    void CPerformanceCounter::CollectData()
    {
        if (!m_pImp)
        {
            return;
        }

        CPerformanceCounterImp* pImp = (CPerformanceCounterImp*)m_pImp;

        pImp->CollectData();
    }

    bool CPerformanceCounter::IsCounterPathValid(const char* szFullCounterPath)
    {
        PDH_STATUS status = PdhValidatePathA(szFullCounterPath);
        if (ERROR_SUCCESS != status)
        {
            return false;
        }

        return true;
    }

    UINT32 CPerformanceCounter::BuildCounterPath(
        const char* szObjectName, 
        const char* szInstanceName, 
        const char* szObjectItemName,
        char* szCounterPathBufferOut,
        UINT32 nBufferSize)
        {
            if (!szObjectName || !szObjectItemName)
            {
                return 0;
            }

            if (NULL == szObjectName[0] || NULL == szObjectItemName)
            {
                return 0;
            }

            //object(instance)\counter 
            char szCounterPath[1024] = {0};

            UINT32 nLen = 0;

            bool bHasInstance = false;

            if (szInstanceName)
            {
                if (NULL != szInstanceName[0])
                {
                    bHasInstance = true;
                }
            }

            if (bHasInstance)
            {
                nLen = (UINT32)sprintf_s(szCounterPath, 1023, "\\%s(%s)\\%s", szObjectName, szInstanceName, szObjectItemName);
            }
            else
            {
                nLen = (UINT32)sprintf_s(szCounterPath, 1023, "\\%s\\%s", szObjectName, szObjectItemName);
            }

            if (nLen > nBufferSize)
            {
                return 0;
            }

            memcpy(szCounterPathBufferOut, szCounterPath, nLen);

            return nLen;
        }

    bool CPerformanceCounter::GetCounterObjectList(list<string>& counterObjectListOut, const char* szMachineName)
    {
        char* pBuffer = NULL;
        DWORD dwBufferSize = 0;

        DWORD dwDetailLevel = PERF_DETAIL_NOVICE | PERF_DETAIL_ADVANCED | PERF_DETAIL_EXPERT | PERF_DETAIL_WIZARD;

        PDH_STATUS status = ERROR_SUCCESS;  

        counterObjectListOut.clear();

        status = PdhEnumObjectsA(NULL, szMachineName, NULL, &dwBufferSize, dwDetailLevel, TRUE);

        if (PDH_MORE_DATA != status)
        {
            return false;
        }

        pBuffer = new char[dwBufferSize];
        ZeroMemory(pBuffer, dwBufferSize);

        status = PdhEnumObjectsA(NULL, szMachineName, pBuffer, &dwBufferSize, dwDetailLevel, FALSE);

        if (ERROR_SUCCESS != status)
        {
            return false;
        }

        char* pStart = pBuffer;
        char* pEnd = pBuffer + dwBufferSize;
        int nLen = 0;

        int nObjectCount = 0;

        while (pStart <= pEnd)
        {
            nLen = (int)strlen(pStart);

            if (nLen <= 0)
            {
                break;
            }

            counterObjectListOut.push_back(pStart);

            nObjectCount++;

            pStart = pStart + nLen + 1;
        }

        delete [] pBuffer;
        pBuffer = NULL;
        dwBufferSize = 0;

        return true;
    }

    bool CPerformanceCounter::GetCounterObjectItemList(
        list<string>& conterObjectItemListOut, 
        list<string>& counterInstanceListOut, 
        const char* szObjectName, 
        const char* szMachineName)
    {
        if (!szObjectName)
        {
            return false;
        }

        DWORD dwBufferSize = 0;
        DWORD dwBufferTwoSize = 0;
        DWORD dwDetailLevel = PERF_DETAIL_NOVICE | PERF_DETAIL_ADVANCED | PERF_DETAIL_EXPERT | PERF_DETAIL_WIZARD;

        char* pBuffer = NULL;
        char*  pBufferTwo = NULL;

        PDH_STATUS status = ERROR_SUCCESS;  

        conterObjectItemListOut.clear();

        counterInstanceListOut.clear();

        status = PdhEnumObjectItemsA(NULL, szMachineName, szObjectName, NULL, &dwBufferSize, NULL, &dwBufferTwoSize, 
            dwDetailLevel, 0);

        if (PDH_MORE_DATA != status)
        {
            return false;
        }

        pBuffer = new char[dwBufferSize];
        ZeroMemory(pBuffer, dwBufferSize);

        if (dwBufferTwoSize > 0)
        {
            pBufferTwo = new char[dwBufferTwoSize];
            ZeroMemory(pBufferTwo, dwBufferTwoSize);
        }
        else
        {
            pBufferTwo = NULL;
        }

        status = PdhEnumObjectItemsA(NULL, szMachineName, szObjectName, pBuffer, &dwBufferSize, pBufferTwo, &dwBufferTwoSize, dwDetailLevel, 0);

        if (ERROR_SUCCESS != status)
        {
            return false;
        }

        if (pBuffer)
        {
            char* pStart = pBuffer;
            char* pEnd = pBuffer + dwBufferSize;
            int nLen = 0;

            while (pStart <= pEnd)
            {
                nLen = (int)strlen(pStart);

                if (nLen <= 0)
                {
                    break;
                }

                conterObjectItemListOut.push_back(pStart);

                pStart = pStart + nLen + 1;
            }
        }

        if (pBufferTwo)
        {
            char* pStart = pBufferTwo;
            char* pEnd = pBufferTwo + dwBufferTwoSize;
            int nLen = 0;

            while (pStart <= pEnd)
            {
                nLen = (int)strlen(pStart);

                if (nLen <= 0)
                {
                    break;
                }

                counterInstanceListOut.push_back(pStart);

                pStart = pStart + nLen + 1;
            }
        }

        if (pBuffer)
        {
            delete [] pBuffer;
            pBuffer = NULL;
            dwBufferSize = 0;
        }

        if (pBufferTwo)
        {
            delete [] pBufferTwo;
            pBufferTwo = NULL;
            dwBufferTwoSize = 0;
        }

        return true;
    }

    bool CPerformanceCounter::GetCounterObjectItemList(
        list<string>& conterObjectItemListOut, 
        list<string>& counterInstanceListOut, 
        const string& szObjectName, 
        const char* szMachineName)
    {
        return CPerformanceCounter::GetCounterObjectItemList(conterObjectItemListOut, counterInstanceListOut, szObjectName.c_str(), szMachineName);
    }

    //////////////////////////////////////////////////////////////////////////
    //CPerformanceCounterMonitor

    CPerformanceCounterMonitor CPerformanceCounterMonitor::s_CPerformanceCounterMonitor;

    CPerformanceCounterMonitor& CPerformanceCounterMonitor::Instance()
    {
        return s_CPerformanceCounterMonitor;
    }

    CPerformanceCounterMonitor::CPerformanceCounterMonitor()
    {
        m_bStop = true;

        m_nIntervalTime = 1000;
    }

    CPerformanceCounterMonitor::~CPerformanceCounterMonitor()
    {
        this->Stop();
    }

    bool CPerformanceCounterMonitor::Start(UINT32 nIntervalTime, const char* szMachineName)
    {
        m_nIntervalTime = nIntervalTime;

        m_CEvent.Reset();

        if (!m_CPerformanceCounter.Init(szMachineName))
        {
            return false;
        }

        m_bStop = false;

        if (CThread::ERROR_CODE_SUCCESS != m_CThread.Start(this))
        {
            m_CPerformanceCounter.Close();

            m_bStop = true;

            return false;
        }

        return true;
    }

    void CPerformanceCounterMonitor::Stop()
    {
        if (m_bStop)
        {
            return;
        }

        m_bStop = true;

        m_CEvent.Set();

        m_CThread.Terminate(3000, 0);

        m_CThread.Close();

        m_CPerformanceCounter.Close();
    }

    bool CPerformanceCounterMonitor::AddCounter(const char* szFullCounterPath)
    {
        if (m_bStop)
        {
            return false;
        }

        return m_CPerformanceCounter.Add(szFullCounterPath);
    }

    void CPerformanceCounterMonitor::RemoveCounter(const char* szFullCounterPath)
    {
        if (m_bStop)
        {
            return;
        }

        return m_CPerformanceCounter.Remove(szFullCounterPath);
    }

    bool CPerformanceCounterMonitor::GetCounterValue(const char* szFullCounterPath, double& dbValue)
    {
        if (m_bStop)
        {
            return false;
        }

        return m_CPerformanceCounter.GetValue(szFullCounterPath, dbValue);
    }

    bool CPerformanceCounterMonitor::GetCounterValue(map<string, double>& counterValueMap)
    {
        if (m_bStop)
        {
            return false;
        }

        return m_CPerformanceCounter.GetValue(counterValueMap);
    }

    void CPerformanceCounterMonitor::Proc()
    {
        while (!m_bStop)
        {
            if (m_CEvent.Wait(m_nIntervalTime))
            {
                return;
            }
            else
            {
                m_CPerformanceCounter.CollectData();
            }
        }
    }
}