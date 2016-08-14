#include "../SysBase_Interface.h"

namespace SysBase
{


    //////////////////////////////////////////////////////////////////////////

    IStatisticsWorkerHandler::IStatisticsWorkerHandler()
    {

    }

    IStatisticsWorkerHandler::~IStatisticsWorkerHandler()
    {

    }

    //////////////////////////////////////////////////////////////////////////

    CStatisticsWorker::CStatisticsWorker()
    {
        m_bStop = true;

        m_pData = NULL;

        m_nTempData = NULL;

        m_nWaitTime = 0;
    }

    CStatisticsWorker::~CStatisticsWorker()
    {
        this->Stop();
    }

    bool CStatisticsWorker::Start(TStatisticsParam& aParam)
    {
        UINT8 nIndex = 0;

        if (!aParam.pHandler)
        {
            return false;
        }

        if (0 == aParam.nTypeCount)
        {
            return false;
        }

        for (nIndex = 0; nIndex < aParam.nTypeCount; ++nIndex)
        {
            if (0 == aParam.nStatisticsCountArray[nIndex])
            {
                return false;
            }
        }

        //////////////////////////////////////////////////////////////////////////

        m_bStop = false;

        m_param = aParam;

        if (m_param.nOnTimeSecond < 5)
        {
            m_param.nOnTimeSecond = 5;
        }

        m_nWaitTime = (UINT32)m_param.nOnTimeSecond * 1000;

        //////////////////////////////////////////////////////////////////////////
        //构造数据

        UINT8 nMaxValueCount = 0;

        m_pData = new TStatisticsData[m_param.nTypeCount];

        for (nIndex = 0; nIndex < m_param.nTypeCount; ++nIndex)
        {
            m_pData[nIndex].nCount = m_param.nStatisticsCountArray[nIndex];

            m_pData[nIndex].pnValue = new(std::nothrow) INT64[m_pData[nIndex].nCount];

            if (!m_pData[nIndex].pnValue)
            {
                this->Stop();
                return false;
            }
            
            ZeroMemory(m_pData[nIndex].pnValue, sizeof(INT64) * m_pData[nIndex].nCount);

            if (nMaxValueCount < m_pData[nIndex].nCount)
            {
                nMaxValueCount = m_pData[nIndex].nCount;
            }
        }

        m_nTempData = new(std::nothrow) INT64[nMaxValueCount];

        if (!m_nTempData)
        {
            this->Stop();
            return false;
        }

        //////////////////////////////////////////////////////////////////////////

        m_event.Reset();

        if (CCustomThread::ERROR_CODE_SUCCESS != CCustomThread::Start())
        {
            this->Stop();
            return false;
        }

        return true;
    }

    void CStatisticsWorker::Stop()
    {
        if (m_bStop)
        {
            return;
        }

        m_bStop = true;
        m_event.Set();

        CCustomThread::Stop();

        if (m_pData)
        {
            for (UINT8 nIndex = 0; nIndex < m_param.nTypeCount; ++nIndex)
            {
                if (m_pData[nIndex].pnValue)
                {
                    delete [] m_pData[nIndex].pnValue;
                }
            }

            delete [] m_pData;
        }

        if (m_nTempData)
        {
            delete [] m_nTempData;
        }
    }

    void CStatisticsWorker::SetOnTimeSecond(UINT16 nSecond)
    {
        if (nSecond >= 5)
        {
            m_param.nOnTimeSecond = nSecond;
            m_nWaitTime  = (UINT32)nSecond * 60 * 1000;
        }
        else
        {
            m_param.nOnTimeSecond = 5;
            m_nWaitTime  = (UINT32)5 * 60 * 1000;
        }
    }

    void CStatisticsWorker::Proc()
    {
        while (!m_event.Wait(m_nWaitTime) && !m_bStop)
        {
            if (m_param.pHandler)
            {
                for (UINT8 nTypeIndex = 0; nTypeIndex < m_param.nTypeCount; ++nTypeIndex)
                {
                    TStatisticsData* pData = m_pData + nTypeIndex;

                    ZeroMemory(m_nTempData, sizeof(UINT64) * pData->nCount);

                    for (UINT8 nValueIndex = 0; nValueIndex < pData->nCount; ++nValueIndex)
                    {
                        m_nTempData[nValueIndex] = InterlockedExchange64(pData->pnValue + nValueIndex, 0);
                    }

                    m_param.pHandler->OnTime(m_nTempData, pData->nCount, nTypeIndex);
                }
            }
        }
    }

    bool CStatisticsWorker::IsStatisticsValid(UINT8 nTypeIndex, UINT8 nStatisticsIndex)
    {
        if (m_bStop)
        {
            return false;
        }

        if (m_param.nTypeCount < nTypeIndex)
        {
            return false;
        }

        if (m_pData[nTypeIndex].nCount < nStatisticsIndex)
        {
            return false;
        }

        return true;
    }

    void CStatisticsWorker::Increment(UINT8 nTypeIndex, UINT8 nStatisticsIndex)
    {
        if (!this->IsStatisticsValid(nTypeIndex, nStatisticsIndex))
        {
            return;
        }

        InterlockedIncrement64(m_pData[nTypeIndex].pnValue + nStatisticsIndex);
    }

    void CStatisticsWorker::Decrement(UINT8 nTypeIndex, UINT8 nStatisticsIndex)
    {
        if (!this->IsStatisticsValid(nTypeIndex, nStatisticsIndex))
        {
            return;
        }

        InterlockedDecrement64(m_pData[nTypeIndex].pnValue + nStatisticsIndex);
    }

    void CStatisticsWorker::Exchange(UINT8 nTypeIndex, UINT8 nStatisticsIndex, INT64 nValue)
    {
        if (!this->IsStatisticsValid(nTypeIndex, nStatisticsIndex))
        {
            return;
        }

        InterlockedExchange64(m_pData[nTypeIndex].pnValue + nStatisticsIndex, nValue);
    }

    void CStatisticsWorker::ExchangeAdd(UINT8 nTypeIndex, UINT8 nStatisticsIndex, INT64 nValue)
    {
        if (!this->IsStatisticsValid(nTypeIndex, nStatisticsIndex))
        {
            return;
        }

        InterlockedExchangeAdd64(m_pData[nTypeIndex].pnValue + nStatisticsIndex, nValue);
    }

    void CStatisticsWorker::SetMax(UINT8 nTypeIndex, UINT8 nStatisticsIndex, INT64 nValue)
    {
        if (!this->IsStatisticsValid(nTypeIndex, nStatisticsIndex))
        {
            return;
        }

        if (nValue > m_pData[nTypeIndex].pnValue[nStatisticsIndex])
        {
            InterlockedExchange64(m_pData[nTypeIndex].pnValue + nStatisticsIndex, nValue);
        }
    }

    void CStatisticsWorker::SetMin(UINT8 nTypeIndex, UINT8 nStatisticsIndex, INT64 nValue)
    {
        if (!this->IsStatisticsValid(nTypeIndex, nStatisticsIndex))
        {
            return;
        }

        if (nValue < m_pData[nTypeIndex].pnValue[nStatisticsIndex])
        {
            InterlockedExchange64(m_pData[nTypeIndex].pnValue + nStatisticsIndex, nValue);
        }
    }
}