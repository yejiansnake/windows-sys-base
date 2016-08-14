#pragma once

/*
该类负责组织与文件管理服务的协议包并调用NetClient发送到服务，等到接收返回信息时再处理返回给前端业务
*/

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //非线程安全，同一个WaitKeyID需要保证在同一个线程中使用

    class CMsgWaitPool
    {
    public:

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,             //成功
            ERROR_CODE_PARAM,                   //参数错误
            ERROR_CODE_OBJECT_EMPTY,            //没有可分配的对象
            ERROR_CODE_NO_OBJECT,               //没有该等待对象
            ERROR_CODE_TIMEOUT,				    //超时
            ERROR_CODE_RECV_SIZE_BIG,			//接收的数据过大
            ERROR_CODE_SYSTEM,                  //内部错误
        };

        CMsgWaitPool();

        virtual ~CMsgWaitPool();

        ERROR_CODE Init(UINT32 nWaitObjectCount);
        
        ERROR_CODE SubmitRequest(char* recvBuffer, UINT32 nMaxRecvSize, INT32& nWaitKeyIDOut);

        void CancelWait(INT32 nWaitKeyID);

        ERROR_CODE WaitResponse(INT32 nWaitKeyID, UINT32 nWaitTime, UINT32 &nRecvSizeOut);
       
        void SetResponce(INT32 nWaitKeyID, const void* pData, UINT32 nDataSize);

    protected:

    private:

        struct TMsgEvent 
        {
            TMsgEvent()
            {
                event.Reset();
                pRecvBuffer = NULL;
                nRecvSize = 0;
                nMaxRecvSize = 0;
                errorCode = ERROR_CODE_SUCCESS;
            }

            CEvent event;
            char* pRecvBuffer;
            UINT32 nRecvSize;
            UINT32 nMaxRecvSize;
            ERROR_CODE errorCode;
        };

        //////////////////////////////////////////////////////////////////////////

        TMsgEvent* GetMsgEvent(INT32 nWaitKeyID);

        //////////////////////////////////////////////////////////////////////////

        INT32 m_lKeyValue;

        CThreadMutex m_lock;

        typedef map<INT32, TMsgEvent*> MsgEventMap;

        MsgEventMap m_msgEventMap;

        typedef CPool<TMsgEvent> MsgEventPool;

        MsgEventPool m_MsgEventPool;
    };
}
