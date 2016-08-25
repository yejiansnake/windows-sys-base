#include "DS_ConfigMoniter.h"
#include "DS_Config.h"
#include "DS_Log.h"
#include "DS_ReloadThread.h"

namespace EM
{
    CConfigMoniter::CConfigMoniter()
    {
        m_bStop = true;
    }

    CConfigMoniter::~CConfigMoniter()
    {
        this->Stop();
    }

    bool CConfigMoniter::Start()
    {
        if (!m_bStop)
        {
            return true;
        }

        TFileMonitorWorkerParam aParam;

        aParam.pHandler = this;
        aParam.bIsMonitorLastWrite = true;
        aParam.SetFileName(CConfig::Instance().GetFilePath());

        if (!m_worker.Start(aParam))
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, "CConfigMoniter::Start start worker failed !");

            return false;
        }

        return true;
    }

    void CConfigMoniter::Stop()
    {
        if (m_bStop)
        {
            return;
        }

        m_bStop = TRUE;

        m_stopEvent.Set();

        m_worker.Stop();
    }

    void CConfigMoniter::OnChanged(bool bChangeSize, bool bChangeAttributes, bool bChangeLastWrite)
    {
        if (bChangeLastWrite)
        {
            CConfig::Instance().Reload();

            CLogLoader::ResetLogLevelFromConfig();

            CReloadThread::Instance().ReloadNow();
        }
    }

}