#include "../SysBase_Interface.h"

#define SYSBASE_IOCP_STOP -1 

#define SYSBASE_SOCKET_KEEP_ALIVE_OP_TYPE_UPDATE 1
#define SYSBASE_SOCKET_KEEP_ALIVE_OP_TYPE_DELETE 2

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CTcpMultiClientImp

	class CTcpMultiClientImp
	{
	public:

		CTcpMultiClientImp(CTcpMultiClient *pCTcpMultiClient);

		virtual ~CTcpMultiClientImp();

		CTcpMultiClient::ERROR_CODE Start(
			ITcpMultiClientHandler* pITcpMultiClientHandler,
			UINT16 nMaxClientCount,
			UINT16 nIOWorkerCount,
			UINT32 nRecvBufferSize,
			UINT32 nSendBufferSize,
			UINT32 nSendBufferCount);

		void Stop();

        CTcpMultiClient::ERROR_CODE Connect(
            UINT32& SocketID_out, 
            UINT32 nIP, 
            UINT16 nPort, 
            UINT32 nWaitTime = 3000,             
            bool bSysKeepAlive = false,
            UINT32 nIntervalTime = 20000);

		CTcpMultiClient::ERROR_CODE Send(
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

		bool InitRecvWorker();

		void CloseRecvWorker();

		bool InitSendWorker();

		void CloseSendWorker();

		bool InitSocketInfos();

		void CloseSocketInfos();

		TSocketInfo* FindSocketInfoLock(UINT32 nSocketID);

		void UnlockSocketInfo(TSocketInfo* pTSocketInfo);

		CTcpMultiClient::ERROR_CODE AddSocket(UINT32& SocketID_out, SOCKET aSocket, UINT32 nIP, UINT16 nPort);

		void DeleteSocketUnLock(TSocketInfo* pTSocketInfo, ITcpMultiClientHandler::ERROR_CODE errorCode);

		void DeleteSocket(UINT32 nSocketID, ITcpMultiClientHandler::ERROR_CODE errorCode);

		//////////////////////////////////////////////////////////////////////////
		//工作线程

		//连接数据接收线程处理逻辑
		static DWORD WINAPI ProcRecvThread(LPVOID lpParam);

		//连接数据发送线程处理逻辑
		static DWORD WINAPI ProcSendThread(LPVOID lpParam);

	private:

		bool m_bStop;                                       //停止

		CTcpMultiClient *m_pCTcpMultiClient;

		ITcpMultiClientHandler* m_pHandler;				    //执行细节

		CThreadMutex m_CThreadMutex;						//线程同步

		UINT16 m_nIOWorkerCount;							//工作线程数

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
	};

	//////////////////////////////////////////////////////////////////////////
	//CTcpMultiClientImp 实现

	CTcpMultiClientImp::CTcpMultiClientImp(CTcpMultiClient* pCTcpMultiClient)
	{
		m_bStop = TRUE;

		m_pCTcpMultiClient = pCTcpMultiClient;

		m_pHandler = NULL;

		m_nCurSocketCount = 0;

		m_nMaxSocketCount = 0;

		m_pSocketInfoMem = NULL;

		m_nIOWorkerCount = 0;

		m_hRecvThreadArray = NULL;

		m_hSendThreadArray = NULL;

		m_nRecvBufferSize = 0;

		m_nSendBufferSize = 0;

		m_nSendBufferCount = 0;

		m_hIocp = 0;

        m_pSenBufferQueueMem = NULL;
	}

	CTcpMultiClientImp::~CTcpMultiClientImp()
	{
		this->Stop();
	}

	CTcpMultiClient::ERROR_CODE CTcpMultiClientImp::Start(
		ITcpMultiClientHandler* pITcpMultiClientHandler,
		UINT16 nMaxClientCount,
		UINT16 nIOWorkerCount,
		UINT32 nRecvBufferSize,
		UINT32 nSendBufferSize,
		UINT32 nSendBufferCount)
	{
		if (!m_bStop)
		{
			return CTcpMultiClient::ERROR_CODE_START;
		}

		if (!pITcpMultiClientHandler
			|| 0 == nMaxClientCount
			|| 0 == nIOWorkerCount
			|| 0 == nRecvBufferSize
			|| 0 == nSendBufferSize
			|| 0 == nSendBufferCount)
		{
			return CTcpMultiClient::ERROR_CODE_PARAM;
		}

		m_pHandler = pITcpMultiClientHandler;

		m_nIOWorkerCount = nIOWorkerCount;
	
		m_nCurSocketCount = 0;

		m_nMaxSocketCount = nMaxClientCount;

		m_nRecvBufferSize = nRecvBufferSize;

		m_nSendBufferSize = nSendBufferSize;

		m_nSendBufferCount = nSendBufferCount;

		m_bStop = false;

		if (!this->InitSocketInfos())
		{
			this->Stop();

			return CTcpMultiClient::ERROR_CODE_SYSTEM;
		}

		if (!this->InitSendWorker())
		{
			this->Stop();

			return CTcpMultiClient::ERROR_CODE_SYSTEM; 
		}

		if (!this->InitRecvWorker())
		{
			this->Stop();

			return CTcpMultiClient::ERROR_CODE_SYSTEM; 
		}

		return CTcpMultiClient::ERROR_CODE_SUCCESS;
	}

	void CTcpMultiClientImp::Stop()
	{
		if (m_bStop)
		{
			return;
		}

		m_bStop = true;

		this->CloseRecvWorker();

		this->CloseSendWorker();

		this->CloseSocketInfos();
	}

    CTcpMultiClient::ERROR_CODE CTcpMultiClientImp::Connect(
        UINT32& SocketID_out, 
        UINT32 nIP, 
        UINT16 nPort, 
        UINT32 nWaitTime,           
        bool bSysKeepAlive,
        UINT32 nIntervalTime)
    {
        if (0 == nPort)
        {
            return CTcpMultiClient::ERROR_CODE_PARAM;
        }

        if (bSysKeepAlive
            && 0 == nIntervalTime)
        {
            return CTcpMultiClient::ERROR_CODE_PARAM;
        }

        //////////////////////////////////////////////////////////////////////////
        //执行连接操作

        SOCKET aSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (INVALID_SOCKET == aSocket)
        {
            return CTcpMultiClient::ERROR_CODE_SYSTEM;
        }

        //设置为非阻塞模式
        // If iMode = 0, blocking is enabled; 
        // If iMode != 0, non-blocking mode is enabled.				
        int iMode = 1;
        if (SOCKET_ERROR == ioctlsocket(aSocket, FIONBIO, (u_long FAR*)&iMode))
        {
            closesocket(aSocket);

            aSocket = INVALID_SOCKET;

            return CTcpMultiClient::ERROR_CODE_SYSTEM;
        }

        if (bSysKeepAlive)
        {
            BOOL bKeepAlive = TRUE; 

            if (SOCKET_ERROR == setsockopt(aSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive)))
            {
                closesocket(aSocket);

                aSocket = INVALID_SOCKET;

                return CTcpMultiClient::ERROR_CODE_SYSTEM;
            }

            //设置KeepAlive检测时间和次数 
            SYSBASE_TCP_KEEPALIVE inKeepAlive = {0}; //输入参数 
            unsigned long ulInLen = sizeof(SYSBASE_TCP_KEEPALIVE); 

            SYSBASE_TCP_KEEPALIVE outKeepAlive = {0}; //输出参数 
            unsigned long ulOutLen = sizeof(SYSBASE_TCP_KEEPALIVE); 

            unsigned long ulBytesReturn = 0; 

            inKeepAlive.onoff = 1; 
            inKeepAlive.keepaliveinterval = nIntervalTime; //两次KeepAlive探测间的时间间隔 
            inKeepAlive.keepalivetime = nIntervalTime; //开始首次KeepAlive探测前的TCP空闭时间 

            if (SOCKET_ERROR == WSAIoctl(aSocket, 
                SIO_KEEPALIVE_VALS, 
                (LPVOID)&inKeepAlive, 
                ulInLen, 
                (LPVOID)&outKeepAlive, 
                ulOutLen, 
                &ulBytesReturn, 
                NULL, 
                NULL))
            {
                closesocket(aSocket);

                aSocket = INVALID_SOCKET;

                return CTcpMultiClient::ERROR_CODE_SYSTEM;
            }
        }

        //////////////////////////////////////////////////////////////////////////

        SOCKADDR_IN sockAddr = {0};
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = nIP;
        sockAddr.sin_port = htons(nPort);

        if (SOCKET_ERROR == connect(aSocket, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR_IN)))
        {
            DWORD dwError = WSAGetLastError();

            if (WSAEWOULDBLOCK != dwError)
            {
                closesocket(aSocket);

                aSocket = INVALID_SOCKET;

                return CTcpMultiClient::ERROR_CODE_SYSTEM;
            }
        }

        //////////////////////////////////////////////////////////////////////////
        //等待连接是否成功

        fd_set fdWrite = {0};
        fd_set fdException = {0};

        FD_SET(aSocket, &fdWrite);
        FD_SET(aSocket, &fdException);

        timeval timeout = {0};
        timeout.tv_sec = nWaitTime / 1000;
        timeout.tv_usec = nWaitTime % 1000 * 1000;

        switch (select((int)aSocket + 1, NULL, &fdWrite, &fdException, &timeout))
        {
        case 0:
            {
                closesocket(aSocket);

                aSocket = INVALID_SOCKET;

                return CTcpMultiClient::ERROR_CODE_CONNECT_FAILED;
            }
            break;
        case SOCKET_ERROR :
            {
                closesocket(aSocket);

                aSocket = INVALID_SOCKET;

                return CTcpMultiClient::ERROR_CODE_SYSTEM;
            }
            break;
        default:
            break;
        }

        //////////////////////////////////////////////////////////////////////////

        if (!FD_ISSET(aSocket, &fdWrite))
        {
            closesocket(aSocket);

            aSocket = INVALID_SOCKET;

            return CTcpMultiClient::ERROR_CODE_CONNECT_FAILED;
        }

        //////////////////////////////////////////////////////////////////////////

        CTcpMultiClient::ERROR_CODE errorCode = this->AddSocket(SocketID_out, aSocket, nIP, nPort);

        if (CTcpMultiClient::ERROR_CODE_SUCCESS != errorCode)
        {
            closesocket(aSocket);

            aSocket = INVALID_SOCKET;
        }

        return errorCode;
    }

	CTcpMultiClient::ERROR_CODE CTcpMultiClientImp::Send(
		UINT32 nSocketID,
		const void* pData,
		UINT32 nDataSize)
	{
		if (m_bStop)
		{
			return CTcpMultiClient::ERROR_CODE_STOP;
		}

		if (!pData || 0 == nDataSize)
		{
			return CTcpMultiClient::ERROR_CODE_PARAM;
		}

		if (nDataSize > m_nSendBufferSize)
		{
			return CTcpMultiClient::ERROR_CODE_OVER_SIZE;
		}

        UINT32 nIndex = nSocketID % m_nIOWorkerCount;

        CBufferQueue* pCBufferQueue = (CBufferQueue*)(m_pSenBufferQueueMem + sizeof(CBufferQueue) * nIndex);

		CBufferWrite aCBufferWrite;

		if (CBufferQueue::ERROR_CODE_SUCCESS != pCBufferQueue->GetBufferWrite(aCBufferWrite))
		{
			return CTcpMultiClient::ERROR_CODE_QUEUE_FULL;
		}

		TSendInfo aTSendInfo = {0};
		aTSendInfo.nSocketID = nSocketID;
		aTSendInfo.nDataSize = nDataSize;

		aCBufferWrite.AddData(&aTSendInfo, sizeof(TSendInfo));
		aCBufferWrite.AddData(pData, nDataSize);

		aCBufferWrite.Push();

		return CTcpMultiClient::ERROR_CODE_SUCCESS;
	}

	void CTcpMultiClientImp::CloseSocket(UINT32 nSocketID)
	{
		if (m_bStop)
		{
			return;
		}

		this->DeleteSocket(nSocketID, ITcpMultiClientHandler::ERROR_CODE_USER_CLOSE);
	}

	UINT32 CTcpMultiClientImp::GetSocketCount()
	{
		if (m_bStop)
		{
			return 0;
		}

		return m_nCurSocketCount;
	}

	bool CTcpMultiClientImp::PostRecv(UINT32 nSocketID)
	{
		TSocketInfo* pTSocketInfo = NULL;

		pTSocketInfo = this->FindSocketInfoLock(nSocketID);

		if (!pTSocketInfo)
		{
			return false;
		}

		return this->PostRecvUnlock(pTSocketInfo);
	}

    bool CTcpMultiClientImp::PostRecvUnlock(TSocketInfo* pTSocketInfo)
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
                this->DeleteSocketUnLock(pTSocketInfo, ITcpMultiClientHandler::ERROR_CODE_SYSTEM);
                return false;
            }
        }

        this->UnlockSocketInfo(pTSocketInfo);

        return true;
    }

	bool CTcpMultiClientImp::InitRecvWorker()
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
			m_hRecvThreadArray[nIndex] = CreateThread(NULL, 0, CTcpMultiClientImp::ProcRecvThread, this, 0, NULL);

			if (!m_hRecvThreadArray[nIndex])
			{
				return false;
			}
		}
	
		return true;
	}

	void CTcpMultiClientImp::CloseRecvWorker()
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

	bool CTcpMultiClientImp::InitSendWorker()
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
			m_hSendThreadArray[nIndex] = CreateThread(NULL, 0, CTcpMultiClientImp::ProcSendThread, this, 0, NULL);

			if (!m_hSendThreadArray[nIndex])
			{
				return false;
			}
		}

		return true;
	}

	void CTcpMultiClientImp::CloseSendWorker()
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

	bool CTcpMultiClientImp::InitSocketInfos()
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

	void CTcpMultiClientImp::CloseSocketInfos()
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

	CTcpMultiClientImp::TSocketInfo* CTcpMultiClientImp::FindSocketInfoLock(UINT32 nSocketID)
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

	void CTcpMultiClientImp::UnlockSocketInfo(TSocketInfo* pTSocketInfo)
	{
		if (!pTSocketInfo)
		{
			return;
		}

		pTSocketInfo->aCThreadMutex.Unlock();
	}

	CTcpMultiClient::ERROR_CODE CTcpMultiClientImp::AddSocket(UINT32& SocketID_out, SOCKET aSocket, UINT32 nIP, UINT16 nPort)
	{
		if (INVALID_SOCKET == aSocket)
		{
			return CTcpMultiClient::ERROR_CODE_PARAM;
		}

		TSocketInfo* pTSocketInfo = NULL;

		if (!m_SocketInfoIdleQueue.Pop(pTSocketInfo))
		{
			return CTcpMultiClient::ERROR_CODE_SOCKET_FULL;
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
			this->DeleteSocketUnLock(pTSocketInfo, ITcpMultiClientHandler::ERROR_CODE_SUCCESS);

			return CTcpMultiClient::ERROR_CODE_SYSTEM;
		}

		pTSocketInfo->aCThreadMutex.Unlock();

		//////////////////////////////////////////////////////////////////////////
		//投递接收请求

		if (!this->PostRecv(aTSocketID.nSocketID))
		{
			return CTcpMultiClient::ERROR_CODE_SYSTEM;
		}

        SocketID_out = aTSocketID.nSocketID;

		return CTcpMultiClient::ERROR_CODE_SUCCESS;
	}

	void CTcpMultiClientImp::DeleteSocketUnLock(TSocketInfo* pTSocketInfo, ITcpMultiClientHandler::ERROR_CODE errorCode)
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

        if (ITcpMultiClientHandler::ERROR_CODE_SUCCESS != errorCode)
        {
            m_pHandler->OnClosed(m_pCTcpMultiClient, aTSocketID.nSocketID, nIP, nPort, errorCode);
        }
	}

	void CTcpMultiClientImp::DeleteSocket(UINT32 nSocketID, ITcpMultiClientHandler::ERROR_CODE errorCode)
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

	DWORD WINAPI CTcpMultiClientImp::ProcRecvThread(LPVOID lpParam)
	{
		CTcpMultiClientImp* pCTcpMultiClientImp = (CTcpMultiClientImp*)lpParam;

		char* recvBuffer = new(std::nothrow) char[pCTcpMultiClientImp->m_nRecvBufferSize];

		UINT32 nIP = 0;
		UINT16 nPort = 0;;

		DWORD dwNumberOfBytes = 0;

		ULONG_PTR nReserveInfo = 0;

		UINT32 nSocketID = 0;

		LPOVERLAPPED lpOverLapped = NULL;

		BOOL bSuccess = TRUE;

		UINT32 nError = 0;

		while (!pCTcpMultiClientImp->m_bStop)
		{
			bSuccess = GetQueuedCompletionStatus(pCTcpMultiClientImp->m_hIocp,
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
					pCTcpMultiClientImp->DeleteSocket(nSocketID, ITcpMultiClientHandler::ERROR_CODE_REMOTE_CLOSE);

					continue;
				}

				//////////////////////////////////////////////////////////////////////////
				//处理接收的数据

				TSocketInfo* pTSocketInfo = pCTcpMultiClientImp->FindSocketInfoLock(nSocketID);

                if (pTSocketInfo)
                {
                    pTSocketInfo->nRecvDataSize += dwNumberOfBytes;

                    if (pTSocketInfo->nRecvDataSize > pCTcpMultiClientImp->m_nRecvBufferSize)   //协议错误
                    {
                        pCTcpMultiClientImp->DeleteSocketUnLock(pTSocketInfo, ITcpMultiClientHandler::ERROR_CODE_PROTOCOL);
                    }
                    else
                    {
                        while (pTSocketInfo)
                        {
                            if (pTSocketInfo->nRecvDataSize >= pCTcpMultiClientImp->m_pHandler->GetHeadSize())
                            {
                                UINT32 nMsgSize = pCTcpMultiClientImp->m_pHandler->GetMsgSize(pTSocketInfo->recvBuffer, pTSocketInfo->nRecvDataSize);

                                //协议错误
                                if (nMsgSize > pCTcpMultiClientImp->m_nRecvBufferSize ||  nMsgSize < pCTcpMultiClientImp->m_pHandler->GetHeadSize())
                                {
                                    pCTcpMultiClientImp->DeleteSocketUnLock(pTSocketInfo, ITcpMultiClientHandler::ERROR_CODE_PROTOCOL);

                                    pTSocketInfo =  NULL;
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

                                        pCTcpMultiClientImp->UnlockSocketInfo(pTSocketInfo);

                                        pCTcpMultiClientImp->m_pHandler->OnReceived(pCTcpMultiClientImp->m_pCTcpMultiClient, nSocketID, nIP, nPort, recvBuffer, nMsgSize);

                                        pTSocketInfo = pCTcpMultiClientImp->FindSocketInfoLock(nSocketID);

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
                            pCTcpMultiClientImp->PostRecvUnlock(pTSocketInfo);
                        }
                    }
                }
			}
			else
			{
				//投递WSARecv获得false则是本地SOCKET主动断开或SOCKET已经不存在
				pCTcpMultiClientImp->DeleteSocket(nSocketID, ITcpMultiClientHandler::ERROR_CODE_REMOTE_CLOSE);
			}
		}

		delete [] recvBuffer;

		return 0;
	}

	DWORD WINAPI CTcpMultiClientImp::ProcSendThread(LPVOID lpParam)
	{
		CTcpMultiClientImp* pCTcpMultiClientImp = (CTcpMultiClientImp*)lpParam;

        CBufferQueue* pCBufferQueue = NULL;

        if (!pCTcpMultiClientImp->m_sendBuufferQueueObjQueue.Pop(pCBufferQueue))
        {
            return 0;
        }

		while (!pCTcpMultiClientImp->m_bStop)
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

            TSocketInfo* pTSocketInfo = pCTcpMultiClientImp->FindSocketInfoLock(pTSendInfo->nSocketID);

			while (nHasSendSize < nTotalSendSize && pTSocketInfo)
			{
				nCurSendSize = send(pTSocketInfo->aSocket, pSendData + nHasSendSize, nTotalSendSize - nHasSendSize, 0); 

				if (SOCKET_ERROR == nCurSendSize)
				{
					DWORD dwError = WSAGetLastError();

					if (WSAEWOULDBLOCK != dwError)
					{
                        ITcpMultiClientHandler::ERROR_CODE errorCode = ITcpMultiClientHandler::ERROR_CODE_SYSTEM;

                        switch (dwError)
                        {
                        case WSAECONNABORTED:
                        case WSAECONNRESET:
                            {
                                errorCode = ITcpMultiClientHandler::ERROR_CODE_REMOTE_CLOSE;
                            }
                            break;
                        }

                        pCTcpMultiClientImp->DeleteSocketUnLock(pTSocketInfo, errorCode);
						
						bSendSuccess = false;

						break;
					}

                    pCTcpMultiClientImp->UnlockSocketInfo(pTSocketInfo);

                    pTSocketInfo = pCTcpMultiClientImp->FindSocketInfoLock(pTSendInfo->nSocketID);

                    continue;
				}
                else if (nCurSendSize > 0)
                {
                    nHasSendSize += nCurSendSize;
                }
                else    //nCurSendSize == 0
                {
                    pCTcpMultiClientImp->UnlockSocketInfo(pTSocketInfo);

                    pTSocketInfo = pCTcpMultiClientImp->FindSocketInfoLock(pTSendInfo->nSocketID);
                }
			}
			
			if (bSendSuccess && pTSocketInfo)
			{
				pCTcpMultiClientImp->UnlockSocketInfo(pTSocketInfo);
			}
		}

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//CTcpMultiClient

	CTcpMultiClient::CTcpMultiClient()
	{
		m_Imp = new CTcpMultiClientImp(this);
	}

	CTcpMultiClient::~CTcpMultiClient()
	{
		if (m_Imp)
		{
			CTcpMultiClientImp* pImp = (CTcpMultiClientImp*)m_Imp;

			delete pImp;

			m_Imp = NULL;
		}
	}

	CTcpMultiClient::ERROR_CODE CTcpMultiClient::Start(
		ITcpMultiClientHandler* pITcpMultiClientHandler,
		UINT16 nMaxClientCount,
		UINT16 nIOWorkerCount,
		UINT32 nRecvBufferSize,
		UINT32 nSendBufferSize,
		UINT32 nSendBufferCount)
	{
		if (!m_Imp)
		{
			return ERROR_CODE_SYSTEM;
		}

		if (!CSocketLoader::InitSocket())
		{
			return ERROR_CODE_SYSTEM;
		}

		CTcpMultiClientImp* pImp = (CTcpMultiClientImp*)m_Imp;

		return pImp->Start(pITcpMultiClientHandler,
			nMaxClientCount,
			nIOWorkerCount,
			nRecvBufferSize,
			nSendBufferSize,
			nSendBufferCount);
	}

	void CTcpMultiClient::Stop()
	{
		if (!m_Imp)
		{
			return;
		}

		CTcpMultiClientImp* pImp = (CTcpMultiClientImp*)m_Imp;

		pImp->Stop();
	}

    CTcpMultiClient::ERROR_CODE CTcpMultiClient::Connect(
        UINT32& SocketID_out, 
        UINT32 nIP, 
        UINT16 nPort, 
        UINT32 nWaitTime,
        bool bSysKeepAlive,
        UINT32 nIntervalTime)
    {
        if (!m_Imp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CTcpMultiClientImp* pImp = (CTcpMultiClientImp*)m_Imp;

        return pImp->Connect(SocketID_out, nIP, nPort, nWaitTime, bSysKeepAlive, nIntervalTime);
    }

	CTcpMultiClient::ERROR_CODE CTcpMultiClient::Send(
		UINT32 nSocketID,
		const void* pData,
		UINT32 nDataSize)
	{
		if (!m_Imp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CTcpMultiClientImp* pImp = (CTcpMultiClientImp*)m_Imp;

		return pImp->Send(nSocketID, pData, nDataSize);
	}

	void CTcpMultiClient::CloseSocket(UINT32 nSocketID)
	{
		if (!m_Imp)
		{
			return;
		}

		CTcpMultiClientImp* pImp = (CTcpMultiClientImp*)m_Imp;

		pImp->CloseSocket(nSocketID);
	}

	UINT32 CTcpMultiClient::GetSocketCount()
	{
		if (!m_Imp)
		{
			return 0;
		}

		CTcpMultiClientImp* pImp = (CTcpMultiClientImp*)m_Imp;

		return pImp->GetSocketCount();
	}
}