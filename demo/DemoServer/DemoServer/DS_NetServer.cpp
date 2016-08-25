#include "DS_NetServer.h"
#include "DS_Config.h"
#include "DS_WorkThread.h"
#include "DS_StatisticsThread.h"

namespace EM
{
    CNetServer::CNetServer()
    {

    }

    CNetServer::~CNetServer()
    {
        this->Stop();
    }

    bool CNetServer::Start()
    {
        THWCpuInfo aCpuInfo = {0};

        CHWCpu::GetCpuInfo(aCpuInfo);

        if (ERROR_CODE_SUCCESS != m_server.Start(this,
            CSocketAddr::ToDwordIP(CConfig::Instance().GetLocalIP()),
            CConfig::Instance().GetPort(),
            CConfig::Instance().GetMaxConnectCount(),
            aCpuInfo.nLogicalProcessorCount,
            CConfig::Instance().GetRecvBufferSize(),
            CConfig::Instance().GetSendBufferSize(),
            CConfig::Instance().GetSendBufferCount(),
            CTcpServer::KEEP_ALIVE_TYPE_DATA,
            CConfig::Instance().GetKeeAliveTimeSecond() * 1000))
        {
            return false;
        }

        return true;
    }

    void CNetServer::Stop()
    {
        m_server.Stop();
    }

    void CNetServer::Send(
        UINT32 nSocketID,
        const void* pData,
        UINT32 nDataSize)
    {
        m_server.Send(nSocketID, pData, nDataSize);

        CStatisticsThread::Instance().Increment(CStatisticsThread::ESS_SEND_COUNT);
    }

    void CNetServer::CloseSocket(UINT32 nSocketID)
    {
        m_server.CloseSocket(nSocketID);
    }

    UINT32 CNetServer::GetSocketCount()
    {
        return m_server.GetSocketCount();
    }

    UINT32 CNetServer::GetHeadSize()
    {
        return sizeof(DS_HEAD);
    }

    UINT32 CNetServer::GetMsgSize(const char* pData, UINT32 nDataSize)
    {
        PDS_HEAD pHead = (PDS_HEAD)pData;

        return pHead->nSize;
    }

    bool CNetServer::OnAccepted(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort)
    {
        char szIP[50] = {0};

        CSocketAddr::ToStringIP(szIP, 50, nIP);

        LOG_WRITE(LOG_KEY_CLIENT, CLog::LOG_LEVEL_INFO, "CNetServer::OnAccepted  SocketID:%lu, IP:%s, Post:%hu", nSocketID, szIP, nPort);

        return true;
    }

    void CNetServer::OnAcceptedError(CTcpServer* pCTcpServer, UINT32 nIP, UINT16 nPort, ERROR_CODE errorCode)
    {
        char szIP[50] = {0};

        CSocketAddr::ToStringIP(szIP, 50, nIP);

        LOG_WRITE(LOG_KEY_CLIENT, CLog::LOG_LEVEL_ERROR, "CNetServer::OnAcceptedError IP:%s, Post:%hu, ErrocCode:%d", szIP, nPort, (int)errorCode);
    }

    void CNetServer::OnReceived(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, const char* pData, UINT nDataSize)
    {
        CStatisticsThread::Instance().Increment(CStatisticsThread::ESS_RECV_COUNT);

        CWorkThread::Instance().AddData(nSocketID, nIP, nPort, pData, nDataSize);
    }

    void CNetServer::OnClosed(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, ERROR_CODE errorCode)
    {
        char szIP[50] = {0};

        CSocketAddr::ToStringIP(szIP, 50, nIP);

        LOG_WRITE(LOG_KEY_CLIENT, CLog::LOG_LEVEL_INFO, "CNetServer::OnClosed SocketID:%lu, IP:%s, Post:%hu, ErrocCode:%d", nSocketID, szIP, nPort, (int)errorCode);
    }
}