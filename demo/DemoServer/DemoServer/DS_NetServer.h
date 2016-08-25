#pragma once

#include "DS_Common.h"

namespace EM
{
    class CNetServer : public CSingletonT<CNetServer>, public ITcpServerHandler
    {
    public:

        CNetServer();

        virtual ~CNetServer();

        bool Start();

        void Stop();

        void Send(
            UINT32 nSocketID,
            const void* pData,
            UINT32 nDataSize);

        void CloseSocket(UINT32 nSocketID);

        UINT32 GetSocketCount();

    protected:

        virtual UINT32 GetHeadSize();

        virtual UINT32 GetMsgSize(const char* pData, UINT32 nDataSize);

        virtual bool OnAccepted(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort);

        virtual void OnAcceptedError(CTcpServer* pCTcpServer, UINT32 nIP, UINT16 nPort, ERROR_CODE errorCode);

        virtual void OnReceived(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, const char* pData, UINT nDataSize);

        virtual void OnClosed(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, ERROR_CODE errorCode);

    private:

        CTcpServer m_server;
    };
}