#include "../SysBase_Interface.h"

namespace SysBase
{
    bool CSysUserGroup::AddGroup(TSysUserGroupInfo& aInfo)
    {
        LOCALGROUP_INFO_1 groupInfo = {0};
        groupInfo.lgrpi1_name = (LPWSTR)aInfo.strName.c_str();
        groupInfo.lgrpi1_comment = (LPWSTR)aInfo.strDesc.c_str();

        DWORD dwRes = NetLocalGroupAdd(NULL, 1, (LPBYTE)&groupInfo, NULL);
        
        if (NERR_Success == dwRes)
        {
            return true;
        }

        return false;
    }

    bool CSysUserGroup::DeleteGroup(PCWCHAR szGroupName)
    {
        DWORD dwRes = NetLocalGroupDel(NULL, szGroupName);

        if (NERR_Success == dwRes)
        {
            return true;
        }

        return false;
    }

    bool CSysUserGroup::GetGroupInfo(PCWCHAR szGroupName, TSysUserGroupInfo& infoOut)
    {
        LPLOCALGROUP_INFO_1 pGroupInfo = NULL;

        DWORD dwRes = NetLocalGroupGetInfo(NULL, szGroupName, 1, (LPBYTE*)&pGroupInfo);

        if (NERR_Success != dwRes)
        {
            return false;
        }

        infoOut.strName = pGroupInfo->lgrpi1_name;

        if (pGroupInfo->lgrpi1_comment)
        {
            infoOut.strDesc = pGroupInfo->lgrpi1_comment;
        }

        if (pGroupInfo)
        {
            NetApiBufferFree(pGroupInfo);
            pGroupInfo = NULL;
        }

        return true;
    }

    bool CSysUserGroup::SetGroupInfo(TSysUserGroupInfo& aInfo)
    {
        return CSysUserGroup::SetGroupInfo(aInfo.strName.c_str(), aInfo);
    }

    bool CSysUserGroup::SetGroupInfo(PCWCHAR szGroupName, TSysUserGroupInfo& aInfo)
    {
        LOCALGROUP_INFO_1 groupInfo = {0};
        groupInfo.lgrpi1_name = (LPWSTR)aInfo.strName.c_str();
        groupInfo.lgrpi1_comment = (LPWSTR)aInfo.strDesc.c_str();

        DWORD dwRes = NetLocalGroupSetInfo(NULL, szGroupName, 1, (LPBYTE)&groupInfo, NULL);

        if (NERR_Success == dwRes)
        {
            return true;
        }

        return false;
    }

    bool CSysUserGroup::GetGroupAll(list<TSysUserGroupInfo>& infoList)
    {
        LPBYTE pBuf = NULL;
        LPLOCALGROUP_INFO_1 pUserInfo = NULL;
        DWORD dwLevel = 1;
        DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
        DWORD dwEntriesRead = 0;
        DWORD dwTotalEntries = 0;
        DWORD_PTR dwResumeHandle = 0;
        DWORD dwIndex;

        bool bSuccess = true;
        NET_API_STATUS nStatus = NERR_Success;

        do
        {
            nStatus = NetLocalGroupEnum(
                NULL,
                dwLevel,
                &pBuf,
                dwPrefMaxLen,
                &dwEntriesRead,
                &dwTotalEntries,
                &dwResumeHandle);

            if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
            {
                if ((pUserInfo = (LPLOCALGROUP_INFO_1)pBuf) != NULL)
                {
                    for (dwIndex = 0; (dwIndex < dwEntriesRead); dwIndex++)
                    {
                        TSysUserGroupInfo groupInfo;
                        groupInfo.strName = pUserInfo->lgrpi1_name;

                        if (pUserInfo->lgrpi1_comment)
                        {
                            groupInfo.strDesc = pUserInfo->lgrpi1_comment;
                        }

                        infoList.push_back(groupInfo);

                        pUserInfo++;
                    }
                }
                else
                {
                    bSuccess = false;
                    break;
                }
            }
            else
            {
                bSuccess = false;
                break;
            }

            if (pBuf)
            {
                NetApiBufferFree(pBuf);
                pBuf = NULL;
            }
        }
        while (nStatus == ERROR_MORE_DATA);

        if (pBuf)
        {
            NetApiBufferFree(pBuf);
        }

        return bSuccess;
    }

