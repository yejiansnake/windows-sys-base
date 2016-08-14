#include "../SysBase_Interface.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CTcpClientImp

    class CTcpClientImp
    {
    public:

        CTcpClientImp(CTcpClient* pCTcpClient);

        virtual ~CTcpClientImp();

        CTcpClient::ERROR_CODE Start(
            ITcpClientHandler* pHandler,
            UINT32 nRecvBufferSize,
            UINT32 nSendBufferSize,
            UINT32 nSendBufferCount,
            UINT32 nIntervalOnTime = 1000,
            bool bSysKeepAlive = false,
            UINT32 nIntervalTime = 0,
            UINT32 nConnectIntervalTime = 1);

        void Stop();

        CTcpClient::ERROR_CODE Send(
            const void* pData, 
            UINT32 nDataSize);

        void Abort();

        bool IsConnected();

    protected:

        static DWORD WINAPI ThreadProc(LPVOID lpParam);

        void ProcThread();

        bool CreateSocket();

        bool ConnectSrv(UINT32 nIP, UINT16 nPort);

        bool RecvAllData();

        bool SendAllData();

        void ProcOnTime();

    private:

        struct TSendInfo 
        {
            UINT32 nSocketIndex;

            UINT32 nSendSize;
        };

        CTcpClient* m_pCTcpClient;

        bool m_bStop;

        SOCKET m_socket;

        UINT32 m_nSocketIndex;

        CThreadMutex m_CThreadMutex;

        ITcpClientHandler* m_pITcpClientHandler;

        HANDLE m_hThread;	

        char* m_recvBuffer;
        UINT32 m_nHasRecvSize;
        UINT32 m_nRecvBufferSize;

        UINT32 m_nSendBufferSize;

        CBufferQueue m_SendBufferQueue;

        bool m_bClose;

        bool m_bConnected;

        UINT32 m_nMsgHeadSize;

        UINT32 m_nIntervalOnTime;

        UINT32 m_nLastTickCount;

        bool m_bSysKeepAlive;

        UINT32 m_nIntervalTime;

        UINT32 m_nConnectIntervalTime;
    };

    //////////////////////////////////////////////////////////////////////////
    //CTcpClientImp

    CTcpClientImp::CTcpClientImp(CTcpClient* pCTcpClient)
    {
        m_pCTcpClient = pCTcpClient;

        m_bStop = true;

        m_socket = INVALID_SOCKET;

        m_pITcpClientHandler = NULL;

        m_hThread = NULL;

        m_nRecvBufferSize = 0;

        m_nSocketIndex = 0;

        m_nSendBufferSize = 0;

        m_bClose = false;

        m_bConnected = false;

        m_nMsgHeadSize = 0;

        m_nIntervalOnTime = 0;

        m_nLastTickCount = 0;

        m_bSysKeepAlive = false;

        m_nIntervalTime = 0;

        m_recvBuffer = NULL;

        m_nHasRecvSize = 0;

        m_nConnectIntervalTime = 1;
    }

    CTcpClientImp::~CTcpClientImp()
    {
        this->Stop();
    }

    CTcpClient::ERROR_CODE CTcpClientImp::Start(
        ITcpClientHandler* pHandler,
        UINT32 nRecvBufferSize,
        UINT32 nSendBufferSize,
        UINT32 nSendBufferCount,
        UINT32 nIntervalOnTime,
        bool bSysKeepAlive,
        UINT32 nIntervalTime,
        UINT32 nConnectIntervalTime)
    {
        if (!m_bStop)
        {
            return CTcpClient::ERROR_CODE_START;
        }

        if (!pHandler
            || 0 == nRecvBufferSize
            || 0 == nSendBufferSize
            || 0 == nSendBufferCount
            || 0 == nIntervalOnTime)
        {
            return CTcpClient::ERROR_CODE_PARAM;
        }

        if (bSysKeepAlive && 0 == nIntervalTime)
        {
            return CTcpClient::ERROR_CODE_PARAM;
        }

        if (!CSocketLoader::InitSocket())
        {
            return CTcpClient::ERROR_CODE_SYSTEM;
        }

        //////////////////////////////////////////////////////////////////////////

        m_bStop = false;

        m_bClose = false;

        m_bConnected = false;

        m_socket = INVALID_SOCKET;

        m_pITcpClientHandler = pHandler;

        m_nRecvBufferSize = nRecvBufferSize;

        m_nSendBufferSize = nSendBufferSize;

        m_nMsgHeadSize = m_pITcpClientHandler->GetHeadSize();

        m_nIntervalOnTime = nIntervalOnTime;

        m_bSysKeepAlive = bSysKeepAlive;

        m_nIntervalTime = nIntervalTime;

        m_nHasRecvSize = 0;

        m_nLastTickCount = 0;

        if (nConnectIntervalTime > 0)
        {
            m_nConnectIntervalTime = nConnectIntervalTime;
        }

        //////////////////////////////////////////////////////////////////////////
        
        m_recvBuffer = new(std::nothrow) char[m_nRecvBufferSize];

        if (!m_recvBuffer)
        {
            return CTcpClient::ERROR_CODE_SYSTEM;
        }

        //////////////////////////////////////////////////////////////////////////

        if (CBufferQueue::ERROR_CODE_SUCCESS != m_SendBufferQueue.Init(sizeof(TSendInfo) +  m_nSendBufferSize,
            nSendBufferCount, false))
        {
            this->Stop();

            return CTcpClient::ERROR_CODE_SYSTEM;
        }

        //////////////////////////////////////////////////////////////////////////

        m_hThread = CreateThread(NULL, 0, CTcpClientImp::ThreadProc, this, 0, NULL);

        if (!m_hThread)
        {
            this->Stop();

            return CTcpClient::ERROR_CODE_SYSTEM;
        }
        //////////////////////////////////////////////////////////////////////////

        return CTcpClient::ERROR_CODE_SUCCESS;
    }

    void CTcpClientImp::Stop()
    {
        if (m_bStop)
        {
            return;
        }

        m_bStop = true;

        if (m_hThread)
        {
            if (WAIT_TIMEOUT == WaitForSingleObject(m_hThread, 3000))
            {
                TerminateThread(m_hThread, 0);
            }

            CloseHandle(m_hThread);

            m_hThread = NULL;
        }

        if (INVALID_SOCKET != m_socket)
        {
            closesocket(m_socket);

            m_socket = INVALID_SOCKET;
        }

        m_SendBufferQueue.Release();

        if (m_recvBuffer)
        {
            delete [] m_recvBuffer;

            m_recvBuffer = NULL;
        }
    }

    CTcpClient::ERROR_CODE CTcpClientImp::Send(
        const void* pData, 
        UINT32 nDataSize)
    {
        if (m_bStop)
        {
            return CTcpClient::ERROR_CODE_STOP;
        }

        if (!pData || 0 == nDataSize)
        {
            return CTcpClient::ERROR_CODE_PARAM;
        }

        if (nDataSize > m_nSendBufferSize)
        {
            return CTcpClient::ERROR_CODE_OVER_SIZE;
        }

        if (!m_bConnected || m_bClose)
        {
            return CTcpClient::ERROR_CDOE_NO_CONNECT;
        }

        CBufferWrite aCBufferWrite;

        if (CBufferQueue::ERROR_CODE_SUCCESS != m_SendBufferQueue.GetBufferWrite(aCBufferWrite))
        {
            return CTcpClient::ERROR_CODE_QUEUE_FULL;
        }

        char* sendBuffer = aCBufferWrite.GetData();

        TSendInfo* pTSendInfo = (TSendInfo*)sendBuffer;

        pTSendInfo->nSocketIndex = m_nSocketIndex;

        pTSendInfo->nSendSize = nDataSize;

        memcpy(sendBuffer + sizeof(TSendInfo), pData, nDataSize);

        aCBufferWrite.SetDataSize(sizeof(TSendInfo) + nDataSize);

        aCBufferWrite.Push();

        return CTcpClient::ERROR_CODE_SUCCESS;
    }

    void CTcpClientImp::Abort()
    {
        if (m_bStop)
        {
            return;
        }

        m_bClose = true;
    }

    bool CTcpClientImp::IsConnected()
    {
		return m_bConnected;
    }

    DWORD WINAPI CTcpClientImp::ThreadProc(LPVOID lpParam)
    {
        CTcpClientImp* pCTcpClientImp = (CTcpClientImp*)lpParam;

        pCTcpClientImp->ProcThread();

        return 0;
    }

    void CTcpClientImp::ProcThread()
    {
        m_bClose = false;

        while (!m_bStop)
        {
            //////////////////////////////////////////////////////////////////////////
            //是否关闭当前

            if (m_bClose)
            {
                m_bConnected = false;

                if (INVALID_SOCKET != m_socket)
                {
                    closesocket(m_socket);

                    m_socket = INVALID_SOCKET;
                }

                m_nHasRecvSize = 0;

                m_bClose = false;
            }

            //////////////////////////////////////////////////////////////////////////
            //创建连接

            if (INVALID_SOCKET == m_socket)
            {
                if (!this->CreateSocket())
                {
                    Sleep(1);

                    continue;
                }
            }

            //////////////////////////////////////////////////////////////////////////
            //连接服务器

            if (!m_bConnected)
            {
                ///////////////////////////////////////////////////////////////////////////
                //获取连接地址

                UINT32 nIP = 0;
                UINT16 nPort = 0;

                if (m_pITcpClientHandler->OnConnecting(m_pCTcpClient, nIP, nPort))
                {
                    if (this->ConnectSrv(nIP, nPort))
                    {
                        m_pITcpClientHandler->OnConnected(m_pCTcpClient, nIP, nPort);
                    }
                    else
                    {
                        m_pITcpClientHandler->OnConnectFailed(m_pCTcpClient, nIP, nPort);

                        m_bClose = true;

                        Sleep(m_nConnectIntervalTime);
                        continue;
                    }
                }
                else
                {
                    Sleep(m_nConnectIntervalTime);
                    continue;
                }
            }

            //////////////////////////////////////////////////////////////////////////
            //处理接收

            if (!this->RecvAllData())
            {
                m_bClose = true;

                continue;
            }

            //////////////////////////////////////////////////////////////////////////
            //处理发送

            if (!this->SendAllData())
            {
                m_bClose = true;

                continue;
            }

            //////////////////////////////////////////////////////////////////////////
            //处理定时器

            this->ProcOnTime();
        }
    }

    bool CTcpClientImp::CreateSocket()
    {
        if (INVALID_SOCKET != m_socket)
        {
            closesocket(m_socket);

            m_socket = INVALID_SOCKET;
        }

        m_bConnected = false;
        m_nHasRecvSize = 0;

        m_socket =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (INVALID_SOCKET == m_socket)
        {
            return false;
        }

        //设置为非阻塞模式
        // If iMode = 0, blocking is enabled; 
        // If iMode != 0, non-blocking mode is enabled.				
        int iMode = 1;
        if (SOCKET_ERROR == ioctlsocket(m_socket, FIONBIO, (u_long FAR*)&iMode))
        {
            closesocket(m_socket);

            m_socket = INVALID_SOCKET;

            return false;
        }

        if (m_bSysKeepAlive)
        {
            BOOL bKeepAlive = TRUE; 

            if (SOCKET_ERROR == setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive)))
            {
                closesocket(m_socket);

                m_socket = INVALID_SOCKET;

                return false;
            }

            //设置KeepAlive检测时间和次数 
            SYSBASE_TCP_KEEPALIVE inKeepAlive = {0}; //输入参数 
            unsigned long ulInLen = sizeof(SYSBASE_TCP_KEEPALIVE); 

            SYSBASE_TCP_KEEPALIVE outKeepAlive = {0}; //输出参数 
            unsigned long ulOutLen = sizeof(SYSBASE_TCP_KEEPALIVE); 

            unsigned long ulBytesReturn = 0; 

            inKeepAlive.onoff = 1; 
            inKeepAlive.keepaliveinterval = m_nIntervalTime; //两次KeepAlive探测间的时间间隔 
            inKeepAlive.keepalivetime = m_nIntervalTime; //开始首次KeepAlive探测前的TCP空闭时间 

            if (SOCKET_ERROR == WSAIoctl(m_socket, 
                SIO_KEEPALIVE_VALS, 
                (LPVOID)&inKeepAlive, 
                ulInLen, 
                (LPVOID)&outKeepAlive, 
                ulOutLen, 
                &ulBytesReturn, 
                NULL, 
                NULL))
            {
                closesocket(m_socket);

                m_socket = INVALID_SOCKET;

                return false;
            }
        }

        m_nLastTickCount = GetTickCount();

        m_nSocketIndex += m_nSocketIndex;

        return true;
    }

    bool CTcpClientImp::ConnectSrv(UINT32 nIP, UINT16 nPort)
    {
        if (INVALID_SOCKET == m_socket)
        {
            return false;
        }

        if (0 == nPort)
        {
            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //执行连接操作

        SOCKADDR_IN sockAddr = {0};
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = nIP;
        sockAddr.sin_port = htons(nPort);

        if (SOCKET_ERROR == connect(m_socket, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR_IN)))
        {
            DWORD dwError = WSAGetLastError();

            if (WSAEWOULDBLOCK != dwError)
            {
                return false;
            }
        }

        //////////////////////////////////////////////////////////////////////////
        //等待连接是否成功

        fd_set fdWrite = {0};
        fd_set fdException = {0};

        FD_SET(m_socket, &fdWrite);
        FD_SET(m_socket, &fdException);

        timeval timeout = {0};
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        switch (select((int)m_socket + 1, NULL, &fdWrite, &fdException, &timeout))
        {
        case 0:
            {
                return false;
            }
            break;
        case SOCKET_ERROR :
            {
                return false;
            }
            break;
        default:
            break;
        }

        //////////////////////////////////////////////////////////////////////////

        if (!FD_ISSET(m_socket, &fdWrite))
        {
            return false;
        }

        m_bConnected = true;

        //////////////////////////////////////////////////////////////////////////

        return true;
    }

    bool CTcpClientImp::RecvAllData()
    {
        if (INVALID_SOCKET == m_socket)
        {
            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //判断是否有数据接收

        fd_set fdRead = {0};
        FD_SET(m_socket, &fdRead);

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;

        switch (select(0, &fdRead, NULL, NULL, &timeout))
        {
        case 0:
            {
                return true;
            }
            break;
        case SOCKET_ERROR :
            {
                m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_SYSTEM_CLOSE);

                return false;
            }
            break;
        default:
            break;
        }

        if (!FD_ISSET(m_socket, &fdRead))   //没有数据可读，则不进行接收处理
        {
            return true;
        }

        //////////////////////////////////////////////////////////////////////////
        //接收数据

        while (!m_bStop && !m_bClose)
        {
            int nRecvSize = recv(m_socket, m_recvBuffer + m_nHasRecvSize, 
                (int)(m_nRecvBufferSize - m_nHasRecvSize), 0);

            if (SOCKET_ERROR == nRecvSize)
            {
                DWORD dwError = WSAGetLastError();

                if (WSAEWOULDBLOCK != dwError)
                {
                    m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_SYSTEM_CLOSE);

                    return false;
                }

                return true;
            }

            if (0 >= nRecvSize)
            {
                //远程连接关闭

                m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_REMOTE_CLOSE);

                return false;
            }

            m_nHasRecvSize += (UINT32)nRecvSize;

            if (m_nHasRecvSize > m_nRecvBufferSize)
            {
                m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_PROTOCOL);

                return false;
            }

            while (m_nHasRecvSize > m_nMsgHeadSize)
            {
                UINT32 nMsgSize = m_pITcpClientHandler->GetMsgSize(m_recvBuffer, m_nHasRecvSize);

                if (nMsgSize > m_nRecvBufferSize || nMsgSize < m_nMsgHeadSize)
                {
                    m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_PROTOCOL);

                    return false;
                }

                if (m_nHasRecvSize >= nMsgSize)
                {
                    m_pITcpClientHandler->OnReceived(m_pCTcpClient, m_recvBuffer, nMsgSize);

                    memmove(m_recvBuffer, m_recvBuffer + nMsgSize, m_nHasRecvSize - nMsgSize);
                    m_nHasRecvSize -= nMsgSize;
                }
                else
                {
                    break;
                }
            }
        }

        return true;
    }

    bool CTcpClientImp::SendAllData()
    {
        if(INVALID_SOCKET == m_socket)
        {
            return false;
        }

        if (m_SendBufferQueue.Empty())
        {
            return true;
        }

        //////////////////////////////////////////////////////////////////////////
        //判断是否可以发送数据

        fd_set fdWrite = {0};
        fd_set fdException = {0};

        FD_SET(m_socket, &fdWrite);
        FD_SET(m_socket, &fdException);

        timeval timeout = {0};

        switch (select(0, NULL, &fdWrite, &fdException, &timeout))
        {
        case 0:
            {
                return true;
            }
            break;
        case SOCKET_ERROR :
            {
                m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_SYSTEM_CLOSE);

                return false;
            }
            break;
        default:
            break;
        }

        if (!FD_ISSET(m_socket, &fdWrite))
        {
            return true;
        }

        if (FD_ISSET(m_socket, &fdException))
        {
            return false;
        }

        //////////////////////////////////////////////////////////////////////////

        int nSendIndex = 0;
        int nSendCount = 200;

        for (nSendIndex = 0; nSendIndex < nSendCount; ++nSendIndex)
        {
            if (m_bStop || m_bClose)
            {
                m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_SYSTEM_CLOSE);

                return false;
            }

            CBufferRead aCBufferRead;

            if (CBufferQueue::ERROR_CODE_SUCCESS != m_SendBufferQueue.Pop(aCBufferRead, 0))
            {
                return true;
            }

            TSendInfo* pTSendInfo = (TSendInfo*)aCBufferRead.GetData();

            //判断是否为同一个连接的发送内容
            if (pTSendInfo->nSocketIndex != m_nSocketIndex)
            {
                continue;
            }

            int nHasSendSize = 0;
            int nTotalSendSize = (int)pTSendInfo->nSendSize;

            const char* sendBuffer = aCBufferRead.GetData(sizeof(TSendInfo));

            while (nHasSendSize < nTotalSendSize)
            {
                if (m_bStop || m_bClose)
                {
                    m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_SYSTEM_CLOSE);

                    return false;
                }

                int nSendSize = send(m_socket, sendBuffer + nHasSendSize, 
                    nTotalSendSize - nHasSendSize, 0);

                if (SOCKET_ERROR == nSendSize)
                {
                    DWORD dwError = WSAGetLastError();

                    if (WSAEWOULDBLOCK == dwError)
                    {
                        //发送对象瞬间发送的包过多，所以无法马上发送，继续发送该包
                        continue;
                    }
                    else
                    {
                        m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_SYSTEM_CLOSE);
                        return false;
                    }
                }
                
                //等于0认为是连接已经断开
                if (0 == nSendSize)
                {
                    m_pITcpClientHandler->OnClosed(m_pCTcpClient, ITcpClientHandler::ERROR_CODE_REMOTE_CLOSE);

                    return false;
                }

                nHasSendSize += nSendSize;
            }
        }

        return true;
    }

    void CTcpClientImp::ProcOnTime()
    {
        if (0 == m_nLastTickCount)
        {
            return;
        }

        UINT32 nCurTickCount = GetTickCount();

        if (nCurTickCount <= m_nLastTickCount)
        {
            return;
        }

        if ((nCurTickCount - m_nLastTickCount) <= m_nIntervalOnTime)
        {
            return;
        }

        m_pITcpClientHandler->OnTime(m_pCTcpClient);

        m_nLastTickCount = GetTickCount();
    }

    //////////////////////////////////////////////////////////////////////////
    //CTcpClient

    CTcpClient::CTcpClient()
    {
        m_pImp = new(std::nothrow) CTcpClientImp(this);
    }

    CTcpClient::~CTcpClient()
    {
        if (m_pImp)
        {
            CTcpClientImp* pCTcpClientImp = (CTcpClientImp*)m_pImp;

            delete pCTcpClientImp;

            m_pImp = NULL;
        }
    }

    CTcpClient::ERROR_CODE CTcpClient::Start(
        ITcpClientHandler* pHandler,
        UINT32 nRecvBufferSize,
        UINT32 nSendBufferSize,
        UINT32 nSendBufferCount,
        UINT32 nIntervalOnTime,
        bool bSysKeepAlive,
        UINT32 nIntervalTime,
        UINT32 nConnectIntervalTime)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CTcpClientImp* pCTcpClientImp = (CTcpClientImp*)m_pImp;

        return pCTcpClientImp->Start(
            pHandler, 
            nRecvBufferSize, 
            nSendBufferSize, 
            nSendBufferCount, 
            nIntervalOnTime, 
            bSysKeepAlive,
            nIntervalTime,
            nConnectIntervalTime);
    }

    void CTcpClient::Stop()
    {
        if (!m_pImp)
        {
            return;
        }

        CTcpClientImp* pCTcpClientImp = (CTcpClientImp*)m_pImp;

        pCTcpClientImp->Stop();
    }

    CTcpClient::ERROR_CODE CTcpClient::Send(
        const void* pData, 
        UINT32 nDataSize)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CTcpClientImp* pCTcpClientImp = (CTcpClientImp*)m_pImp;

        return pCTcpClientImp->Send(pData, nDataSize);
    }

    void CTcpClient::Abort()
    {
        if (!m_pImp)
        {
            return;
        }

        CTcpClientImp* pCTcpClientImp = (CTcpClientImp*)m_pImp;

        pCTcpClientImp->Abort();
    }

    bool CTcpClient:: IsConnected()
    {
        if (!m_pImp)
        {
            return false;
        }

        CTcpClientImp* pCTcpClientImp = (CTcpClientImp*)m_pImp;

        return pCTcpClientImp->IsConnected();
    }
}