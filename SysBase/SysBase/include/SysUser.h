#pragma once

#include "SysBase.h"

namespace SysBase
{
    struct TSysUserParam
    {
        TSysUserParam()
        {
            nUserFlags = 0;
            nExpiresTime = INFINITE;
        }

        wstring strUserName;        //用户名
        wstring strPwd;             //密码
        wstring strFullName;        //全称
        wstring strDesc;            //描述
        UINT64  nExpiresTime;       //账户的过期时间(CSysUser::ESysUserFlag)
        UINT32  nUserFlags;         //用户账户标志
    };
    
    struct TSysUserInfo 
    {
        TSysUserInfo()
        {
            nPwdAge = 0;
            nPrivilege = 0;
            nUserFlags = 0;
            nExpiresTime = INFINITE;
            nLastLogonTime = 0;
            nBadPwdCount = 0;
            nLogonCount = 0;
        }

        wstring strUserName;
        wstring strFullName;        //全称
        wstring strDesc;            //描述
        UINT32  nUserFlags;         //用户账户标志(CSysUser::ESysUserFlag)
        UINT64  nExpiresTime;       //账户的过期时间

        //////////////////////////////////////////////////////////////////////////
        //以下的成员数据只能获取，无法 Set

        UINT32  nPrivilege;         //特权（CSysUser::ESysUserPrivilege）
        UINT32  nPwdAge;            //距离上一次修改密码后到现在的时间
        UINT64  nLastLogonTime;     //最后一次成功登录的时间
        UINT32  nBadPwdCount;       //使用该用户名输入错误的密码次数
        UINT32  nLogonCount;        //使用该用户名成功登录系统的次数
    };

    class CSysUser
    {
    public:

        //用户权限类型
        enum ESysUserPrivilege
        {
            Guest = 0,     //访客
            User = 1,      //用户
            Admin = 2,     //系统管理员
        };

        //用户账户标志
        enum ESysUserFlag
        {
            None = 0,                   //无
            AccountDisable = 1,         //禁用账户
            PwdCantChange = 2,          //密码不能被修改
            DontExpirePwd = 4,          //密码永不过期
            PwdExpired = 8,             //密码已经过期（必须重新修改密码）
        };

        static bool AddUser(TSysUserParam& aParam);

        static bool DeleteUser(PCWCHAR szUserName);
        
        //参数:
        //  pnErrorCode:    系统错误码
        //      ERROR_ACCESS_DENIED 5         拒绝访问
        //      ERROR_INVALID_PASSWORD 86L    无效的密码
        //      NERR_InvalidComputer 2351     无效计算机
        //      NERR_NotPrimary 2226          该操作只允许在域控制器的域
        //      NERR_UserNotFound 2221        无该用户
        //      NERR_PasswordTooShort 2245    密码短于所需。（密码也可能太长，在它的更改历史上太近，没有足够的特殊字符，或不符合另一个密码策略要求。）
        static bool ChangeUserPwd(PCWCHAR szUserName, PCWCHAR szOldPwd, PCWCHAR szNewPwd, UINT32* pnErrorCode = NULL);

        static bool GetUserInfo(PCWCHAR szUserName, TSysUserInfo& infoOut);

        static bool SetUserInfo(TSysUserInfo& aParam);

        static bool SetUserInfo(PCWCHAR szUserName, TSysUserInfo& aParam);

        static bool GetUserAll(list<TSysUserInfo>& infoList);
        
        static bool GetGroupAllByUser(PCWCHAR szName, list<wstring>& groupList);

    };
}