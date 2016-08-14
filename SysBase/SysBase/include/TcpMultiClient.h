#pragma once

#include "SysBase.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//类型声明

	class CTcpMultiClient;

	//////////////////////////////////////////////////////////////////////////  
	//ITcpMultiClientHandler 的执行细节

	class ITcpMultiClientHandler
	{
	public:

		//////////////////////////////////////////////////////////////////////////

		enum ERROR_CODE
		{
			ERROR_CODE_SUCCESS = 0,
			ERROR_CODE_USER_CLOSE,				//业务部分主动关开
			ERROR_CODE_SYSTEM_CLOSE,			//系统主动关闭
			ERROR_CODE_REMOTE_CLOSE,			//远程连接主动关闭
			ERROR_CODE_SYSTEM,					//系统内部错误
            ERROR_CODE_PROTOCOL,                //协议错误
		};

		//////////////////////////////////////////////////////////////////////////

		ITcpMultiClientHandler(){};

		virtual ~ITcpMultiClientHandler(){};

		//////////////////////////////////////////////////////////////////////////
		//接口函数，需要重载

		virtual UINT32 GetHeadSize() = 0;

		virtual UINT32 GetMsgSize(const char* pData, UINT32 pDataSize) = 0;

		virtual void OnReceived(CTcpMultiClient* pCTcpMultiClient, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, const char* pData, UINT nDataSize){};

		virtual void OnClosed(CTcpMultiClient* pCTcpMultiClient, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, ERROR_CODE errorCode){};
	};

	//////////////////////////////////////////////////////////////////////////
	//CTcpMultiClient

	class CTcpMultiClient : public CObject 
	{
	public:

		//////////////////////////////////////////////////////////////////////////

		enum ERROR_CODE
		{
			ERROR_CODE_SUCCESS = 0,
			ERROR_CODE_PARAM,
			ERROR_CODE_SYSTEM,			//系统级别错误
			ERROR_CODE_STOP,			//对象未启动或已停止
			ERROR_CODE_START,			//对象已启动
			ERROR_CODE_QUEUE_FULL,		//队列满
			ERROR_CODE_OVER_SIZE,		//发送的数据大于发送缓冲区大小
			ERROR_CODE_SOCKET_FULL,		//连接数满
            ERROR_CODE_CONNECT_FAILED,  //连接失败
		};

		//////////////////////////////////////////////////////////////////////////

		CTcpMultiClient();

		virtual ~CTcpMultiClient();

		//////////////////////////////////////////////////////////////////////////

		ERROR_CODE Start(
			ITcpMultiClientHandler* pITcpMultiClientHandler,
			UINT16 nMaxClientCount,
			UINT16 nIOWorkerCount,
			UINT32 nRecvBufferSize,
			UINT32 nSendBufferSize,
			UINT32 nSendBufferCount);

		void Stop();

        ERROR_CODE Connect(
            UINT32& SocketID_out, 
            UINT32 nIP, 
            UINT16 nPort, 
            UINT32 nWaitTime = 3000,
            bool bSysKeepAlive = false,
            UINT32 nIntervalTime = 20000);

		ERROR_CODE Send(
			UINT32 nSocketID,
			const void* pData,
			UINT32 nDataSize);

		void CloseSocket(UINT32 nSocketID);

		UINT32 GetSocketCount();

	private:

		void* m_Imp;
	};
}