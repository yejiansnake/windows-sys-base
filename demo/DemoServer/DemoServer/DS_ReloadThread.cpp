#include "DS_ReloadThread.h"
#include "DS_Config.h"

#define SAS_RELOAD_EVENT_COUNT 2
#define SAS_RELOAD_ON_TIME_EVENT 0
#define SAS_RELOAD_STOP_EVENT 1

#define SAS_MIN_RELOAD_TICK_COUNT 60000

namespace EM
{
    CReloadThread::CReloadThread()
    {
        m_bStop = true;
    }

    CReloadThread::~CReloadThread()
    {
        this->Stop();
    }

    bool CReloadThread::Start()
    {
        if (0 == CConfig::Instance().GetReloadTimeSecond())
        {
            return true;
        }

        m_bStop = false;
        m_StopEvent.Reset();

        if (CCustomThread::ERROR_CODE_SUCCESS != CCustomThread::Start())
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, "CReloadThread::Start thread start failed !");

            return false;
        }

        return true;
    }

    void CReloadThread::Stop()
    {
        if (0 == CConfig::Instance().GetReloadTimeSecond())
        {
            return;
        }

        if (m_bStop)
        {
            return;
        }

        m_bStop = true;
        m_StopEvent.Set();

        CCustomThread::Stop(3000, 0);
    }

    void CReloadThread::ReloadNow()
    {
        if (m_bStop)
        {
            return;
        }

        m_ReLoadEvent.Set();
    }

    void CReloadThread::Proc()
    {
        HANDLE hEvents[SAS_RELOAD_EVENT_COUNT] = {0};

        CSyncObject* pSyncObject[SAS_RELOAD_EVENT_COUNT] = {0};

        pSyncObject[SAS_RELOAD_ON_TIME_EVENT] = &m_ReLoadEvent;
        pSyncObject[SAS_RELOAD_STOP_EVENT] = &m_StopEvent;

        UINT32 nCurrentTickCount = GetTickCount();
        UINT32 nLastReloadDBInfoTickCount = nCurrentTickCount;

        UINT32 nReloadWaitTime = CConfig::Instance().GetReloadTimeSecond() * 1000;

        if (0 == nReloadWaitTime)
        {
            nReloadWaitTime = INFINITE;
        }

        while (!m_bStop)
        {
            UINT32 nWaitIndex = 0;

            if (CSyncObject::WaitSyncObjects(pSyncObject, SAS_RELOAD_EVENT_COUNT, false, nReloadWaitTime, nWaitIndex))
            {
                switch (nWaitIndex)
                {
                case SAS_RELOAD_ON_TIME_EVENT:
                    {
                        nCurrentTickCount = GetTickCount();

                        if (nCurrentTickCount - nLastReloadDBInfoTickCount > SAS_MIN_RELOAD_TICK_COUNT)
                        {
                            nLastReloadDBInfoTickCount = nCurrentTickCount;
                        }
                        else
                        {
                        }
                    }
                    break;
                case SAS_RELOAD_STOP_EVENT:
                    {
                        return;
                    }
                }
            }
            else
            {
                m_ReLoadEvent.Set();
            }
        }
    }
}