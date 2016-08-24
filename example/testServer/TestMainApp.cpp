#define _WIN32_WINNT 0x0502

#include <iostream>
#include "SnmpThread.h"

#define TEST_DATA_SIZE 1024

typedef struct MSG_HEAD 
{
	DWORD dwSize;
}*PMSG_HEAD;

typedef struct MSG_BODY
{
	char szTest[100];
}*PMSG_BODY;

static LONG nCount = 0;

SysBase::CTcpServer aServer;

class CMyTcpServerHandler : public SysBase::ITcpServerHandler
{
public:

    virtual UINT32 GetHeadSize()
    {
        return sizeof(MSG_HEAD);
    }

	virtual UINT32 GetMsgSize(const char* pData, UINT32 nDataSize)
    {
        MSG_HEAD *pHead = (MSG_HEAD *)pData;

        return pHead->dwSize;
    }

	virtual bool OnAccepted(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort)
    {
		CHAR szIP[50] = {0};

		SysBase::CSocketAddr::ToStringIP(szIP, 50, nIP);

        printf("[当接收新连接时] 连接数：%ld, SocketID:%ld, IP:%s, Port:%d \r\n", 
			InterlockedIncrement(&nCount), nSocketID, szIP, nPort);

		return true;
    }

	virtual void OnReceived(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, const char* pData, UINT nDataSize)
    {
		CSnmpThread::Instance().Add(CSnmpThread::SNMP_TYPE_RECV);
    
		int nCount = 1000;
		int nSleep = 1;

		char pSendBuff[TEST_DATA_SIZE] = {0};

		MSG_HEAD *pHead = (MSG_HEAD *)pSendBuff;

		MSG_BODY *pBody = (MSG_BODY *)(pSendBuff + sizeof(MSG_HEAD));

		pHead->dwSize = sizeof(MSG_HEAD) + sizeof(MSG_BODY);

		try
		{
			pCTcpServer->Send(nSocketID, pSendBuff, pHead->dwSize);

			CSnmpThread::Instance().Add(CSnmpThread::SNMP_TYPE_SEND);
		}
		catch (...)
		{
			int a = 1;
		}

		return;

		while (TRUE)
		{
			for (int nIndex = 0; nIndex < nCount; ++nIndex)
			{
				//strcpy(pBody->szTest, "服务器的信息");
				try
				{
					pCTcpServer->Send(nSocketID, pSendBuff, pHead->dwSize);

					CSnmpThread::Instance().Add(CSnmpThread::SNMP_TYPE_SEND);
				}
				catch (...)
				{
					int a = 1;
				}
			}

			Sleep(1);
		}
    }

	virtual void OnClosed(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, ERROR_CODE errorCode)
    {
        printf("[当关闭连接时]SocketID:%ld \r\n", nSocketID);
    }
};

static CMyTcpServerHandler s_CMyTcpServerHandler;

void main()
{

    SysBase::CSocketLoader::InitSocket();

	CSnmpThread::Instance().Start();

    aServer.Start(
		&s_CMyTcpServerHandler, 
		INET_ADDR_ANY_IP, 
		6543, 
		10,
		5,
		TEST_DATA_SIZE,
		TEST_DATA_SIZE,
		100,
		SysBase::CTcpServer::KEEP_ALIVE_TYPE_DATA,
		30 * 1000);

    while (1)
    {
        Sleep(INFINITE);
    }

    aServer.Stop();
}