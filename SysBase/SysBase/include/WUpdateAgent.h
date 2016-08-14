#pragma once

#include "SysBase.h"
#include <Wuapi.h>
#pragma comment(lib, "Wuguid.lib")
#pragma comment(lib, "Advapi32.lib")

namespace SysBase
{
    //该类底层实现为COM，最好把实例生成在某个具体的线程下一直保留，以后每次调用即可
    //重复初始化释放COM CoInitialize 与 CoUninitialize 可能会存在效率低问题
    class CWUpdateAgent
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_SYSTEM,          //内部错误
            ERROR_CODE_INIT_FAILED,     //初始化失败
            ERROR_CODE_PARAM,           //参数错误
        };
   
        //////////////////////////////////////////////////////////////////////////

        enum SERVICE_STATUS
        {
            SERVICE_STATUS_ON = 0,  //服务开启
            SERVICE_STATUS_OFF,     //服务关闭
        };

        enum NOTIFY_TYPE
        {
            NOTIFY_TYPE_NOT_CONFIGURED = 0,     //没有配置自动更新由用户或组策略管理员。提示用户定期来配置自动更新。
            NOTIFY_TYPE_DISABLED,               //自动更新将被禁用。不会通知用户的计算机的重要更新。
            NOTIFY_TYPE_BEFORE_DOWNLOAD,        //自动更新提示用户批准更新之前下载或安装
            NOTIFY_TYPE_BEFORE_INSTALL,         //自动更新自动下载更新，但提示用户批准并在安装前
            NOTIFY_TYPE_SCHEDULED_INSTALL,      //自动下载并安装更新
        }; 

        //当 NOTIFY_TYPE 为 NOTIFY_TYPE_SCHEDULED_INSTALL 时该有效
        enum AUTO_UPDATE_DAY
        {
            AUTO_UPDATE_NONE = 0,       //无更新日期
            AUTO_UPDATE_MONDAY = 1,     //星期一
            AUTO_UPDATE_TUESDAY = 2,    //星期二
            AUTO_UPDATE_WEDNESDAY = 3,  //星期三
            AUTO_UPDATE_THURSDAY = 4,   //星期四
            AUTO_UPDATE_FRIDAY = 5,     //星期五
            AUTO_UPDATE_SATURDAY = 6,   //星期六
            AUTO_UPDATE_SUNDAY = 7,     //星期天
            AUTO_UPDATE_EVERY = 8,      //每天
        };

        //////////////////////////////////////////////////////////////////////////

        CWUpdateAgent();

        virtual ~CWUpdateAgent();

        ERROR_CODE GetServiceStatus(SERVICE_STATUS& serviceStatus);

        //功能：启动WINDOWS UPDATE 自动更新（包括运行服务）[在任何情况下都尝试启动，如果已经启动，也返回成功]
        ERROR_CODE EnableService();

        ERROR_CODE DiableService();

        //功能：获取WINODWS UPDATE 类型
        //参数：
        //      notifyType OUT 更新类型
        //      pAutoUpdateDay OUT 自动更新日期（一周中的第几天）
        //          [notifyType == NOTIFY_TYPE_SCHEDULED_INSTALL 时有效, 如果 notifyType 不满足该值，则返回 AUTO_UPDATE_NONE；参数为NULL则不返回任何内容]
        //      pnAutoUpdateTime OUT 自动更新时间（一天内的第几点，范围 0 ~ 23，对应凌晨0点到晚上23点）
        //          [notifyType == NOTIFY_TYPE_SCHEDULED_INSTALL 时有效, 如果 notifyType 不满足该值，则返回 -1；参数为NULL则不返回任何内容]
        ERROR_CODE GetNotiyType(
            NOTIFY_TYPE& notifyType, 
            AUTO_UPDATE_DAY* pAutoUpdateDay, 
            INT16* pnAutoUpdateTime);

        //功能：设置 WINODWS UPDATE 类型
        //参数：
        //      notifyType OUT 更新类型
        //      pAutoUpdateDay OUT 自动更新日期（一周中的第几天）
        //          [notifyType == NOTIFY_TYPE_SCHEDULED_INSTALL 时有效, 如果 notifyType 不满足该值或参数为NULL则不进行设置]
        //      pnAutoUpdateTime OUT 自动更新时间（一天内的第几点，范围 0 ~ 23，对应凌晨0点到晚上23点）
        //          [notifyType == NOTIFY_TYPE_SCHEDULED_INSTALL 时有效, 如果 notifyType 不满足该值或参数为NULL则不进行设置]
        ERROR_CODE SetNotiyType(
            NOTIFY_TYPE notifyType, 
            AUTO_UPDATE_DAY* pAutoUpdateDay, 
            INT16* pnAutoUpdateTime);

        ERROR_CODE GetLastUpdateTime(bool bHasUpdate, UINT64& nTime);

        //////////////////////////////////////////////////////////////////////////

        static bool GetWSUSUrl(string& strUrl);

    private:

        IAutomaticUpdates* m_pIAutomaticUpdates;
    };
}
