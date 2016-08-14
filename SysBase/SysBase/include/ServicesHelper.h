#pragma once

#include "SysBase.h"

namespace SysBase
{
    //服务启动类型
    enum SYSBASE_SERVICE_RUN_TYPE
    {
        SYSBASE_SERVICE_RUN_TYPE_AUTO_START = 0,      //自动启动
        SYSBASE_SERVICE_RUN_TYPE_DEMAND_START = 1,    //手动启动
        SYSBASE_SERVICE_RUN_TYPE_DISABLED = 2,        //禁用
        SYSBASE_SERVICE_RUN_TYPE_BOOT_START =3,       //驱动启动
        SYSBASE_SERVICE_RUN_TYPE_SYSTEM_START = 4,    //系统启动
    };

    //服务当前运行状态
    enum SYSBASE_SERVICE_STATE
    {
        SYSBASE_SERVICE_STATE_CONTINUE_PENDING = 0,     //继续等待
        SYSBASE_SERVICE_STATE_PAUSE_PENDING = 1,        //暂停等待
        SYSBASE_SERVICE_STATE_PAUSED = 2,               //已暂停
        SYSBASE_SERVICE_STATE_RUNNING = 3,              //运行中
        SYSBASE_SERVICE_START_PENDING = 4,              //开始等待
        SYSBASE_SERVICE_STATE_STOP_PENDING = 5,         //停止等待
        SYSBASE_SERVICE_STATE_STOPPED = 6,              //已停止
    };

    //服务类型
    enum SYSBASE_SERVICE_TYPE
    {
        SYSBASE_SERVICE_TYPE_KERNEL_DRIVER = 1,             //核心驱动服务
        SYSBASE_SERVICE_TYPE_FILE_SYSTEM_DRIVER = 2,        //文件系统驱动服务
        SYSBASE_SERVICE_TYPE_WIN32_OWN_PROCESS = 10,        //WIN32一般进程
        SYSBASE_SERVICE_TYPE_WIN32_SHARE_PROCESS = 16,      //WIN32共享进程
        SYSBASE_SERVICE_TYPE_INTERACTIVE_PROCESS = 256,     //互动进程（暂时不知道什么意思）
    };

    struct TServiceInfo
    {
        TServiceInfo()
        {
            runType = SYSBASE_SERVICE_RUN_TYPE_DISABLED;
            curState = SYSBASE_SERVICE_STATE_STOPPED;
            nServicesType = SYSBASE_SERVICE_TYPE_WIN32_OWN_PROCESS;
            nProcessID = 0;
        }

        string strServiceName;                  //服务名
        string strDisplayName;                  //服务显示名
        string strBinaryPathName;               //服务对应的进程完整路径和命令行
        string strStartName;                    //启动服务的用户名
        SYSBASE_SERVICE_RUN_TYPE runType;       //服务启动类型
        SYSBASE_SERVICE_STATE curState;         //服务当前状态
        UINT32 nServicesType;                   //服务类型（SYSBASE_SERVICE_TYPE）
        UINT32 nProcessID;                      //对应的进程ID
    };

    struct TServiceInfoW
    {
        TServiceInfoW()
        {
            runType = SYSBASE_SERVICE_RUN_TYPE_DISABLED;
            curState = SYSBASE_SERVICE_STATE_STOPPED;
            nServicesType = SYSBASE_SERVICE_TYPE_WIN32_OWN_PROCESS;
            nProcessID = 0;
        }

        wstring strServiceName;                 //服务名
        wstring strDisplayName;                 //服务显示名
        wstring strBinaryPathName;              //服务对应的进程完整路径和命令行
        wstring strStartName;                   //启动服务的用户名
        SYSBASE_SERVICE_RUN_TYPE runType;       //服务启动类型
        SYSBASE_SERVICE_STATE curState;         //服务当前状态
        UINT32 nServicesType;                   //服务类型（SYSBASE_SERVICE_TYPE）
        UINT32 nProcessID;                      //对应的进程ID
    };

    class CServicesHelper
    {
    public:

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS,
            ERROR_CODE_PARAM,               //参数错误
            ERROR_CODE_SYSTEM,              //逻辑错误
            ERROR_CODE_INVALID_HANDLE,      //无效的句柄
        };

        static bool GetServiceInfo(list<TServiceInfo>& serviceInfoList);
        static bool GetServiceInfo(list<TServiceInfoW>& serviceInfoList);

        static ERROR_CODE GetServiceInfo(const char* szServiceName, TServiceInfo& serviceInfo);
        static ERROR_CODE GetServiceInfo(const WCHAR* szServiceName, TServiceInfoW& serviceInfo);

        static ERROR_CODE InstallService(const char* szServiceName,const char* szDisplayName, const char* szExePath);
        static ERROR_CODE InstallService(const WCHAR* szServiceName,const WCHAR* szDisplayName, const WCHAR* szExePath);

        static ERROR_CODE UnInstallService(const char* szServiceName);
        static ERROR_CODE UnInstallService(const WCHAR* szServiceName);

        static bool IsServiceExist(const char* szServiceName);
        static bool IsServiceExist(const WCHAR* szServiceName);

        static ERROR_CODE RunService(const char* szServiceName);
        static ERROR_CODE RunService(const WCHAR* szServiceName);

        static ERROR_CODE StopService(const char* szServiceName);
        static ERROR_CODE StopService(const WCHAR* szServiceName);

        static ERROR_CODE PauseService(const char* szServiceName);
        static ERROR_CODE PauseService(const WCHAR* szServiceName);

        static ERROR_CODE ContinueService(const char* szServiceName);
        static ERROR_CODE ContinueService(const WCHAR* szServiceName);

    private:

        CServicesHelper();

        virtual ~CServicesHelper();
    };
}
