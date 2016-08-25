#include "DS_WorkThread.h"
#include "DS_Config.h"

namespace EM
{
    //////////////////////////////////////////////////////////////////////////

    CTask::CTask()
    {

    }

    CTask::~CTask()
    {

    }

    void CTask::Proc(void* pParam)
    {
        ::CoInitialize(NULL);

        CWorkThread* pWorkThread = (CWorkThread*)pParam;

        CBufferQueue::ERROR_CODE errorCode = CBufferQueue::ERROR_CODE_SUCCESS;

        while (!pWorkThread->m_bStop)
        {
            CBufferRead bufferRead;

            errorCode = pWorkThread->m_queue.Pop(bufferRead);

            if (CBufferQueue::ERROR_CODE_SUCCESS != errorCode)
            {
                continue;
            }

            TSocketInfo* pTSocketInfo = (TSocketInfo*)bufferRead.GetData();

            const char* pData = bufferRead.GetData(sizeof(TSocketInfo));

            if (m_handler.ProcData(pTSocketInfo, pData))
            {

            }
            else
            {

            }
        }

        ::CoUninitialize();
    }

    void CTask::Stop(const TTaskInfo* pTTaskInfo)
    {
        
    }

    //////////////////////////////////////////////////////////////////////////

    CWorkThread::CWorkThread()
    {
        m_bStop = true;
    }

    CWorkThread::~CWorkThread()
    {
        this->Stop();
    }

    bool CWorkThread::Start()
    {
        if (!m_bStop)
        {
            return true;
        }

        m_queue.Release();
        if (CBufferQueue::ERROR_CODE_SUCCESS != m_queue.Init(
            CConfig::Instance().GetRecvBufferSize() + sizeof(TSocketInfo),
            CConfig::Instance().GetRecvBufferCount(),
            true))
        {
            return false;
        }

        m_bStop = false;
        
        if (CTaskMultiThread::ERROR_CODE_SUCCESS != m_TaskThread.Start(this, 
            CConfig::Instance().GetWorkThreadCount(),
            this))
        {
            this->Stop();

            return false;
        }

        return true;
    }

    void CWorkThread::Stop()
    {
        if (m_bStop)
        {
            return;
        }

        m_bStop = true;

        m_TaskThread.Stop();
    }

    void CWorkThread::AddData(UINT32 nSocketID, UINT32 nIP, UINT16 nPort, const char* pData, UINT nDataSize)
    {
        if (m_bStop)
        {
            return;
        }

        TSocketInfo socketInfo = {0};

        socketInfo.nSocketID = nSocketID;
        socketInfo.nIP = nIP;
        socketInfo.nPort = nPort;
        socketInfo.nDataSize = nDataSize;

        CBufferWrite bufferWirte;

        CBufferQueue::ERROR_CODE errorCode = m_queue.GetBufferWrite(bufferWirte);

        if (CBufferQueue::ERROR_CODE_SUCCESS != errorCode)
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, "CWorkThread::AddData queue get buffer wirte failed !");

            return;
        }

        bufferWirte.AddData(&socketInfo, sizeof(TSocketInfo));
        bufferWirte.AddData(pData, nDataSize);

        bufferWirte.Push();
    }

    ITaskHandler* CWorkThread::Create()
    {
        return new(std::nothrow) CTask();
    }

    void CWorkThread::Release(ITaskHandler* pITaskHandler)
    {
        if (!pITaskHandler)
        {
            return;
        }

        CTask* pTask = (CTask*)pITaskHandler;

        delete pTask;
    }
}