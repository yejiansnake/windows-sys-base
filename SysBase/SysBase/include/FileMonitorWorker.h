#pragma once

#include "Thread.h"
#include "ThreadSync.h"

namespace SysBase
{
    class IFileMonitorWorkerHandler
    {
    public:

        IFileMonitorWorkerHandler();

        virtual ~IFileMonitorWorkerHandler();

        virtual void OnChanged(bool bChangeSize, bool bChangeAttributes, bool bChangeLastWrite) = 0;
    };

    struct TFileMonitorWorkerParam 
    {
        TFileMonitorWorkerParam()
        {
            ZeroMemory(wzFilePath, sizeof(WCHAR) * MAX_PATH);

            bIsMonitorSize = false;
            bIsMonitorAttributes = false;
            bIsMonitorLastWrite = false;

            pHandler = NULL;
        }

        void SetFileName(const char* szFileName);

        WCHAR wzFilePath[MAX_PATH];

        bool bIsMonitorSize;
        bool bIsMonitorAttributes;
        bool bIsMonitorLastWrite;

        IFileMonitorWorkerHandler* pHandler;
    };

    class CFileMonitorWorker : protected CCustomThread
    {
    public:

        CFileMonitorWorker();

        virtual ~CFileMonitorWorker();

        bool Start(TFileMonitorWorkerParam& aParam);

        void Stop();

    protected:

        virtual void Proc();

    private:

        bool m_bStop;

        CEvent m_stopEvent;

        TFileMonitorWorkerParam m_param;
    };
}