    bool CSysUserGroup::AddGroupMembers(PCWCHAR szGroupName, list<wstring>& domainAndNameList)
    {
        if (!szGroupName || 0 == domainAndNameList.size())
        {
            return false;
        }

        if (NULL == szGroupName[0])
        {
            return false;
        }

        vector<LOCALGROUP_MEMBERS_INFO_3> memberArray;
        memberArray.resize(domainAndNameList.size());

        list<wstring>::iterator entity = domainAndNameList.begin();
        list<wstring>::iterator endEntity = domainAndNameList.end();

        int nIndex = 0;
        for (; entity != endEntity; ++entity)
        {
            memberArray[nIndex].lgrmi3_domainandname = (LPWSTR)entity->c_str();
            nIndex++;
        }

        DWORD dwRes = NetLocalGroupAddMembers(NULL, szGroupName, 3, (LPBYTE)memberArray.data(), (DWORD)memberArray.size());

        if (NERR_Success == dwRes)
        {
            return true;
        }

        return false;
    }

    bool CSysUserGroup::DeleteGroupMembers(PCWCHAR szGroupName, list<wstring>& domainAndNameList)
    {
        if (!szGroupName || 0 == domainAndNameList.size())
        {
            return false;
        }

        if (NULL == szGroupName[0])
        {
            return false;
        }

        vector<LOCALGROUP_MEMBERS_INFO_3> memberArray;
        memberArray.resize(domainAndNameList.size());

        list<wstring>::iterator entity = domainAndNameList.begin();
        list<wstring>::iterator endEntity = domainAndNameList.end();

        int nIndex = 0;
        for (; entity != endEntity; ++entity)
        {
            memberArray[nIndex].lgrmi3_domainandname = (LPWSTR)entity->c_str();
            nIndex++;
        }

        DWORD dwRes = NetLocalGroupDelMembers(NULL, szGroupName, 3, (LPBYTE)memberArray.data(), (DWORD)memberArray.size());

        if (NERR_Success == dwRes)
        {
            return true;
        }

        return false;
    }

    bool CSysUserGroup::GetGroupMembers(PCWCHAR szGroupName, list<wstring>& domainAndNameListOut)
    {
        LPBYTE pBuf = NULL;
        LPLOCALGROUP_MEMBERS_INFO_3 pMemberInfo = NULL;
        DWORD dwLevel = 3;
        DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
        DWORD dwEntriesRead = 0;
        DWORD dwTotalEntries = 0;
        DWORD_PTR dwResumeHandle = 0;
        DWORD dwIndex;

        bool bSuccess = true;
        NET_API_STATUS nStatus = NERR_Success;

        do
        {
            nStatus = NetLocalGroupGetMembers(
                NULL,
                szGroupName,
                dwLevel,
                &pBuf,
                dwPrefMaxLen,
                &dwEntriesRead,
                &dwTotalEntries,
                &dwResumeHandle);

            if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
            {
                if ((pMemberInfo = (LPLOCALGROUP_MEMBERS_INFO_3)pBuf) != NULL)
                {
                    for (dwIndex = 0; (dwIndex < dwEntriesRead); dwIndex++)
                    {
                        domainAndNameListOut.push_back(pMemberInfo->lgrmi3_domainandname);

                        pMemberInfo++;
                    }
                }
                else
                {
                    bSuccess = false;
                    break;
                }
            }
            else
            {
                bSuccess = false;
                break;
            }

            if (pBuf)
            {
                NetApiBufferFree(pBuf);
                pBuf = NULL;
            }
        }
        while (nStatus == ERROR_MORE_DATA);

        if (pBuf)
        {
            NetApiBufferFree(pBuf);
        }

        return bSuccess;
    }

