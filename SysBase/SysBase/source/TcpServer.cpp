#include "../SysBase_Interface.h"

#define SYSBASE_IOCP_STOP -1 

#define SYSBASE_SOCKET_KEEP_ALIVE_OP_TYPE_UPDATE 1
#define SYSBASE_SOCKET_KEEP_ALIVE_OP_TYPE_DELETE 2

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CTcpServerImp

	class CTcpServerImp
	{
	public:

		CTcpServerImp(CTcpServer *pCTcpServer);

		virtual ~CTcpServerImp();

		CTcpServer::ERROR_CODE Start(
			ITcpServerHandler* pITcpServerHandler,
			UINT32 nLocalIP,
			UINT16 nPort,
			UINT16 nMaxConnectCount,
			UINT16 nIOWorkerCount,
			UINT32 nRecvBufferSize,
			UINT32 nSendBufferSize,
			UINT32 nSendBufferCount, 
			CTcpServer::KEEP_ALIVE_TYPE eKeepAliveType = CTcpServer::KEEP_ALIVE_TYPE_NONE,
			UINT32 nKeepAliveIntervalTime = 0);

		void Stop();

		CTcpServer::ERROR_CODE Send(
			UINT32 nSocketID,
			const void* pData,
			UINT32 nDataSize);

		void CloseSocket(UINT32 nSocketID);

		UINT32 GetSocketCount();
	
	protected:

		//////////////////////////////////////////////////////////////////////////

		union TSocketID
		{
			struct  
			{
				UINT16 nIndex;									//索引
				UINT16 nValue;									//校验值
			};

			UINT32 nSocketID;
		};

		struct TSocketInfo 
		{
			UINT32 nIndex;									//索引

			UINT32 nValue;									//校验值

			SOCKET aSocket;                                 //远程SOCKET对象

			char* recvBuffer;								//接收缓冲区
			UINT32 nRecvDataSize;							//当前有效的接收缓冲区大小

			UINT32 nIP;										//远程连接地址
			UINT16 nPort;									//远程连接端口

			CThreadMutex aCThreadMutex;						//线程同步

			UINT32 nLastRecvTickTime;						//最后一次收包时间

			WSAOVERLAPPED wsaOverLapped;					
		};

		struct TSocketKeepAlive 
		{
			UINT32 nSocketID;
			char btOpType;		//0:更新;1:删除
		};

		struct TSendInfo 
		{
			UINT32 nSocketID;
			UINT32 nDataSize;
		};

		//////////////////////////////////////////////////////////////////////////

		bool PostRecv(UINT32 nSocketID);

        bool PostRecvUnlock(TSocketInfo* pTSocketInfo);

		bool InitListenSocket();

		void CloseListenSocket();

		bool InitRecvWorker();

		void CloseRecvWorker();

		bool InitSendWorker();

		void CloseSendWorker();

		bool InitKeepAliveCheck();

		void CloseKeepAliveCheck();

		bool InitSocketInfos();

		void CloseSocketInfos();

		TSocketInfo* FindSocketInfoLock(UINT32 nSocketID);

		void UnlockSocketInfo(TSocketInfo* pTSocketInfo);

		bool AddSocket(SOCKET aSocket, UINT32 nIP, UINT16 nPort);

		void DeleteSocketUnLock(TSocketInfo* pTSocketInfo, ITcpServerHandler::ERROR_CODE errorCode);

		void DeleteSocket(UINT32 nSocketID, ITcpServerHandler::ERROR_CODE errorCode);

		//////////////////////////////////////////////////////////////////////////
		//工作线程

		//连接监听线程处理逻辑
		static DWORD WINAPI ProcListenThread(LPVOID lpParam);

		//连接数据接收线程处理逻辑
		static DWORD WINAPI ProcRecvThread(LPVOID lpParam);

		//连接数据发送线程处理逻辑
		static DWORD WINAPI ProcSendThread(LPVOID lpParam);

		//数据包心跳检测处理逻辑
		static DWORD WINAPI ProcKeepAliveThread(LPVOID lpParam);

	private:

		bool m_bStop;                                       //停止

		CTcpServer *m_pCTcpServer;

		ITcpServerHandler* m_pHandler;						//执行细节

		CThreadMutex m_CThreadMutex;						//线程同步

		UINT16 m_nIOWorkerCount;							//工作线程数

		//////////////////////////////////////////////////////////////////////////
		//监听相关

		UINT32 m_nListenIP;									//监听地址
		UINT16 m_nListenPort;								//监听端口

		SOCKET m_ListenSocket;                              //监听的SOCKET

		HANDLE m_hListenThread;								//连接监听线程

		//////////////////////////////////////////////////////////////////////////
		//连接管理

		UINT16 m_nCurSocketCount;							//当前已经使用的SOCKET资源数

		UINT16 m_nMaxSocketCount;                           //最大保存SOCKET数

		char* m_pSocketInfoMem;							    //连接集合

		CQueue<TSocketInfo*> m_SocketInfoIdleQueue;			//对象信息队列

		//////////////////////////////////////////////////////////////////////////
		//数据接收

		UINT32 m_nRecvBufferSize;							//接收缓存区大小

		HANDLE m_hIocp;										

		HANDLE* m_hRecvThreadArray;							//数据接收线程

		//////////////////////////////////////////////////////////////////////////
		//数据发送

		UINT32 m_nSendBufferSize;							//发送缓存区大小

		UINT32 m_nSendBufferCount;							//发送缓存区个数

		HANDLE* m_hSendThreadArray;							//数据发送线程

        char* m_pSenBufferQueueMem;						    //连接集合

        CQueue<CBufferQueue*> m_sendBuufferQueueObjQueue;	//对象信息队列

		//////////////////////////////////////////////////////////////////////////
		//心跳处理逻辑相关

		CTcpServer::KEEP_ALIVE_TYPE m_eKeepAliveType;		//心跳类型

		HANDLE m_hKeepAliveThread;							//心跳线程

		UINT32 m_nKeepAliveIntervalTime;					//心跳间隔时间

		typedef map<UINT32, UINT32> KeepAliveCheckMap;

		CEvent m_KeepAliveEvent;
	};

	//////////////////////////////////////////////////////////////////////////
	//CTcpServerImp 实现

	CTcpServerImp::CTcpServerImp(CTcpServer* pCTcpServer)
	{
		m_bStop = TRUE;

		m_pCTcpServer = pCTcpServer;

		m_pHandler = NULL;

		m_nCurSocketCount = 0;

		m_nMaxSocketCount = 0;

		m_ListenSocket = INVALID_SOCKET;

		m_pSocketInfoMem = NULL;

		m_nIOWorkerCount = 0;

		m_hListenThread = NULL;

		m_hRecvThreadArray = NULL;

		m_hSendThreadArray = NULL;

		m_hKeepAliveThread = NULL;

		m_nKeepAliveIntervalTime = 0;

		m_nListenIP = 0;

		m_nListenPort = 0;

		m_nRecvBufferSize = 0;

		m_nSendBufferSize = 0;

		m_nSendBufferCount = 0;

		m_hIocp = 0;

        m_pSenBufferQueueMem = NULL;
	}

	CTcpServerImp::~CTcpServerImp()
	{
		this->Stop();
	}

	CTcpServer::ERROR_CODE CTcpServerImp::Start(
		ITcpServerHandler* pITcpServerHandler,
		UINT32 nLocalIP,
		UINT16 nPort,
		UINT16 nMaxConnectCount,
		UINT16 nIOWorkerCount,
		UINT32 nRecvBufferSize,
		UINT32 nSendBufferSize,
		UINT32 nSendBufferCount, 
		CTcpServer::KEEP_ALIVE_TYPE eKeepAliveType,
		UINT32 nKeepAliveIntervalTime)
	{
		if (!m_bStop)
		{
			return CTcpServer::ERROR_CODE_START;
		}

		if (!pITcpServerHandler
			|| 0 == nPort
			|| 0 == nMaxConnectCount
			|| 0 == nIOWorkerCount
			|| 0 == nRecvBufferSize
			|| 0 == nSendBufferSize
			|| 0 == nSendBufferCount)
		{
			return CTcpServer::ERROR_CODE_PARAM;
		}

        if (CTcpServer::KEEP_ALIVE_TYPE_NONE != eKeepAliveType
            && 0 == nKeepAliveIntervalTime)
        {
            return CTcpServer::ERROR_CODE_PARAM;
        }

		m_pHandler = pITcpServerHandler;

		m_nIOWorkerCount = nIOWorkerCount;
	
		m_nListenIP = nLocalIP;

		m_nListenPort = nPort;

		m_nCurSocketCount = 0;

		m_nMaxSocketCount = nMaxConnectCount;

		m_nRecvBufferSize = nRecvBufferSize;

		m_nSendBufferSize = nSendBufferSize;

		m_nSendBufferCount = nSendBufferCount;

		m_eKeepAliveType = eKeepAliveType;

		m_nKeepAliveIntervalTime = nKeepAliveIntervalTime;

		m_bStop = false;

		if (!this->InitSocketInfos())
		{
			this->Stop();

			return CTcpServer::ERROR_CODE_SYSTEM;
		}

		if (!this->InitKeepAliveCheck())
		{
			this->Stop();

			return CTcpServer::ERROR_CODE_SYSTEM; 
		}

		if (!this->InitSendWorker())
		{
			this->Stop();

			return CTcpServer::ERROR_CODE_SYSTEM; 
		}

		if (!this->InitRecvWorker())
		{
			this->Stop();

			return CTcpServer::ERROR_CODE_SYSTEM; 
		}

		if (!this->InitListenSocket())
		{
			this->Stop();

			return CTcpServer::ERROR_CODE_SYSTEM; 
		}

		return CTcpServer::ERROR_CODE_SUCCESS;
	}

	void CTcpServerImp::Stop()
	{
		if (m_bStop)
		{
			return;
		}

		m_bStop = true;

		this->CloseListenSocket();

		this->CloseRecvWorker();

		this->CloseSendWorker();

		this->CloseKeepAliveCheck();

		this->CloseSocketInfos();
	}

	CTcpServer::ERROR_CODE CTcpServerImp::Send(
		UINT32 nSocketID,
		const void* pData,
		UINT32 nDataSize)
	{
		if (m_bStop)
		{
			return CTcpServer::ERROR_CODE_STOP;
		}

		if (!pData || 0 == nDataSize)
		{
			return CTcpServer::ERROR_CODE_PARAM;
		}

		if (nDataSize > m_nSendBufferSize)
		{
			return CTcpServer::ERROR_CODE_OVER_SIZE;
		}

        UINT32 nIndex = nSocketID % m_nIOWorkerCount;

        CBufferQueue* pCBufferQueue = (CBufferQueue*)(m_pSenBufferQueueMem + sizeof(CBufferQueue) * nIndex);

		CBufferWrite aCBufferWrite;

		if (CBufferQueue::ERROR_CODE_SUCCESS != pCBufferQueue->GetBufferWrite(aCBufferWrite))
		{
			return CTcpServer::ERROR_CODE_QUEUE_FULL;
		}

		TSendInfo aTSendInfo = {0};
		aTSendInfo.nSocketID = nSocketID;
		aTSendInfo.nDataSize = nDataSize;

		aCBufferWrite.AddData(&aTSendInfo, sizeof(TSendInfo));
		aCBufferWrite.AddData(pData, nDataSize);

		aCBufferWrite.Push();

		return CTcpServer::ERROR_CODE_SUCCESS;
	}

	void CTcpServerImp::CloseSocket(UINT32 nSocketID)
	{
		if (m_bStop)
		{
			return;
		}

		this->DeleteSocket(nSocketID, ITcpServerHandler::ERROR_CODE_USER_CLOSE);
	}

	UINT32 CTcpServerImp::GetSocketCount()
	{
		if (m_bStop)
		{
			return 0;
		}

		return m_nCurSocketCount;
	}

	bool CTcpServerImp::PostRecv(UINT32 nSocketID)
	{
		TSocketInfo* pTSocketInfo = NULL;

		pTSocketInfo = this->FindSocketInfoLock(nSocketID);

		if (!pTSocketInfo)
		{
			return false;
		}

		return this->PostRecvUnlock(pTSocketInfo);
	}

    bool CTcpServerImp::PostRecvUnlock(TSocketInfo* pTSocketInfo)
    {
        if (!pTSocketInfo)
        {
            return false;
        }

        WSABUF wsaBuf = {0};
        wsaBuf.buf = pTSocketInfo->recvBuffer + pTSocketInfo->nRecvDataSize;
        wsaBuf.len = m_nRecvBufferSize - pTSocketInfo->nRecvDataSize;

        DWORD dwFlags = 0;
        DWORD dwNumOfBytes = 0;

        ZeroMemory(&pTSocketInfo->wsaOverLapped, sizeof(WSAOVERLAPPED));

        if (SOCKET_ERROR == WSARecv(pTSocketInfo->aSocket, 
            &wsaBuf, 
            1, 
            &dwNumOfBytes, 
            &dwFlags, 
            &pTSocketInfo->wsaOverLapped,
            NULL))
        {
            DWORD dwError = WSAGetLastError();

            if (dwError != WSA_IO_PENDING)
            {
                this->DeleteSocketUnLock(pTSocketInfo, ITcpServerHandler::ERROR_CODE_SYSTEM);
                return false;
            }
        }

        this->UnlockSocketInfo(pTSocketInfo);

        return true;
    }

	bool CTcpServerImp::InitListenSocket()
	{
		m_ListenSocket = WSASocketA(AF_INET, 
			SOCK_STREAM, 
			IPPROTO_TCP, 
			NULL, 
			0, 
			WSA_FLAG_OVERLAPPED);

		//m_ListenSocket =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET == m_ListenSocket)
		{
			return false;
		}

        //设置系统接受缓冲区大小

        int nSysRecvBufferSize = SO_MAX_MSG_SIZE;

        if (setsockopt(m_ListenSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nSysRecvBufferSize, sizeof(int)) == SOCKET_ERROR)
        {
            closesocket(m_ListenSocket);

            return false;
        }

        //设置系统发送缓冲区大小

        int nSysSendBufferSize = SO_MAX_MSG_SIZE;

        if (setsockopt(m_ListenSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&nSysSendBufferSize, sizeof(int)) == SOCKET_ERROR)
        {
            closesocket(m_ListenSocket);

            return false;
        }

        //设置系统提供的心跳检查
        if (CTcpServer::KEEP_ALIVE_TYPE_SYSTEM == m_eKeepAliveType)
        {
            //设置KeepAlive检测时间和次数 
            SYSBASE_TCP_KEEPALIVE inKeepAlive = {0}; //输入参数 
            unsigned long ulInLen = sizeof(SYSBASE_TCP_KEEPALIVE); 

            SYSBASE_TCP_KEEPALIVE outKeepAlive = {0}; //输出参数 
            unsigned long ulOutLen = sizeof(SYSBASE_TCP_KEEPALIVE); 

            unsigned long ulBytesReturn = 0; 

            inKeepAlive.onoff = 1; 
            inKeepAlive.keepaliveinterval = m_nKeepAliveIntervalTime;     //两次KeepAlive探测间的时间间隔 
            inKeepAlive.keepalivetime = m_nKeepAliveIntervalTime;         //开始首次KeepAlive探测前的TCP空闭时间 

            if (SOCKET_ERROR == WSAIoctl(m_ListenSocket, 
                SIO_KEEPALIVE_VALS, 
                (LPVOID)&inKeepAlive, 
                ulInLen, 
                (LPVOID)&outKeepAlive, 
                ulOutLen, 
                &ulBytesReturn, 
                NULL, 
                NULL))
            {
                closesocket(m_ListenSocket);

                return false;
            }
        }

		//设置为非阻塞模式
		// If iMode = 0, blocking is enabled; 
		// If iMode != 0, non-blocking mode is enabled.				
		int iMode = 1;
		if (SOCKET_ERROR == ioctlsocket(m_ListenSocket, FIONBIO, (u_long FAR*)&iMode))
		{
			closesocket(m_ListenSocket);

			m_ListenSocket = INVALID_SOCKET;

			return false;
		}

        //绑定端口
		SOCKADDR_IN socketAddr = {0};

		socketAddr.sin_family = AF_INET;
		socketAddr.sin_addr.s_addr = m_nListenIP;
		socketAddr.sin_port = htons(m_nListenPort);

		if (SOCKET_ERROR == bind(m_ListenSocket, (SOCKADDR*)&socketAddr, sizeof(SOCKADDR_IN)))
		{
			closesocket(m_ListenSocket);

			m_ListenSocket = INVALID_SOCKET;

			return false;
		}

        //启动监听
		if (SOCKET_ERROR == listen(m_ListenSocket, m_nMaxSocketCount))
		{
			closesocket(m_ListenSocket);

			m_ListenSocket = INVALID_SOCKET;

			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		//启动监听线程

		m_hListenThread = CreateThread(NULL, 0, CTcpServerImp::ProcListenThread, this, 0, NULL);

		if (NULL == m_hListenThread)
		{
			closesocket(m_ListenSocket);

			m_ListenSocket = INVALID_SOCKET;

			return false;
		}

		return true;
	}

	bool CTcpServerImp::InitRecvWorker()
	{
		m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 0);

		if (!m_hIocp)
		{
			return false;
		}

		m_hRecvThreadArray = new(std::nothrow) HANDLE[m_nIOWorkerCount];

		if (!m_hRecvThreadArray)
		{
			return false;
		}

		UINT16 nIndex = 0;

		for (nIndex = 0; nIndex < m_nIOWorkerCount; ++nIndex)
		{
			m_hRecvThreadArray[nIndex] = CreateThread(NULL, 0, CTcpServerImp::ProcRecvThread, this, 0, NULL);

			if (!m_hRecvThreadArray[nIndex])
			{
				return false;
			}
		}
	
		return true;
	}

	void CTcpServerImp::CloseRecvWorker()
	{
		if (m_hRecvThreadArray)
		{
			UINT16 nIndex = 0;

			// 通知所有I/O处理线程退出
			for(nIndex = 0; nIndex < m_nIOWorkerCount; ++nIndex)
			{	
				PostQueuedCompletionStatus(m_hIocp, SYSBASE_IOCP_STOP, 0, NULL);
			}

			if (WAIT_TIMEOUT == WaitForMultipleObjects(m_nIOWorkerCount, m_hRecvThreadArray, TRUE, 3000))
			{
				for(nIndex = 0; nIndex < m_nIOWorkerCount; ++nIndex)
				{	
					TerminateThread(m_hRecvThreadArray[nIndex], 0);
				}
			}

			for(nIndex = 0; nIndex < m_nIOWorkerCount; ++nIndex)
			{	
				CloseHandle(m_hRecvThreadArray[nIndex]);
			}

			delete [] m_hRecvThreadArray;
			
			m_hRecvThreadArray = NULL;
		}
	}

	bool CTcpServerImp::InitSendWorker()
	{
        m_pSenBufferQueueMem = new(std::nothrow) char[sizeof(CBufferQueue) * m_nIOWorkerCount];

        if (!m_pSenBufferQueueMem)
        {
            return false;
        }

        if (!m_sendBuufferQueueObjQueue.Init(m_nIOWorkerCount, false, true))
        {
            return false;
        }

        UINT16 nIndex = 0;

        for (nIndex = 0; nIndex < m_nIOWorkerCount; ++nIndex)
        {
            CBufferQueue* pCBufferQueue = new(m_pSenBufferQueueMem + sizeof(CBufferQueue) * nIndex) CBufferQueue();

            if (!pCBufferQueue)
            {
                return false;
            }

            if (CBufferQueue::ERROR_CODE_SUCCESS != pCBufferQueue->Init(m_nSendBufferSize + sizeof(TSendInfo), m_nSendBufferCount, true))
            {
                return false;
            }

            m_sendBuufferQueueObjQueue.Push(pCBufferQueue);
        }

		m_hSendThreadArray = new(std::nothrow) HANDLE[m_nIOWorkerCount];

		if (!m_hSendThreadArray)
		{
			return false;
		}

		for (nIndex = 0; nIndex < m_nIOWorkerCount; ++nIndex)
		{
			m_hSendThreadArray[nIndex] = CreateThread(NULL, 0, CTcpServerImp::ProcSendThread, this, 0, NULL);

			if (!m_hSendThreadArray[nIndex])
			{
				return false;
			}
		}

		return true;
	}

	void CTcpServerImp::CloseSendWorker()
	{
		if (m_hSendThreadArray)
		{
			UINT16 nIndex = 0;

			if (WAIT_TIMEOUT == WaitForMultipleObjects(m_nIOWorkerCount, m_hSendThreadArray, TRUE, 3000))
			{
				for(nIndex = 0; nIndex < m_nIOWorkerCount; ++nIndex)
				{	
					TerminateThread(m_hSendThreadArray[nIndex], 0);
				}
			}

			for(nIndex = 0; nIndex < m_nIOWorkerCount; ++nIndex)
			{	
				CloseHandle(m_hSendThreadArray[nIndex]);
			}

			delete [] m_hSendThreadArray;

			m_hSendThreadArray = NULL;
		}

        if (m_pSenBufferQueueMem)
        {
            UINT16 nIndex = 0;

            for (nIndex = 0; nIndex < m_nIOWorkerCount; ++nIndex)
            {
                CBufferQueue* pCBufferQueue = (CBufferQueue*)(m_pSenBufferQueueMem + sizeof(CBufferQueue) * nIndex);

                pCBufferQueue->~CBufferQueue();
            }

            delete [] m_pSenBufferQueueMem;

            m_pSenBufferQueueMem = NULL;
        }
	}

	void CTcpServerImp::CloseListenSocket()
	{
		if (m_hListenThread)
		{
			if (WAIT_TIMEOUT == WaitForSingleObject(m_hListenThread, 3000))
			{
				TerminateThread(m_hListenThread, 0);
			}

			CloseHandle(m_hListenThread);

			m_hListenThread = NULL;
		}

		if (INVALID_SOCKET != m_ListenSocket)
		{
			closesocket(m_ListenSocket);

			m_ListenSocket = INVALID_SOCKET;
		}
	}

	bool CTcpServerImp::InitKeepAliveCheck()
	{
		if (m_eKeepAliveType != CTcpServer::KEEP_ALIVE_TYPE_DATA)
		{
			return true;
		}

		m_hKeepAliveThread = CreateThread(NULL, 0, CTcpServerImp::ProcKeepAliveThread, this, 0, NULL);

		if (NULL == m_hKeepAliveThread)
		{
			return false;
		}

		return true;
	}

	void CTcpServerImp::CloseKeepAliveCheck()
	{
		if (m_eKeepAliveType != CTcpServer::KEEP_ALIVE_TYPE_DATA)
		{
			return;
		}

		m_KeepAliveEvent.Set();

		if (m_hKeepAliveThread)
		{
			if (WAIT_TIMEOUT == WaitForSingleObject(m_hKeepAliveThread, 3000))
			{
				TerminateThread(m_hKeepAliveThread, 0);
			}

			CloseHandle(m_hKeepAliveThread);

			m_hKeepAliveThread = NULL;
		}
	}

	bool CTcpServerImp::InitSocketInfos()
	{
		UINT32 nSocketInfoMemSize = (sizeof(TSocketInfo) + m_nRecvBufferSize) * m_nMaxSocketCount;

		m_pSocketInfoMem = new(std::nothrow) char[nSocketInfoMemSize];

		if (!m_pSocketInfoMem)
		{
			return false;
		}

		ZeroMemory(m_pSocketInfoMem, nSocketInfoMemSize);

		if (!m_SocketInfoIdleQueue.Init(m_nMaxSocketCount, false, true))
		{
			return false;
		}

		UINT16 nIndex = 0;

		char* pRecvBufferMem = m_pSocketInfoMem + m_nMaxSocketCount * sizeof(TSocketInfo);

		for (nIndex = 0; nIndex < m_nMaxSocketCount; nIndex++)
		{
			TSocketInfo* pTSocketInfo = new(m_pSocketInfoMem + nIndex * sizeof(TSocketInfo)) TSocketInfo();

			pTSocketInfo->aSocket = INVALID_SOCKET;

			pTSocketInfo->nIndex = nIndex;

			pTSocketInfo->nValue = 0;

			pTSocketInfo->nIP = 0;

			pTSocketInfo->nPort = 0;

			pTSocketInfo->nLastRecvTickTime = 0;

			pTSocketInfo->nRecvDataSize = 0;

			pTSocketInfo->recvBuffer = pRecvBufferMem + nIndex * m_nRecvBufferSize;

			m_SocketInfoIdleQueue.Push(pTSocketInfo);
		}

		return true;
	}

	void CTcpServerImp::CloseSocketInfos()
	{
		if (!m_SocketInfoIdleQueue.Empty())
		{
			m_SocketInfoIdleQueue.Clear();
		}

		if (m_pSocketInfoMem)
		{
			UINT16 nIndex = 0;

			for (nIndex = 0; nIndex < m_nMaxSocketCount; nIndex++)
			{
				TSocketInfo* pTSocketInfo = (TSocketInfo*)(m_pSocketInfoMem + nIndex * sizeof(TSocketInfo));

				pTSocketInfo->~TSocketInfo();
			}

			delete [] m_pSocketInfoMem;

			m_pSocketInfoMem = NULL;
		}
	}

	CTcpServerImp::TSocketInfo* CTcpServerImp::FindSocketInfoLock(UINT32 nSocketID)
	{
		TSocketID aTSocketID = {0};
		aTSocketID.nSocketID = nSocketID;

		if (aTSocketID.nIndex >= m_nMaxSocketCount)
		{
			return NULL;
		}

		TSocketInfo* pTSocketInfo = (TSocketInfo*)(m_pSocketInfoMem + sizeof(TSocketInfo) * aTSocketID.nIndex);

		pTSocketInfo->aCThreadMutex.Lock();

		if (pTSocketInfo->nValue != aTSocketID.nValue
            || INVALID_SOCKET == pTSocketInfo->aSocket)
		{
			pTSocketInfo->aCThreadMutex.Unlock();

			return NULL;
		}

		return pTSocketInfo;
	}

	void CTcpServerImp::UnlockSocketInfo(TSocketInfo* pTSocketInfo)
	{
		if (!pTSocketInfo)
		{
			return;
		}

		pTSocketInfo->aCThreadMutex.Unlock();
	}

	bool CTcpServerImp::AddSocket(SOCKET aSocket, UINT32 nIP, UINT16 nPort)
	{
		if (INVALID_SOCKET == aSocket)
		{
			return false;
		}

		TSocketInfo* pTSocketInfo = NULL;

		if (!m_SocketInfoIdleQueue.Pop(pTSocketInfo))
		{
			m_pHandler->OnAcceptedError(m_pCTcpServer, nIP, nPort, ITcpServerHandler::ERROR_CODE_SOCKET_FULL);

            closesocket(aSocket);

			return false;
		}

		pTSocketInfo->aCThreadMutex.Lock();

		pTSocketInfo->aSocket = aSocket;

		pTSocketInfo->nIP = nIP;

		pTSocketInfo->nPort = nPort;

		pTSocketInfo->nRecvDataSize = 0;

		pTSocketInfo->nLastRecvTickTime = GetTickCount();

		TSocketID aTSocketID = {0};
		aTSocketID.nIndex = pTSocketInfo->nIndex;
		aTSocketID.nValue = pTSocketInfo->nValue;

		//绑定到IOCP端口
		if (NULL == CreateIoCompletionPort((HANDLE)pTSocketInfo->aSocket, m_hIocp, (ULONG_PTR)aTSocketID.nSocketID, 0))
		{
			this->DeleteSocketUnLock(pTSocketInfo, ITcpServerHandler::ERROR_CODE_SUCCESS);

            m_pHandler->OnAcceptedError(m_pCTcpServer, nIP, nPort, ITcpServerHandler::ERROR_CODE_SYSTEM);

			return false;
		}

		pTSocketInfo->aCThreadMutex.Unlock();

		if (!m_pHandler->OnAccepted(m_pCTcpServer, aTSocketID.nSocketID, nIP, nPort))
		{
			//如果返回false，则关闭当前连接
			this->DeleteSocket(aTSocketID.nSocketID, ITcpServerHandler::ERROR_CODE_USER_CLOSE);

			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		//投递接收请求

		if (!this->PostRecv(aTSocketID.nSocketID))
		{
			return false;
		}

		return true;
	}

	void CTcpServerImp::DeleteSocketUnLock(TSocketInfo* pTSocketInfo, ITcpServerHandler::ERROR_CODE errorCode)
	{
		if (!pTSocketInfo)
		{
			return;
		}

		TSocketID aTSocketID = {0};
		aTSocketID.nIndex = pTSocketInfo->nIndex;
		aTSocketID.nValue = pTSocketInfo->nValue;

		UINT32 nIP = pTSocketInfo->nIP;
		UINT16 nPort = pTSocketInfo->nPort;

		pTSocketInfo->nValue++;

		closesocket(pTSocketInfo->aSocket);

		pTSocketInfo->aSocket = INVALID_SOCKET;

		pTSocketInfo->aCThreadMutex.Unlock();

		m_SocketInfoIdleQueue.Push(pTSocketInfo);

        if (ITcpServerHandler::ERROR_CODE_SUCCESS != errorCode)
        {
            m_pHandler->OnClosed(m_pCTcpServer, aTSocketID.nSocketID, nIP, nPort, errorCode);
        }
	}

	void CTcpServerImp::DeleteSocket(UINT32 nSocketID, ITcpServerHandler::ERROR_CODE errorCode)
	{
		TSocketInfo* pTSocketInfo = this->FindSocketInfoLock(nSocketID);

        if (!pTSocketInfo)
        {
            return;
        }

        this->DeleteSocketUnLock(pTSocketInfo, errorCode);
	}

	//////////////////////////////////////////////////////////////////////////
	//工作线程

	DWORD WINAPI CTcpServerImp::ProcListenThread(LPVOID lpParam)
	{
		CTcpServerImp* pCTcpServerImp = (CTcpServerImp*)lpParam;

		while (!pCTcpServerImp->m_bStop)
		{
			fd_set fdRead = {0};
			FD_SET(pCTcpServerImp->m_ListenSocket, &fdRead);

			timeval timeout = {0};
			timeout.tv_sec = 1;

			if (select((int)pCTcpServerImp->m_ListenSocket + 1, &fdRead, NULL, NULL, &timeout) > 0)
			{
				if (FD_ISSET(pCTcpServerImp->m_ListenSocket, &fdRead))
				{
					SOCKADDR_IN sockAddr = {0};
					int sockAddrLen = sizeof(SOCKADDR_IN);

					SOCKET acceptSocket = accept(pCTcpServerImp->m_ListenSocket, (SOCKADDR*)&sockAddr, &sockAddrLen);

					if (INVALID_SOCKET != acceptSocket)
					{
						UINT32 nIP = sockAddr.sin_addr.s_addr;
						UINT16 nPort = ntohs(sockAddr.sin_port);

						pCTcpServerImp->AddSocket(acceptSocket, nIP, nPort);
					}
				}
			}
		}

		return 0;
	}

	DWORD WINAPI CTcpServerImp::ProcRecvThread(LPVOID lpParam)
	{
		CTcpServerImp* pCTcpServerImp = (CTcpServerImp*)lpParam;

		char* recvBuffer = new(std::nothrow) char[pCTcpServerImp->m_nRecvBufferSize];

		UINT32 nIP = 0;
		UINT16 nPort = 0;;

		DWORD dwNumberOfBytes = 0;

		ULONG_PTR nReserveInfo = 0;

		UINT32 nSocketID = 0;

		LPOVERLAPPED lpOverLapped = NULL;

		BOOL bSuccess = TRUE;

		UINT32 nError = 0;

		while (!pCTcpServerImp->m_bStop)
		{
			bSuccess = GetQueuedCompletionStatus(pCTcpServerImp->m_hIocp,
				&dwNumberOfBytes, &nReserveInfo, &lpOverLapped, 1000);

			//通知退出 
			if (SYSBASE_IOCP_STOP == dwNumberOfBytes)
			{
				break;
			}

			if (!bSuccess)
			{
				DWORD dwError = GetLastError();

				if (WAIT_TIMEOUT == dwError)
				{
					continue;
				}
			}

			//////////////////////////////////////////////////////////////////////////
			//处理接收请求

			nSocketID = (UINT32)nReserveInfo;

			if (bSuccess)
			{
				//////////////////////////////////////////////////////////////////////////
				//接收到 0 字节为远程连接主动断开
				if (dwNumberOfBytes <= 0)
				{
					pCTcpServerImp->DeleteSocket(nSocketID, ITcpServerHandler::ERROR_CODE_REMOTE_CLOSE);

					continue;
				}

				//////////////////////////////////////////////////////////////////////////
				//处理接收的数据

				TSocketInfo* pTSocketInfo = pCTcpServerImp->FindSocketInfoLock(nSocketID);

                if (pTSocketInfo)
                {
                    pTSocketInfo->nRecvDataSize += dwNumberOfBytes;

                    //协议错误
                    if (pTSocketInfo->nRecvDataSize > pCTcpServerImp->m_nRecvBufferSize)
                    {
                        pCTcpServerImp->DeleteSocketUnLock(pTSocketInfo, ITcpServerHandler::ERROR_CODE_PROTOCOL);
                    }
                    else
                    {
                        while (pTSocketInfo)
                        {
                            if (pTSocketInfo->nRecvDataSize >= pCTcpServerImp->m_pHandler->GetHeadSize())
                            {
                                UINT32 nMsgSize = pCTcpServerImp->m_pHandler->GetMsgSize(pTSocketInfo->recvBuffer, pTSocketInfo->nRecvDataSize);

                                //协议错误
                                if (nMsgSize > pCTcpServerImp->m_nRecvBufferSize ||  nMsgSize < pCTcpServerImp->m_pHandler->GetHeadSize())
                                {
                                    pCTcpServerImp->DeleteSocketUnLock(pTSocketInfo, ITcpServerHandler::ERROR_CODE_PROTOCOL);

                                    pTSocketInfo = NULL;
                                }
                                else
                                {
                                    if (pTSocketInfo->nRecvDataSize >= nMsgSize)
                                    {
                                        nIP = pTSocketInfo->nIP;
                                        nPort = pTSocketInfo->nPort;

                                        memcpy(recvBuffer, pTSocketInfo->recvBuffer, nMsgSize);

                                        pTSocketInfo->nRecvDataSize -= nMsgSize;

                                        if (pTSocketInfo->nRecvDataSize > 0)
                                        {
                                            memmove(pTSocketInfo->recvBuffer,
                                                pTSocketInfo->recvBuffer + nMsgSize,
                                                pTSocketInfo->nRecvDataSize);
                                        }

                                        pTSocketInfo->nLastRecvTickTime = GetTickCount();

                                        pCTcpServerImp->UnlockSocketInfo(pTSocketInfo);

                                        pCTcpServerImp->m_pHandler->OnReceived(pCTcpServerImp->m_pCTcpServer, nSocketID, nIP, nPort, recvBuffer, nMsgSize);

                                        pTSocketInfo = pCTcpServerImp->FindSocketInfoLock(nSocketID);

                                        continue;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                break;
                            }
                        }

                        if (pTSocketInfo)
                        {
                            pCTcpServerImp->PostRecvUnlock(pTSocketInfo);
                        }
                    }
                }
			}
			else
			{
				//投递WSARecv获得false则是本地SOCKET主动断开或SOCKET已经不存在
				pCTcpServerImp->DeleteSocket(nSocketID, ITcpServerHandler::ERROR_CODE_REMOTE_CLOSE);
			}
		}

		delete [] recvBuffer;

		return 0;
	}

	DWORD WINAPI CTcpServerImp::ProcSendThread(LPVOID lpParam)
	{
		CTcpServerImp* pCTcpServerImp = (CTcpServerImp*)lpParam;

        CBufferQueue* pCBufferQueue = NULL;

        if (!pCTcpServerImp->m_sendBuufferQueueObjQueue.Pop(pCBufferQueue))
        {
            return 0;
        }

		while (!pCTcpServerImp->m_bStop)
		{
			CBufferRead aCBufferRead;

			if (CBufferQueue::ERROR_CODE_SUCCESS != pCBufferQueue->Pop(aCBufferRead, 1000))
			{
				continue;
			}

			char* pData = (char*)aCBufferRead.GetData();

			TSendInfo* pTSendInfo = (TSendInfo*)pData;

			char* pSendData = pData + sizeof(TSendInfo);

			int nTotalSendSize = (int)pTSendInfo->nDataSize;
			int nHasSendSize = 0;
			int nCurSendSize = 0;

			bool bSendSuccess = true;

            TSocketInfo* pTSocketInfo = pCTcpServerImp->FindSocketInfoLock(pTSendInfo->nSocketID);

			while (nHasSendSize < nTotalSendSize && pTSocketInfo)
			{
				nCurSendSize = send(pTSocketInfo->aSocket, pSendData + nHasSendSize, nTotalSendSize - nHasSendSize, 0); 

				if (SOCKET_ERROR == nCurSendSize)
				{
					DWORD dwError = WSAGetLastError();

					if (WSAEWOULDBLOCK != dwError)
					{
                        ITcpServerHandler::ERROR_CODE errorCode = ITcpServerHandler::ERROR_CODE_SYSTEM;

                        switch (dwError)
                        {
                        case WSAECONNABORTED:
                        case WSAECONNRESET:
                            {
                                errorCode = ITcpServerHandler::ERROR_CODE_REMOTE_CLOSE;
                            }
                            break;
                        }

                        pCTcpServerImp->DeleteSocketUnLock(pTSocketInfo, errorCode);
						
						bSendSuccess = false;

						break;
					}

                    pCTcpServerImp->UnlockSocketInfo(pTSocketInfo);

                    pTSocketInfo = pCTcpServerImp->FindSocketInfoLock(pTSendInfo->nSocketID);

                    continue;
				}
                else if (nCurSendSize > 0)
                {
                    nHasSendSize += nCurSendSize;
                }
                else    //nCurSendSize == 0
                {
                    pCTcpServerImp->UnlockSocketInfo(pTSocketInfo);

                    pTSocketInfo = pCTcpServerImp->FindSocketInfoLock(pTSendInfo->nSocketID);
                }
			}
			
			if (bSendSuccess && pTSocketInfo)
			{
				pCTcpServerImp->UnlockSocketInfo(pTSocketInfo);
			}
		}

		return 0;
	}

	DWORD WINAPI CTcpServerImp::ProcKeepAliveThread(LPVOID lpParam)
	{
		CTcpServerImp* pCTcpServerImp = (CTcpServerImp*)lpParam;

		while (!pCTcpServerImp->m_bStop)
		{
			if (pCTcpServerImp->m_KeepAliveEvent.Wait(pCTcpServerImp->m_nKeepAliveIntervalTime))
			{
				break;
			}

			DWORD dwCurTickCount = GetTickCount();

			UINT16 nIndex = pCTcpServerImp->m_nMaxSocketCount;

			for (nIndex = 0; nIndex < pCTcpServerImp->m_nMaxSocketCount; nIndex++)
			{
				TSocketInfo* pTSocketInfo = (TSocketInfo*)(pCTcpServerImp->m_pSocketInfoMem + nIndex * sizeof(TSocketInfo));

				pTSocketInfo->aCThreadMutex.Lock();

				if (INVALID_SOCKET != pTSocketInfo->aSocket)
				{
					if (dwCurTickCount > pTSocketInfo->nLastRecvTickTime)
					{
						if (dwCurTickCount - pTSocketInfo->nLastRecvTickTime > pCTcpServerImp->m_nKeepAliveIntervalTime)
						{
							pCTcpServerImp->DeleteSocketUnLock(pTSocketInfo, ITcpServerHandler::ERROR_CODE_KEEP_ALIVE_TIME_OUT);

							continue;
						}
					}
				}

				pTSocketInfo->aCThreadMutex.Unlock();
			}
		}

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//CTcpServer

	CTcpServer::CTcpServer()
	{
		m_Imp = new CTcpServerImp(this);
	}

	CTcpServer::~CTcpServer()
	{
		if (m_Imp)
		{
			CTcpServerImp* pImp = (CTcpServerImp*)m_Imp;

			delete pImp;

			m_Imp = NULL;
		}
	}

	CTcpServer::ERROR_CODE CTcpServer::Start(
		ITcpServerHandler* pITcpServerHandler,
		UINT32 nListenIP,
		UINT16 nPort,
		UINT16 nMaxConnectCount,
		UINT16 nIOWorkerCount,
		UINT32 nRecvBufferSize,
		UINT32 nSendBufferSize,
		UINT32 nSendBufferCount, 
		KEEP_ALIVE_TYPE eKeepAliveType,
		UINT32 nKeepAliveIntervalTime)
	{
		if (!m_Imp)
		{
			return ERROR_CODE_SYSTEM;
		}

		if (!CSocketLoader::InitSocket())
		{
			return ERROR_CODE_SYSTEM;
		}

		CTcpServerImp* pImp = (CTcpServerImp*)m_Imp;

		return pImp->Start(pITcpServerHandler,
			nListenIP,
			nPort,
			nMaxConnectCount,
			nIOWorkerCount,
			nRecvBufferSize,
			nSendBufferSize,
			nSendBufferCount,
			eKeepAliveType,
			nKeepAliveIntervalTime);
	}

    CTcpServer::ERROR_CODE CTcpServer::Start(
        ITcpServerHandler* pITcpServerHandler,
        PCCHAR szListenIP,
        UINT16 nPort,
        UINT16 nMaxConnectCount,
        UINT16 nIOWorkerCount,
        UINT32 nRecvBufferSize,
        UINT32 nSendBufferSize,
        UINT32 nSendBufferCount, 
        KEEP_ALIVE_TYPE eKeepAliveType,
        UINT32 nKeepAliveIntervalTime)
    {
        if (!m_Imp || !szListenIP)
        {
            return ERROR_CODE_SYSTEM;
        }

        if (!CSocketLoader::InitSocket())
        {
            return ERROR_CODE_SYSTEM;
        }

        DWORD nListenIP = CSocketAddr::ToDwordIP(szListenIP);

        CTcpServerImp* pImp = (CTcpServerImp*)m_Imp;

        return pImp->Start(pITcpServerHandler,
            nListenIP,
            nPort,
            nMaxConnectCount,
            nIOWorkerCount,
            nRecvBufferSize,
            nSendBufferSize,
            nSendBufferCount,
            eKeepAliveType,
            nKeepAliveIntervalTime);
    }

	void CTcpServer::Stop()
	{
		if (!m_Imp)
		{
			return;
		}

		CTcpServerImp* pImp = (CTcpServerImp*)m_Imp;

		pImp->Stop();
	}

	CTcpServer::ERROR_CODE CTcpServer::Send(
		UINT32 nSocketID,
		const void* pData,
		UINT32 nDataSize)
	{
		if (!m_Imp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CTcpServerImp* pImp = (CTcpServerImp*)m_Imp;

		return pImp->Send(nSocketID, pData, nDataSize);
	}

	void CTcpServer::CloseSocket(UINT32 nSocketID)
	{
		if (!m_Imp)
		{
			return;
		}

		CTcpServerImp* pImp = (CTcpServerImp*)m_Imp;

		pImp->CloseSocket(nSocketID);
	}

	UINT32 CTcpServer::GetSocketCount()
	{
		if (!m_Imp)
		{
			return 0;
		}

		CTcpServerImp* pImp = (CTcpServerImp*)m_Imp;

		return pImp->GetSocketCount();
	}
}