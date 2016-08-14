#include "../SysBase_Interface.h"

namespace SysBase
{
    CMsgWaitPool::CMsgWaitPool()
    {
        m_lKeyValue = 0;
    }

    CMsgWaitPool::~CMsgWaitPool()
    {
        CREATE_SMART_MUTEX(m_lock);

        m_msgEventMap.clear();
    }

    CMsgWaitPool::ERROR_CODE CMsgWaitPool::Init(UINT32 nWaitObjectCount)
    {
        if (0 ==nWaitObjectCount)
        {
            return ERROR_CODE_PARAM;
        }

        if (m_MsgEventPool.Init(nWaitObjectCount))
        {
            return ERROR_CODE_SYSTEM;
        }

        return ERROR_CODE_SUCCESS;
    }

    CMsgWaitPool::ERROR_CODE CMsgWaitPool::SubmitRequest(char* recvBuffer, UINT32 nMaxRecvSize, INT32& nWaitKeyIDOut)
    {
        if (!recvBuffer || 0 == nMaxRecvSize)
        {
            return ERROR_CODE_PARAM;
        }

        INT32 nWaitKeyID = InterlockedIncrement((LONG*)&m_lKeyValue);

        TMsgEvent* pMsgEvent = this->GetMsgEvent(nWaitKeyID);

        if (!pMsgEvent)
        {
            return ERROR_CODE_OBJECT_EMPTY;
        }
        pMsgEvent->pRecvBuffer = recvBuffer;
        pMsgEvent->nMaxRecvSize = nMaxRecvSize;

        nWaitKeyIDOut = nWaitKeyID;

        return ERROR_CODE_SUCCESS;
    }

    void CMsgWaitPool::CancelWait(INT32 nWaitKeyID)
    {
        CREATE_SMART_MUTEX(m_lock);

        MsgEventMap::iterator entity = m_msgEventMap.find(nWaitKeyID);

        if (entity != m_msgEventMap.end())
        {
            TMsgEvent* pMsgEvent = entity->second;
            m_msgEventMap.erase(entity);
            m_MsgEventPool.Push(pMsgEvent);
        }
    }

    CMsgWaitPool::ERROR_CODE CMsgWaitPool::WaitResponse(INT32 nWaitKeyID, UINT32 nWaitTime, UINT32 &nRecvSizeOut)
    {
        TMsgEvent* pMsgEvent = NULL;

        {
            CREATE_SMART_MUTEX(m_lock);

            MsgEventMap::iterator entity = m_msgEventMap.find(nWaitKeyID);

            if (m_msgEventMap.end() == entity)
            {
                return ERROR_CODE_NO_OBJECT;
            }
            else
            {
                pMsgEvent = entity->second;
            }
        }

        bool bSuccess =false;

        ERROR_CODE resCode = ERROR_CODE_SUCCESS;

        if (pMsgEvent->event.Wait(nWaitTime))
        {
            if (ERROR_CODE_SUCCESS != pMsgEvent->errorCode)
            {
                resCode = pMsgEvent->errorCode;
            }
            else
            {
                nRecvSizeOut = pMsgEvent->nRecvSize;
            }
        }
        else
        {
            resCode = ERROR_CODE_TIMEOUT;
        }

        {
            CREATE_SMART_MUTEX(m_lock);

            m_msgEventMap.erase(nWaitKeyID);
            m_MsgEventPool.Push(pMsgEvent);
        }

        return resCode;
    }

    void CMsgWaitPool::SetResponce(INT32 nWaitKeyID, const void* pData, UINT32 nDataSize)
    {
        CREATE_SMART_MUTEX(m_lock);

        MsgEventMap::iterator entity = m_msgEventMap.find(nWaitKeyID);

        if (entity == m_msgEventMap.end())
        {
            return;
        }

        TMsgEvent* pTMsgEvent = entity->second;

        if (nDataSize > pTMsgEvent->nMaxRecvSize)
        {
            pTMsgEvent->nRecvSize = 0;
            pTMsgEvent->errorCode = ERROR_CODE_RECV_SIZE_BIG;
            pTMsgEvent->event.Set();

            return;
        }

        memcpy(pTMsgEvent->pRecvBuffer, pData, nDataSize);
        pTMsgEvent->nRecvSize = nDataSize;
        entity->second->event.Set();
    }


    CMsgWaitPool::TMsgEvent* CMsgWaitPool::GetMsgEvent(INT32 nWaitKeyID)
    {
        TMsgEvent* pMsgEvent = NULL;

        CREATE_SMART_MUTEX(m_lock);

        MsgEventMap::iterator entity = m_msgEventMap.find(nWaitKeyID);

        if (m_msgEventMap.end() == entity)
        {
            pMsgEvent = m_MsgEventPool.Pop();

            if (!pMsgEvent)
            {
                return NULL;
            }
        }

        pMsgEvent->event.Reset();
        pMsgEvent->nRecvSize = 0;
        pMsgEvent->pRecvBuffer = NULL;

        m_msgEventMap.insert(MsgEventMap::value_type(nWaitKeyID, pMsgEvent));

        return pMsgEvent;
    }
}