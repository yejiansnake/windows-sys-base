#pragma once

#include "DS_Common.h"

namespace EM
{
    class CStatisticsThread : protected IStatisticsWorkerHandler, public CSingletonT<CStatisticsThread>
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        enum ESystemStatistics
        {
            ESS_RECV_COUNT = 0,
            ESS_SEND_COUNT,
            ESS_CLIENT_COUNT,

            ESS_MAX_COUNT,
        };

        //////////////////////////////////////////////////////////////////////////

        CStatisticsThread();

        virtual ~CStatisticsThread();

        bool Start();

        void Stop();

        void Increment(UINT8 nStatisticsIndex);

        void Exchange(UINT8 nStatisticsIndex, INT64 nValue);

    protected:

         virtual void OnTime(INT64* nDataArray, UINT8 nDataCount, UINT8 nTypeIndex);

    private:

        CStatisticsWorker m_worker;
    };
}
