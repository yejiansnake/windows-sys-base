#pragma once

#include "DS_Common.h"

namespace EM
{
    class CContentHandler
    {
    public:

        CContentHandler();

        virtual ~CContentHandler();

        bool ProcData(TSocketInfo* pTSocketInfo, const char* pData);

        //////////////////////////////////////////////////////////////////////////

        void ProcUnkownReq(TSocketInfo* pTSocketInfo, const char* pData);

        void ProcKeepAliveReq(TSocketInfo* pTSocketInfo, const char* pData);

        //////////////////////////////////////////////////////////////////////////

    private:

        PCHAR m_pSendBuffer;

    };
}