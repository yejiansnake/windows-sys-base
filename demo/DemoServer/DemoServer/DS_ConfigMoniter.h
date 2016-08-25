#pragma once

#include "DS_Common.h"

namespace EM
{
    class CConfigMoniter : public CSingletonT<CConfigMoniter>, public IFileMonitorWorkerHandler
    {
    public:

        CConfigMoniter();

        virtual ~CConfigMoniter();

        bool Start();

        void Stop();

    protected:

        void OnChanged(bool bChangeSize, bool bChangeAttributes, bool bChangeLastWrite);

    private:

        bool m_bStop;

        CEvent m_stopEvent;

        CFileMonitorWorker m_worker;
    };
}