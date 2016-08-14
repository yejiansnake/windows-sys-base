#pragma once

#include "SysBase.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//类型声明

	class CTcpServer;

	//////////////////////////////////////////////////////////////////////////  
	//ITcpServerHandler : CTcpServer 的执行细节

	class ITcpServerHandler
	{
	public:

		//////////////////////////////////////////////////////////////////////////

		enum ERROR_CODE
		{
			ERROR_CODE_SUCCESS = 0,
			ERROR_CODE_USER_CLOSE,				//业务部分主动关开
			ERROR_CODE_SYSTEM_CLOSE,			//系统主动关闭
			ERROR_CODE_REMOTE_CLOSE,			//远程连接主动关闭
			ERROR_CODE_SOCKET_FULL,				//连接数满
			ERROR_CODE_KEEP_ALIVE_TIME_OUT,		//心跳超时
			ERROR_CODE_SYSTEM,					//系统内部错误
            ERROR_CODE_PROTOCOL,                //协议错误
		};

		//////////////////////////////////////////////////////////////////////////

		ITcpServerHandler(){};

		virtual ~ITcpServerHandler(){};

		//////////////////////////////////////////////////////////////////////////
		//接口函数，需要重载

		virtual UINT32 GetHeadSize() = 0;

		virtual UINT32 GetMsgSize(const char* pData, UINT32 nDataSize) = 0;

		//事件：接收到新连接 
		//返回值: true:正常;false:放弃当前接收到的新连接
		virtual bool OnAccepted(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort){ return true; };

		virtual void OnAcceptedError(CTcpServer* pCTcpServer, UINT32 nIP, UINT16 nPort, ERROR_CODE errorCode){};

		virtual void OnReceived(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, const char* pData, UINT nDataSize){};

		virtual void OnClosed(CTcpServer* pCTcpServer, UINT32 nSocketID, UINT32 nIP, UINT16 nPort, ERROR_CODE errorCode){};
	};

	//////////////////////////////////////////////////////////////////////////
	//CTcpServer 

	class CTcpServer : public CObject 
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
		};

		enum KEEP_ALIVE_TYPE
		{
			KEEP_ALIVE_TYPE_NONE = 0,	//不打开心跳
			KEEP_ALIVE_TYPE_SYSTEM,		//系统心跳功能
			KEEP_ALIVE_TYPE_DATA		//通过接收数据包方式作为心跳功能
		};

		//////////////////////////////////////////////////////////////////////////

		CTcpServer();

		virtual ~CTcpServer();

		//////////////////////////////////////////////////////////////////////////

		ERROR_CODE Start(
			ITcpServerHandler* pITcpServerHandler,
			UINT32 nListenIP,
			UINT16 nPort,
			UINT16 nMaxConnectCount,
			UINT16 nIOWorkerCount,
			UINT32 nRecvBufferSize,
			UINT32 nSendBufferSize,
			UINT32 nSendBufferCount, 
			KEEP_ALIVE_TYPE eKeepAliveType = KEEP_ALIVE_TYPE_NONE,
			UINT32 nKeepAliveIntervalTime = 0);

        ERROR_CODE Start(
            ITcpServerHandler* pITcpServerHandler,
            PCCHAR szListenIP,
            UINT16 nPort,
            UINT16 nMaxConnectCount,
            UINT16 nIOWorkerCount,
            UINT32 nRecvBufferSize,
            UINT32 nSendBufferSize,
            UINT32 nSendBufferCount, 
            KEEP_ALIVE_TYPE eKeepAliveType = KEEP_ALIVE_TYPE_NONE,
            UINT32 nKeepAliveIntervalTime = 0);

		void Stop();

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
