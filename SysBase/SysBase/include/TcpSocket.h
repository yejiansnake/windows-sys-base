#pragma once

#include "SysBase.h"
#include "Socket.h"

namespace SysBase
{
    class CTcpSocket : public CSocket
    {
    public:

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_PARAM,           //参数错误
            ERROR_CODE_NO_CONNECT,      //没有连接
            ERROR_CODE_HAS_CONNECT,     //已经连接
            ERROR_CODE_DISCONNECT,      //连接中断或断开
            ERROR_CODE_HAS_CLOSE,       //连接已经关闭
            ERROR_CODE_REMOTE_CLOSE,    //远程连接关闭
            ERROR_CODE_SYSTEM,			//系统级别错误
            ERROR_CODE_CONNECT_FAILED,  //连接失败
            ERROR_CODE_TIMEOUT,         //超时
            ERROR_CODE_RECV_NO_DATA,    //没有接收到任何数据
            ERROR_CODE_RECV_OVER_SIZE,  //接收到数据大于提交的缓冲区数据
            ERROR_CODE_PROTOCOL,        //协议错误
        };

        CTcpSocket();

        virtual ~CTcpSocket();

        ERROR_CODE Connect(const char* szIP, UINT16 nPort, UINT32 nWaitTime = INFINITE);

        ERROR_CODE Connect(UINT32 nIP, UINT16 nPort, UINT32 nWaitTime = INFINITE);

        void Disconnect();

        bool IsConnected();

        ERROR_CODE Send(const void* pData, UINT32 nSize);

        //接收数据
        ERROR_CODE Recv(void* pBuffer, UINT32 nBufferSize, UINT32& nRecvSzie, UINT32 nWaitTime = INFINITE);

        //通过自定义的组包协议方式接收完整数据(默认协议包的大小字段都在包头的前4个字节)
        ERROR_CODE RecvEx(void* pBuffer, UINT32 nBufferSize, UINT32& nRecvSzie, UINT32 nWaitTime = INFINITE);

    private:

        bool m_bConnected;
    };
}
