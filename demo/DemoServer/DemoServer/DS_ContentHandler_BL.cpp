#include "DS_ContentHandler.h"
#include "DS_NetServer.h"
#include "DS_Config.h"

namespace EM
{
    void CContentHandler::ProcUnkownReq(TSocketInfo* pTSocketInfo, const char* pData)
    {
        PDS_HEAD pHaed = (PDS_HEAD)pData;

        LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_INFO, "CContentHandler::ProcUnkownReq cmd:%lu, size:%lu", pHaed->sCmd, pHaed->nSize);
    }

    void CContentHandler::ProcKeepAliveReq(TSocketInfo* pTSocketInfo, const char* pData)
    {
        char szIP[50] = {0};
        CSocketAddr::ToStringIP(szIP, 50, pTSocketInfo->nIP);

        LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_DEBUG, "CContentHandler::ProcKeepAliveReq SocketID:%lu, IP:%s, Port:%hu", 
            pTSocketInfo->nSocketID, szIP, pTSocketInfo->nPort);

        //这里只是示例将客户端发送的数据原封不动的回发
        CNetServer::Instance().Send(pTSocketInfo->nSocketID, pData, pTSocketInfo->nDataSize);
    }
}