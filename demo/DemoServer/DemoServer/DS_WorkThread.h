#pragma once

#include "DS_Common.h"
#include "DS_ContentHandler.h"

namespace EM
{
    class CTask : public ITaskHandler
    {
    public:

        CTask();

        virtual ~CTask();

        virtual void Proc(void* pParam);

        virtual void Stop(const TTaskInfo* pTTaskInfo);

    protected:
    private:

        CContentHandler m_handler;
    };

    //////////////////////////////////////////////////////////////////////////

    class CWorkThread : public CSingletonT<CWorkThread>, public ITaskHandlerFactory
    {
    public:

        friend class CTask;

        CWorkThread();

        virtual ~CWorkThread();

        bool Start();

        void Stop();

        void AddData(UINT32 nSocketID, UINT32 nIP, UINT16 nPort, const char* pData, UINT nDataSize);

    protected:

        virtual ITaskHandler* Create();

        virtual void Release(ITaskHandler* pITaskHandler);

    private:

        bool m_bStop;

        CBufferQueue m_queue;

        CTaskMultiThread m_TaskThread;
    };
}