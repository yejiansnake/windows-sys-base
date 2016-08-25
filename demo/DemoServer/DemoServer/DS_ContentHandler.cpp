#include "DS_ContentHandler.h"
#include "DS_Config.h"

namespace EM
{
    CContentHandler::CContentHandler()
    {
        m_pSendBuffer = new(std::nothrow) CHAR[CConfig::Instance().GetSendBufferSize()];
    }

    CContentHandler::~CContentHandler()
    {

    }

    bool CContentHandler::ProcData(TSocketInfo* pTSocketInfo, const char* pData)
    {
        PDS_HEAD pHaed = (PDS_HEAD)pData;

        switch (pHaed->sCmd)
        {
        case DS_CMD_KEEP_ALIVE:
            {
                this->ProcKeepAliveReq(pTSocketInfo, pData);
            }
            break;
        default:
            {
                this->ProcUnkownReq(pTSocketInfo, pData);
            }
            break;
        }

        return true;
    }
}