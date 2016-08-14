#pragma once

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //声明

    class CTcpClient;

    //////////////////////////////////////////////////////////////////////////
    //ITcpClientHandler 的执行细节

    class ITcpClientHandler
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

        ITcpClientHandler(){};
        virtual ~ITcpClientHandler(){};

        //////////////////////////////////////////////////////////////////////////
        //接口函数，需要重载

        //功能:获取接收到消息头大小 （由CAsynTcpServer调用） [一定需要重载]
        //返回值:消息头大小;如果返回 0 则CTcpClient对象不缓存数据，收到包直接通知 OnReceive
        virtual UINT32 GetHeadSize() = 0;

        //功能:获取当前接收到消息总大小 （由CAsynTcpServer调用） [一定需要重载]
        //返回值:消息大小
        virtual UINT32 GetMsgSize(const char* pData, UINT32 nDataSize) = 0;

        //事件:接收到消息 [一定需要重载]
        virtual void OnReceived(CTcpClient* pCTcpClient, const char* pData, UINT32 nSize) = 0;

        //事件:连接前
        //返回值：true 执行连接; false 不连接
        virtual bool OnConnecting(CTcpClient* pCTcpClient, UINT32& nIP, UINT16& nPort) = 0;

        //事件:连接成功
        virtual void OnConnected(CTcpClient* pCTcpClient, UINT32 nIP, UINT16 nPort){};

        //事件:连接失败
        virtual void OnConnectFailed(CTcpClient* pCTcpClient, UINT32 nIP, UINT16 nPort){};

        //事件:连接关闭
        virtual void OnClosed(CTcpClient* pCTcpClient, ERROR_CODE errorCode){};

        //事件:间隔时间(只在连接成功后触发)
        virtual void OnTime(CTcpClient* pCTcpClient){};
    };

    //////////////////////////////////////////////////////////////////////////
    //CTcpClient SOCKE 客户端类

    class CTcpClient : public SysBase::CObject
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
            ERROR_CDOE_NO_CONNECT,      //未成功连接服务器
            ERROR_CODE_QUEUE_FULL,		//队列满
            ERROR_CODE_OVER_SIZE,		//发送的数据大于发送缓冲区大小
        };

        //////////////////////////////////////////////////////////////////////////

        CTcpClient();

        virtual ~CTcpClient();
        
        //功能:启动
        //参数:
        //      ITcpClientHandler* pHandler IN 执行细节
        //		UINT32 nRecvBufferSize IN 接收缓冲区大小
        //		UINT32 nSendBufferSize IN 发送缓冲区大小
        //      UINT32 nSendBufferCount IN 发送缓冲区队列数
        //		UINT32 nIntervalOnTime IN 间隔时间（触发 OnTime 事件）
        //      bool bSysKeepAlive IN 是否启动系统心跳功能
        //      UINT32 nIntervalTime IN 打开心跳功能后，两次心跳间隔的时间[单位为毫秒]
        //      UINT32 nConnectIntervalTime IN 两次连接的间隔时间[单位为毫秒]
        ERROR_CODE Start(
            ITcpClientHandler* pHandler,
            UINT32 nRecvBufferSize,
            UINT32 nSendBufferSize,
            UINT32 nSendBufferCount,
            UINT32 nIntervalOnTime = 1000,
            bool bSysKeepAlive = false,
            UINT32 nIntervalTime = 0,
            UINT32 nConnectIntervalTime = 1);

        //功能:停止
        void Stop();

        //功能:投递异步发送信息
        //参数:
        //      const void* pData IN 发送的缓冲区首地址
        //      UINT32 nDataSize IN 发送缓冲区大小
        //返回值:投递发送成功或否
        ERROR_CODE Send(
            const void* pData, 
            UINT32 nDataSize);
        
        //连接是否成功
        bool IsConnected();

        //功能:主动断开连接（程序会再自动重连）
        void Abort();

    private:

        void* m_pImp;

    };
}