    bool CSysUserGroup::SetGroupMembers(PCWCHAR szGroupName, list<wstring>& domainAndNameList)
    {
        if (!szGroupName || 0 == domainAndNameList.size())
        {
            return false;
        }

        if (NULL == szGroupName[0])
        {
            return false;
        }

        vector<LOCALGROUP_MEMBERS_INFO_3> memberArray;
        memberArray.resize(domainAndNameList.size());

        list<wstring>::iterator entity = domainAndNameList.begin();
        list<wstring>::iterator endEntity = domainAndNameList.end();

        int nIndex = 0;
        for (; entity != endEntity; ++entity)
        {
            memberArray[nIndex].lgrmi3_domainandname = (LPWSTR)entity->c_str();
            nIndex++;
        }

        DWORD dwRes = NetLocalGroupSetMembers(NULL, szGroupName, 3, (LPBYTE)memberArray.data(), (DWORD)memberArray.size());

        if (NERR_Success == dwRes)
        {
            return true;
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////////

    bool CSysUserGroup::AddLocalGroupMembers(PCWCHAR szGroupName, list<wstring>& userNameList)
    {
        if (!szGroupName || 0 == userNameList.size())
        {
            return false;
        }

        if (NULL == szGroupName[0])
        {
            return false;
        }

        list<wstring> domainAndNameList;

        wstring strPCName;
        CSystemInfo::GetCurComputerName(strPCName);

        list<wstring>::iterator entity = userNameList.begin();
        list<wstring>::iterator endEntity = userNameList.end();

        for (; entity != endEntity; ++entity)
        {
            WCHAR szTemp[1024] = {0};
            swprintf_s(szTemp, L"%s\\%s", strPCName.c_str(), entity->c_str());

            domainAndNameList.push_back(szTemp);
        }

        return CSysUserGroup::AddGroupMembers(szGroupName, domainAndNameList);
    }

    bool CSysUserGroup::DeleteLocalGroupMembers(PCWCHAR szGroupName, list<wstring>& userNameList)
    {
        if (!szGroupName || 0 == userNameList.size())
        {
            return false;
        }

        if (NULL == szGroupName[0])
        {
            return false;
        }

        list<wstring> domainAndNameList;

        wstring strPCName;
        CSystemInfo::GetCurComputerName(strPCName);

        list<wstring>::iterator entity = userNameList.begin();
        list<wstring>::iterator endEntity = userNameList.end();

        for (; entity != endEntity; ++entity)
        {
            WCHAR szTemp[1024] = {0};
            swprintf_s(szTemp, L"%s\\%s", strPCName.c_str(), entity->c_str());

            domainAndNameList.push_back(szTemp);
        }

        return CSysUserGroup::DeleteGroupMembers(szGroupName, domainAndNameList);
    }

    bool CSysUserGroup::SetLocalGroupMembers(PCWCHAR szGroupName, list<wstring>& userNameList)
    {
        if (!szGroupName || 0 == userNameList.size())
        {
            return false;
        }

        if (NULL == szGroupName[0])
        {
            return false;
        }

        list<wstring> domainAndNameList;

        wstring strPCName;
        CSystemInfo::GetCurComputerName(strPCName);

        list<wstring>::iterator entity = userNameList.begin();
        list<wstring>::iterator endEntity = userNameList.end();

        for (; entity != endEntity; ++entity)
        {
            WCHAR szTemp[1024] = {0};
            swprintf_s(szTemp, L"%s\\%s", strPCName.c_str(), entity->c_str());

            domainAndNameList.push_back(szTemp);
        }

        return CSysUserGroup::SetGroupMembers(szGroupName, domainAndNameList);
    }

    //////////////////////////////////////////////////////////////////////////

    bool CSysUserGroup::AddLocalGroupMember(PCWCHAR szGroupName, PCWCHAR szUserName)
    {
        if (!szGroupName || !szUserName)
        {
            return false;
        }

        if (NULL == szGroupName[0] || NULL == szUserName[0])
        {
            return false;
        }

        list<wstring> domainAndNameList;
        wstring strPCName;
        CSystemInfo::GetCurComputerName(strPCName);
        WCHAR szTemp[1024] = {0};
        swprintf_s(szTemp, L"%s\\%s", strPCName.c_str(), szUserName);
        domainAndNameList.push_back(szTemp);

        return CSysUserGroup::AddGroupMembers(szGroupName, domainAndNameList);
    }

    bool CSysUserGroup::DeleteLocalGroupMember(PCWCHAR szGroupName, PCWCHAR szUserName)
    {
        if (!szGroupName || !szUserName)
        {
            return false;
        }

        if (NULL == szGroupName[0] || NULL == szUserName[0])
        {
            return false;
        }

        list<wstring> domainAndNameList;
        wstring strPCName;
        CSystemInfo::GetCurComputerName(strPCName);
        WCHAR szTemp[1024] = {0};
        swprintf_s(szTemp, L"%s\\%s", strPCName.c_str(), szUserName);
        domainAndNameList.push_back(szTemp);

        return CSysUserGroup::DeleteGroupMembers(szGroupName, domainAndNameList);
    }
}