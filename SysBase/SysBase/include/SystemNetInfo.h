#pragma once

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////

    enum SYSBASE_TCP_STATUS : UINT32
    {
        SYSBASE_TCP_STATUS_CLOSED = 1,              //The TCP connection is closed.（关闭）
        SYSBASE_TCP_STATUS_LISTEN = 2,              //The TCP connection is in the listen state.（监听）
        SYSBASE_TCP_STATUS_SYN_SENT = 3,            //A SYN packet has been sent.
        SYSBASE_TCP_STATUS_SYN_RCVD = 4,            //A SYN packet has been received.
        SYSBASE_TCP_STATUS_ESTAB = 5,               //The TCP connection has been established. (连接到远程端口)
        SYSBASE_TCP_STATUS_FIN_WAIT1 = 6,           //The TCP connection is waiting for a FIN packet.
        SYSBASE_TCP_STATUS_FIN_WAIT2 = 7,           //The TCP connection is waiting for a FIN packet.
        SYSBASE_TCP_STATUS_CLOSE_WAIT = 8,          //The TCP connection is in the close wait state.（等待关闭）
        SYSBASE_TCP_STATUS_CLOSING = 9,             //The TCP connection is closing.（关闭中）
        SYSBASE_TCP_STATUS_LAST_ACK = 10,           //The TCP connection is in the last ACK state.
        SYSBASE_TCP_STATUS_TIME_WAIT = 11,          //The TCP connection is in the time wait state.（连接超时状态）
        SYSBASE_TCP_STATUS_DELETE_TCB = 12          //The TCP connection is in the delete TCB state.
    };

    //////////////////////////////////////////////////////////////////////////

    struct TTcpNetInfo
    {
        UINT32 nProcessID;
        UINT32 nLocalIP;
        UINT32 nRemoteIP;
        UINT16 nLocalPort;
        UINT16 nRemotePort;
        UINT32 nStatus;
    };

    struct TUdpNetInfo
    {
        UINT32 nProcessID;
        UINT32 nLocalIP;
        UINT16 nLocalPort;
    };

    //////////////////////////////////////////////////////////////////////////
    /*

    通过 GetTcpNetInfoList 获取当前TCP信息

    通过 GetUdpNetInfoList 获取当前UDP信息

    再通过 CSysBase::CProcessHelper::GetAllProcessInfo 获取所有进程信息

    比对相同的 PID 即可获取 szExeFile 

    */
    //////////////////////////////////////////////////////////////////////////

    class CSystemNetInfo
    {
    public:

        static bool GetTcpNetInfoList(list<TTcpNetInfo>& tcpNetInfoList);

        static bool GetUdpNetInfoList(list<TUdpNetInfo>& udpNetInfoList);

    protected:
    private:

        CSystemNetInfo();

        virtual ~CSystemNetInfo();
    };
}