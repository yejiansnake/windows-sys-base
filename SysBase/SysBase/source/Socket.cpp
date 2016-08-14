#include "../SysBase_Interface.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//ESocketProtocolType SOCKET的协议枚举

	enum ESocketProtocolType
	{
		PROTOCOLTYPE_TCP,
		PROTOCOLTYPE_UDP
	};
	
	//////////////////////////////////////////////////////////////////////////
	//CSocketAddr
	
	WORD CSocketAddr::ToStringIP(char* szIP, WORD wSize, UINT32 nIP)
	{
		if (!szIP)
		{
			return 0;
		}

		if (wSize > SOCKET_IP_MAX_SIZE)
		{
			return 0;
		}

		unsigned char* ptr = (unsigned char*)&nIP;

		sprintf(szIP, "%u.%u.%u.%u", ptr[0], ptr[1], ptr[2], ptr[3]);

		return (WORD)strlen(szIP);
	}
	
	DWORD CSocketAddr::ToDwordIP(const char* szIP)
	{
		if (!szIP)
		{
			return 0;
		}
		
		return inet_addr(szIP);
	}

	bool CSocketAddr::ToTSockAddr(
		const INET_SOCKET_ADDRESS_STRUCT* pSockAddr, 
		TSockAddr* pTSockAddr)
	{
		if (!pSockAddr)
		{
			return false;
		}

		if (!pTSockAddr)
		{
			return false;
		}

		char szAddress[SOCKET_IP_MAX_SIZE] = {0};
		DWORD dwLength = SOCKET_IP_MAX_SIZE;

		//WSAAddressToString 执行成功时返回 0
		if (WSAAddressToStringA((SOCKADDR* )pSockAddr, 
			(DWORD)INET_SOCKET_ADDRESS_STRUCT_LENGTH, 
			NULL, 
			szAddress, 
			&dwLength))
		{
			return false;
		}

		char* ptr = strrchr(szAddress, ':');

		if (!ptr)
		{
			return false;
		}

		//获取端口信息
		pTSockAddr->uPort = atoi(ptr + 1);

		//获取IP地址信息
		switch(INET_SOCKET_ADDRESS_STRUCT_LENGTH)
		{
		case INET_SOCKET_ADDRESS_STRUCT_LENGTH_IPV4:
			{
				size_t nLength = ptr - szAddress;

				strncpy(pTSockAddr->szIP, szAddress, nLength);

				break;
			}
		case INET_SOCKET_ADDRESS_STRUCT_LENGTH_IPV6:
			{
				size_t nLength = ptr - szAddress -2; 

				strncpy(pTSockAddr->szIP, szAddress, nLength);

				break;
			}
		default:
			return false;
		}

		return true;
	}

	bool CSocketAddr::ToInetSockAddr(
		const TSockAddr* pTSockAddr,
		INET_SOCKET_ADDRESS_STRUCT* pSockAddr)
	{
		if (!pTSockAddr)
		{
			return false; 
		}

		if (!pSockAddr)
		{
			return false; 
		}

		char szAddress[SOCKET_IP_MAX_SIZE] = {0};

		if (pTSockAddr->uPort)
		{   
			sprintf(szAddress, "%s:%d", pTSockAddr->szIP, pTSockAddr->uPort);
		}
		else
		{
			sprintf(szAddress, "%s", pTSockAddr->szIP);
		}

		INT nSockAddrLen = INET_SOCKET_ADDRESS_STRUCT_LENGTH;

		//WSAStringToAddress 执行成功时返回 0
		if (WSAStringToAddressA(szAddress, INET_PF_FAMILY, NULL, 
			(SOCKADDR* )pSockAddr, &nSockAddrLen))
		{
			return false; 
		}

		return true;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//CSocket

	CSocket::CSocket()
	{
		m_bIsClose = false;

		m_Socket = INVALID_SOCKET;
	}

	CSocket::~CSocket()
	{
		if (m_Socket != INVALID_SOCKET)
		{
			closesocket(m_Socket);
		}
	}

	bool CSocket::SetBlock(bool bBlock)
	{
		// If iMode = 0, blocking is enabled; 
		// If iMode != 0, non-blocking mode is enabled.

		int iMode = bBlock ? 0 : 1;
		if (SOCKET_ERROR == ioctlsocket(m_Socket, FIONBIO, (u_long FAR*)&iMode))
		{
			return false;
		}

		return true;
	}

	bool CSocket::WaitRecv(UINT32 nWaitTime)
	{
		fd_set fdRead = {0};
		FD_SET(m_Socket, &fdRead);

		timeval timeout;
		timeout.tv_sec = nWaitTime / 1000;
		timeout.tv_usec = nWaitTime % 1000 * 1000;

		switch (select((int)m_Socket + 1, &fdRead, NULL, NULL, &timeout))
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
			{
				if (FD_ISSET(m_Socket, &fdRead))
				{
					return true;
				}
			}
		}

		return false;
	}
	
	bool CSocket::WaitSend(UINT32 nWaitTime)
	{
		if (INVALID_SOCKET == m_Socket)
		{
			return false;
		}
		
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
				return false;
			}
			break;
		case SOCKET_ERROR :
			{
				return false;
			}
			break;
		default:
			{
				if (FD_ISSET(m_Socket, &fdWrite))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool CSocket::SetSocketOption(
		int level, 
		int optname, 
		const char* optval, 
		int optlen)
	{
		if (SOCKET_ERROR == setsockopt(m_Socket, level, optname, optval, optlen))
		{
			return false;
		}

		return true;
	}

	bool CSocket::GetSocketOption(
		int level, 
		int optname, 
		char* optval, 
		int* optlen)
	{
		if (SOCKET_ERROR == getsockopt(m_Socket, level, optname, optval, optlen))
		{
			return false;
		}

		return true;
	}

	bool CSocket::SetReuseAddr(bool bIsReUse)
	{
		return SetSocketOption(SOL_SOCKET, SO_REUSEADDR, (const char* )&bIsReUse, sizeof(bool));
	}

	bool CSocket::IsReuseAddr()
	{
		bool bReuseaddr = true;
		int nSize = sizeof(bool);

		if (!GetSocketOption(SOL_SOCKET, SO_REUSEADDR, (char* )&bReuseaddr, &nSize))
		{
			return false;
		}

		return bReuseaddr;
	}

	bool CSocket::SetDontLinger(bool bDontLinger)
	{
		return SetSocketOption(SOL_SOCKET, SO_DONTLINGER, (const char*)&bDontLinger, sizeof(bool));
	}

	bool CSocket::IsDontLinger()
	{
		bool bDontLinger = true;
		int nSize = sizeof(bool);

		if (!GetSocketOption(SOL_SOCKET, SO_DONTLINGER, (char* )&bDontLinger, &nSize))
		{
			return false;
		}

		return bDontLinger;
	}

	bool CSocket::SetSendBufferSize(int nBufferSize)
	{
		return SetSocketOption(SOL_SOCKET, SO_SNDBUF, (const char*)&nBufferSize, sizeof(int));
	}

	int CSocket::GetSendBufferSize()
	{
		int nBufferSize = 0;
		int nSize = sizeof(int);

		if (!GetSocketOption(SOL_SOCKET, SO_SNDBUF, (char* )&nBufferSize, &nSize))
		{
			return 0;
		}

		return nBufferSize;
	}

	bool CSocket::SetRecvBufferSzie(int nBufferSize)
	{
		return SetSocketOption(SOL_SOCKET, SO_RCVBUF, (const char*)&nBufferSize, sizeof(int));
	}

	int CSocket::GetRecvBufferSize()
	{
		int nBufferSize = 0;
		int nSize = sizeof(int);

		if (!GetSocketOption(SOL_SOCKET, SO_RCVBUF, (char* )&nBufferSize, &nSize))
		{
			return 0;
		}

		return nBufferSize;
	}

	bool CSocket::SetBroadcast(bool bBroadcast)
	{
		return SetSocketOption(SOL_SOCKET, SO_BROADCAST, (const char*)&bBroadcast, sizeof(bool));
	}

	bool CSocket::IsBroadcast()
	{
		bool bBroadcast = true;
		int nSize = sizeof(bool);

		if (!GetSocketOption(SOL_SOCKET, SO_BROADCAST, (char* )&bBroadcast, &nSize))
		{
			return false;
		}

		return bBroadcast;
	}

	bool CSocket::SetConditionalAccept(bool bConditionalAccept)
	{
		return SetSocketOption(SOL_SOCKET, SO_CONDITIONAL_ACCEPT, 
			(const char*)&bConditionalAccept, sizeof(bool));
	}

	bool CSocket::IsConditionalAccept()
	{
		bool bConditionalAccept = true;
		int nSize = sizeof(bool);

		if (!GetSocketOption(SOL_SOCKET, SO_CONDITIONAL_ACCEPT, 
			(char* )&bConditionalAccept, &nSize))
		{
			return false;
		}

		return bConditionalAccept;
	}

	bool CSocket::SetWaitTime(u_short dwSecond)
	{
		linger sLinger;
		sLinger.l_onoff = 1;
		sLinger.l_linger = dwSecond;

		return SetSocketOption(SOL_SOCKET, SO_LINGER, (const char*)&sLinger, sizeof(linger));
	}

	WORD CSocket::GetWaitTime()
	{
		linger sLinger;
		int nSize = sizeof(linger);

		if (!GetSocketOption(SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char* )&sLinger, &nSize))
		{
			return 0;
		}

		return sLinger.l_linger;
	}

	bool CSocket::OpenKeepAlive(
		UINT32 nIntervalTime, 
		UINT32 nTime)
	{
		bool bKeepAlive = true; 

		if (!SetSocketOption(SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive)))
		{
			return false;
		}

		//设置KeepAlive检测时间和次数 
		SYSBASE_TCP_KEEPALIVE inKeepAlive = {0}; //输入参数 
		unsigned long ulInLen = sizeof(SYSBASE_TCP_KEEPALIVE); 

		SYSBASE_TCP_KEEPALIVE outKeepAlive = {0}; //输出参数 
		unsigned long ulOutLen = sizeof(SYSBASE_TCP_KEEPALIVE); 

		unsigned long ulBytesReturn = 0; 

		inKeepAlive.onoff = 1; 
		inKeepAlive.keepaliveinterval = nIntervalTime; //两次KeepAlive探测间的时间间隔 
		inKeepAlive.keepalivetime = nTime; //开始首次KeepAlive探测前的TCP空闭时间 

		if (SOCKET_ERROR == WSAIoctl(m_Socket, 
			SIO_KEEPALIVE_VALS, 
			(LPVOID)&inKeepAlive, 
			ulInLen, 
			(LPVOID)&outKeepAlive, 
			ulOutLen, 
			&ulBytesReturn, 
			NULL, 
			NULL))
		{
			return false;
		}

		return true;
	}

	bool CSocket::CloseKeepAlive()
	{
		bool bKeepAlive = false; 

		return SetSocketOption(SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive));
	}

	void CSocket::Stop(int nHow)
	{
		m_bIsClose = true;

		shutdown(m_Socket, nHow);
	}

	bool CSocket::IsStop()
	{
		return m_bIsClose;
	}

	void CSocket::Close()
	{
		if (m_Socket != INVALID_SOCKET)
		{
			closesocket(m_Socket);

			m_Socket = INVALID_SOCKET;
		}
	}

	bool CSocket::IsClose()
	{
		return m_Socket == INVALID_SOCKET ? true : false;
	}

	bool CSocket::GetRemoveAddress(INET_SOCKET_ADDRESS_STRUCT* pSocketAddress)
	{
		if (!pSocketAddress)
		{
			return false;
		}

		int nSockAddrLen = INET_SOCKET_ADDRESS_STRUCT_LENGTH;

		if (SOCKET_ERROR == getpeername(m_Socket, (SOCKADDR* )pSocketAddress, &nSockAddrLen))
		{
			return false;
		}

		return true;
	}

	bool CSocket::GetRemoveAddress(TSockAddr* pSocketAddress)
	{
		if (!pSocketAddress)
		{
			return false;
		}

			INET_SOCKET_ADDRESS_STRUCT aSocketAddress = {0};

			if (!this->GetRemoveAddress(&aSocketAddress))
			{
				return false;
			}

			if (!CSocketAddr::ToTSockAddr(&aSocketAddress, pSocketAddress))
			{
				return false;
			}

			return true;
	}

	bool CSocket::GetLoaclAddress(INET_SOCKET_ADDRESS_STRUCT* pSocketAddress)
	{
		if (!pSocketAddress)
		{
			return false;
		}

		int nSockAddrLen = INET_SOCKET_ADDRESS_STRUCT_LENGTH;

		if (SOCKET_ERROR == getsockname(m_Socket, (SOCKADDR* )pSocketAddress, &nSockAddrLen))
		{
			return false;
		}

		return true;
	}

	bool CSocket::GetLoaclAddress(TSockAddr* pSocketAddress)
	{
		if (!pSocketAddress)
		{
			return false;
		}

		INET_SOCKET_ADDRESS_STRUCT aSocketAddress = {0};

		if (!this->GetLoaclAddress(&aSocketAddress))
		{
			return false;
		}

		if (!CSocketAddr::ToTSockAddr(&aSocketAddress, pSocketAddress))
		{
			return false;
		}

		return true;
	}
}
