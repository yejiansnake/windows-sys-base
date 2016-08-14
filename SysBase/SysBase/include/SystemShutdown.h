#pragma once

#include "SysBase.h"

namespace SysBase
{
    class CSystemShutdown
    {
    public:

        //退出类型
        enum EExitFlag
        {
            EXIT_FLAG_LOGOFF = 1,       //用户登出（注销）
            EXIT_FLAG_POWER_OFF = 2,    //系统关闭并关闭电源
            EXIT_FLAG_REBOOT = 4,       //系统关闭并重新启动
            EXIT_FLAG_SHUTDOWN = 8,     //系统关闭，但不关闭电源（类似休眠）
        };
    
        //退出系统
        //参数：
        //      UINT32 nFlags IN 退出标志，数值为 EExitFlag 中的其中一个或多个，多个用 | 操作合并
        static bool ExitSystem(UINT32 nFlags);

        //关闭系统
        //参数:
        //      PCWCHAR szMsg IN
        //      UINT32 nTimeout IN
        //      bool bForceAppsClosed IN
        //      bool bRebootAfterShutdown IN 
        static bool ShutdownSystem(PCWCHAR szMsg, UINT32 nTimeout, bool bForceAppsClosed, bool bRebootAfterShutdown);

        //取消关闭：在没有调用强制关闭并关闭电源而显示关闭提示确认的情况下有用
        static bool CancelShutdown();

        //锁屏
        static bool LockSystem();
    };
}