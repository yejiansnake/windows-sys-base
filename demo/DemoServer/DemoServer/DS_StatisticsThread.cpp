#include "DS_StatisticsThread.h"
#include "DS_Config.h"
#include "DS_NetServer.h"

namespace EM
{
    CStatisticsThread::CStatisticsThread()
    {
        
    }

    CStatisticsThread::~CStatisticsThread()
    {
        this->Stop();
    }

    bool CStatisticsThread::Start()
    {
        TStatisticsParam aParam;

        aParam.nTypeCount = 1;
        aParam.pHandler = this;

        aParam.nStatisticsCountArray[0] = (UINT8)ESS_MAX_COUNT;

        return m_worker.Start(aParam);
    }

    void CStatisticsThread::Stop()
    {
        m_worker.Stop();
    }

    void CStatisticsThread::Increment(UINT8 nStatisticsIndex)
    {
        m_worker.Increment(0, nStatisticsIndex);
    }

    void CStatisticsThread::Exchange(UINT8 nStatisticsIndex, INT64 nValue)
    {
        m_worker.Exchange(0, nStatisticsIndex, nValue);
    }

    void CStatisticsThread::OnTime(INT64* nDataArray, UINT8 nDataCount, UINT8 nTypeIndex)
    {
        if (!CConfig::Instance().GetIsOnStatLog())
        {
            return;
        }

        nDataArray[ESS_CLIENT_COUNT] = CNetServer::Instance().GetSocketCount();

        if (0 == nTypeIndex)
        {
            LOG_WRITE(LOG_KEY_STATISTICS, CLog::LOG_LEVEL_INFO, "RecvCount:%lld, SendCount:%lld, ClientCount:%lld", 
                nDataArray[ESS_RECV_COUNT], nDataArray[ESS_SEND_COUNT], nDataArray[ESS_CLIENT_COUNT]);
        }

    }
}