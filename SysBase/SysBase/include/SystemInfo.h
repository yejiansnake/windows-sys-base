#pragma once

#include "SysBase.h"

namespace SysBase
{
    class CSystemInfo
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        enum SYSTEM_VER
        {
            SYSTEM_VER_NONSUPPORT = 0,      //不支持版本
            SYSTEM_VER_XP,                  //Windows XP 32位
            SYSTEM_VER_XP_64,               //Windows XP Professional x64 Edition
            SYSTEM_VER_2003,                //Windows Server 2003
            SYSTEM_VER_HOME_SRV,            //Windows Home Server
            SYSTEM_VER_2003_R2,             //Windows Server 2003 R2
            SYSTEM_VER_VISTA,               //Windows Vista
            SYSTEM_VER_2008,                //Windows Server 2008
            SYSTEM_VER_2008_R2,             //Windows Server 2008 R2
            SYSTEM_VER_WIN7,                //Windows 7
            SYSTEM_VER_2012,                //Windows Server 2012
            SYSTEM_VER_WIN8,                //Windows 8
        };

        enum SYSTEM_VER_SUITE
        {
            SYSTEM_VER_SUITE_UNKNOWN = 0,       //未知版本
            SYSTEM_VER_SUITE_WEB,               //WEB服务器版本
            SYSTEM_VER_SUITE_CLUSTER,           //计算机集群版本
            SYSTEM_VER_SUITE_DATACENTER,        //数据中心版本
            SYSTEM_VER_SUITE_ENTERPRISE,        //企业版本
            SYSTEM_VER_SUITE_EMBEDDEDNT,        //嵌入式版本
            SYSTEM_VER_SUITE_PERSONAL,          //个人版本
            SYSTEM_VER_SUITE_STORAGE,           //存储服务版本
            SYSTEM_VER_SUITE_HOME_SRV,          //家庭服务版本
        };

        enum SYSTEM_NET_JOIN_TYPE
        {
            SYSTEM_NET_JOIN_TYPE_UN_JOIN = 0,
            SYSTEM_NET_JOIN_TYPE_WORK_GROUP,    //工作组
            SYSTEM_NET_JOIN_TYPE_DOMAIN,        //域
        };

        //////////////////////////////////////////////////////////////////////////

        static SYSTEM_VER GetSystemVersion();

        static SYSTEM_VER_SUITE GetSystemVerSuite();

        static void GetCurComputerName(string& strName);

        static void GetCurComputerName(wstring& strName);

        //功能：获取本机加入的工作组或域的名称
        //参数:
        //      wstring& strMame OUT 加入的工作组或域的名称｛当返回值不等于 SYSTEM_NET_JOIN_TYPE_UN_JOIN 时｝
        //返回值：加入的网络状态情况
        static SYSTEM_NET_JOIN_TYPE GetNetJoinName(wstring& strMame);

        static bool Is64bitSystem();

        //RemoteDesktopService
        static bool IsRDPRunOn();

    protected:
    private:
    };
}
