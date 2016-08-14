#pragma once

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //备注:
    //AddGroupMembers,DeleteGroupMembers,SetGroupMembers 的参数 domainAndNameList 中的每一个项，都是 "域名\用户" 这种格式
    //GetGroupMembers 返回的 domainAndNameListOut 也是一样

    //////////////////////////////////////////////////////////////////////////

    struct TSysUserGroupInfo
    {
        wstring strName;
        wstring strDesc;
    };

    class CSysUserGroup
    {
    public:

        static bool AddGroup(TSysUserGroupInfo& aInfo);

        static bool DeleteGroup(PCWCHAR szGroupName);

        static bool GetGroupInfo(PCWCHAR szGroupName, TSysUserGroupInfo& infoOut);

        static bool SetGroupInfo(TSysUserGroupInfo& aInfo);

        static bool SetGroupInfo(PCWCHAR szGroupName, TSysUserGroupInfo& aInfo);

        static bool GetGroupAll(list<TSysUserGroupInfo>& infoList);

        //domainAndNameList 的每个元素的格式: <DomainName>\<AccountName>     
        static bool AddGroupMembers(PCWCHAR szGroupName, list<wstring>& domainAndNameList);

        static bool DeleteGroupMembers(PCWCHAR szGroupName, list<wstring>& domainAndNameList);

        static bool GetGroupMembers(PCWCHAR szGroupName, list<wstring>& domainAndNameListOut);

        static bool SetGroupMembers(PCWCHAR szGroupName, list<wstring>& domainAndNameList);

        //////////////////////////////////////////////////////////////////////////

        static bool AddLocalGroupMembers(PCWCHAR szGroupName, list<wstring>& userNameList);

        static bool DeleteLocalGroupMembers(PCWCHAR szGroupName, list<wstring>& userNameList);

        static bool SetLocalGroupMembers(PCWCHAR szGroupName, list<wstring>& userNameList);

        //////////////////////////////////////////////////////////////////////////

        static bool AddLocalGroupMember(PCWCHAR szGroupName, PCWCHAR szUserName);

        static bool DeleteLocalGroupMember(PCWCHAR szGroupName, PCWCHAR szUserName);
    };
}