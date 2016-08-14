#include "../SysBase_Interface.h"

namespace SysBase
{
    CTcpSocket::CTcpSocket()
    {
        m_bConnected = false;
    }

    CTcpSocket::~CTcpSocket()
    {

    }

    CTcpSocket::ERROR_CODE CTcpSocket::Connect(const char* szIP, UINT16 nPort, UINT32 nWaitTime)
    {
        if (!szIP || 0 == nPort)
        {
            return CTcpSocket::ERROR_CODE_PARAM;
        }

        return this->Connect(CSocketAddr::ToDwordIP(szIP), nPort, nWaitTime);
    }

    CTcpSocket::ERROR_CODE CTcpSocket::Connect(UINT32 nIP, UINT16 nPort, UINT32 nWaitTime)
    {
        if (0 == nPort)
        {
            return CTcpSocket::ERROR_CODE_PARAM;
        }

        if (INVALID_SOCKET != m_Socket)
        {
            return CTcpSocket::ERROR_CODE_HAS_CONNECT;
        }

        //////////////////////////////////////////////////////////////////////////
        //执行连接操作

        m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (INVALID_SOCKET == m_Socket)
        {
            return CTcpSocket::ERROR_CODE_SYSTEM;
        }

        //设置为非阻塞模式
        // If iMode = 0, blocking is enabled; 
        // If iMode != 0, non-blocking mode is enabled.				
        int iMode = 1;
        if (SOCKET_ERROR == ioctlsocket(m_Socket, FIONBIO, (u_long FAR*)&iMode))
        {
            closesocket(m_Socket);

            m_Socket = INVALID_SOCKET;

            return CTcpSocket::ERROR_CODE_SYSTEM;
        }

        //////////////////////////////////////////////////////////////////////////

        SOCKADDR_IN sockAddr = {0};
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = nIP;
        sockAddr.sin_port = htons(nPort);

        if (SOCKET_ERROR == connect(m_Socket, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR_IN)))
        {
            DWORD dwError = WSAGetLastError();

            if (WSAEWOULDBLOCK != dwError)
            {
                closesocket(m_Socket);

                m_Socket = INVALID_SOCKET;

                return CTcpSocket::ERROR_CODE_SYSTEM;
            }
        }

        //////////////////////////////////////////////////////////////////////////
        //等待连接是否成功

        fd_set fdWrite = {0};
        fd_set fdException = {0};

        FD_SET(m_Socket, &fdWrite);
        FD_SET(m_Socket, &fdException);

        timeval timeout = {0};
        timeout.tv_sec = nWaitTime / 1000;
        timeout.tv_usec = nWaitTime % 1000 * 1000;

        switch (select((int)m_Socket + 1, NULL, &fdWrite, &fdException, &timeout))
        {
        case 0:
            {
                closesocket(m_Socket);

                m_Socket = INVALID_SOCKET;

                return CTcpSocket::ERROR_CODE_CONNECT_FAILED;
            }
            break;
        case SOCKET_ERROR :
            {
                closesocket(m_Socket);

                m_Socket = INVALID_SOCKET;

                return CTcpSocket::ERROR_CODE_SYSTEM;
            }
            break;
        default:
            break;
        }

        //////////////////////////////////////////////////////////////////////////

        if (!FD_ISSET(m_Socket, &fdWrite))
        {
            closesocket(m_Socket);

            m_Socket = INVALID_SOCKET;

            return CTcpSocket::ERROR_CODE_CONNECT_FAILED;
        }

        m_bConnected = true;

        return CTcpSocket::ERROR_CODE_SUCCESS;
    }

    void CTcpSocket::Disconnect()
    {
        m_bConnected = false;

        CSocket::Stop();

        CSocket::Close();
    }

    bool CTcpSocket::IsConnected()
    {
        if (m_bConnected)
        {
            return true;
        }

        return false;
    }

    CTcpSocket::ERROR_CODE CTcpSocket::Send(const void* pData, UINT32 nSize)
    {
        if (!pData || 0 == nSize)
        {
            return CTcpSocket::ERROR_CODE_PARAM;
        }

        if (INVALID_SOCKET == m_Socket)
        {
            return CTcpSocket::ERROR_CODE_NO_CONNECT;
        }

        const char* pBuf = (const char*)pData;

        int nHasSendSize = 0;
        int nNeedSendSize = nSize;

        while (nNeedSendSize > 0)
        {
            int nSendSize = send(m_Socket, pBuf + nHasSendSize, nNeedSendSize, 0);

            if (SOCKET_ERROR == nSendSize)
            {
                DWORD dwError = WSAGetLastError();

                if (WSAEWOULDBLOCK != dwError)
                {
                    CTcpSocket::ERROR_CODE errorCode = CTcpSocket::ERROR_CODE_SYSTEM;

                    switch (dwError)
                    {
                    case WSAECONNABORTED:
                    case WSAECONNRESET:
                        {
                            return CTcpSocket::ERROR_CODE_REMOTE_CLOSE;
                        }
                        break;
                    
                    }

                    return errorCode;

                    break;
                }
            }
            else if (nSendSize > 0)
            {
                nHasSendSize += nSendSize;
                nNeedSendSize -= nSendSize;
            }
            else
            {
                return CTcpSocket::ERROR_CODE_SYSTEM;
            }            
        }

        return CTcpSocket::ERROR_CODE_SUCCESS;
    }

    CTcpSocket::ERROR_CODE CTcpSocket::Recv(void* pBuffer, UINT32 nBufferSize, UINT32& nRecvSzie, UINT32 nWaitTime)
    {
        if (!pBuffer || 0 == nBufferSize)
        {
            return CTcpSocket::ERROR_CODE_PARAM;
        }

        if (INVALID_SOCKET == m_Socket)
        {
            return CTcpSocket::ERROR_CODE_NO_CONNECT;
        }

        if (!this->WaitRecv(nWaitTime))
        {
            return CTcpSocket::ERROR_CODE_TIMEOUT;
        }

        int nRecvLen = recv(m_Socket, (char*)pBuffer, (int)nBufferSize, 0);

        if (SOCKET_ERROR == nRecvLen)
        {
            DWORD dwError = WSAGetLastError();

            if (WSAEWOULDBLOCK != dwError)
            {
                return CTcpSocket::ERROR_CODE_HAS_CLOSE;
            }

            else
            {
                return CTcpSocket::ERROR_CODE_RECV_NO_DATA;
            }
        }
        else if (nRecvLen = 0)
        {
            return CTcpSocket::ERROR_CODE_DISCONNECT;
        }

        nRecvSzie = nRecvLen;

        return CTcpSocket::ERROR_CODE_SUCCESS;
    }

    CTcpSocket::ERROR_CODE CTcpSocket::RecvEx(void* pBuffer, UINT32 nBufferSize, UINT32& nRecvSzie, UINT32 nWaitTime)
    {
        if (!pBuffer || nBufferSize < sizeof(UINT32))
        {
            return CTcpSocket::ERROR_CODE_PARAM;
        }

        if (INVALID_SOCKET == m_Socket)
        {
            return CTcpSocket::ERROR_CODE_NO_CONNECT;
        }

        UINT32 nNeedWaitTime = nWaitTime;
        DWORD dwTickCount = GetTickCount();

        UINT32 nHasRecvSize = 0;
        int nCurNeedRecvSize = 0;
        UINT32 nMsgSize = 0;

        while (true)
        {
            if (!this->WaitRecv(nNeedWaitTime))
            {
                return CTcpSocket::ERROR_CODE_TIMEOUT;
            }

            //////////////////////////////////////////////////////////////////////////

            if (nHasRecvSize < sizeof(UINT32))
            {
                nCurNeedRecvSize = sizeof(UINT32) - nHasRecvSize;
            }
            else
            {
                nCurNeedRecvSize = nMsgSize - nHasRecvSize;
            }

            int nRecvLen = recv(m_Socket, (char*)pBuffer + nHasRecvSize, nCurNeedRecvSize, 0);

            if (SOCKET_ERROR == nRecvLen)
            {
                DWORD dwError = WSAGetLastError();

                if (WSAEWOULDBLOCK != dwError)
                {
                    return CTcpSocket::ERROR_CODE_HAS_CLOSE;
                }

                else
                {
                    return CTcpSocket::ERROR_CODE_RECV_NO_DATA;
                }
            }
            else if (0 == nRecvLen)
            {
                return CTcpSocket::ERROR_CODE_DISCONNECT;
            }

            //到这里肯定是 nRecvLen > 0

            nHasRecvSize += nRecvLen;

            if (0 == nMsgSize)
            {
                if (nHasRecvSize >= sizeof(UINT32))
                {
                    nMsgSize = *((UINT32*)pBuffer);

                    if (nMsgSize > nBufferSize || nMsgSize < sizeof(UINT32))
                    {
                        return CTcpSocket::ERROR_CODE_PROTOCOL;
                    }
                }
            }
            else if (nMsgSize == nHasRecvSize)
            {
                break;
            }

            //////////////////////////////////////////////////////////////////////////

            DWORD dwIntervalTime = GetTickCount() - dwTickCount;

            if (nNeedWaitTime > dwIntervalTime)
            {
                nNeedWaitTime -= dwIntervalTime;
            }
            else
            {
                nNeedWaitTime = 0;
            }

            dwTickCount = GetTickCount();
            
        }

        return CTcpSocket::ERROR_CODE_SUCCESS; 
    }
}