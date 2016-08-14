#include "../SysBase_Interface.h"

namespace SysBase
{
    bool CSysUser::AddUser(TSysUserParam& aParam)
    {
        if (aParam.strUserName.length() <= 0 || aParam.strPwd.length() <= 0)
        {
            return false;
        }

        USER_INFO_2 userInfo = {0};
        userInfo.usri2_name = (LPWSTR)aParam.strUserName.c_str();
        userInfo.usri2_password = (LPWSTR)aParam.strPwd.c_str();
        userInfo.usri2_full_name = (LPWSTR)aParam.strFullName.c_str();
        userInfo.usri2_comment = (LPWSTR)aParam.strDesc.c_str();
        userInfo.usri2_priv = USER_PRIV_USER;

        if (0 == aParam.nExpiresTime)
        {
            userInfo.usri2_acct_expires = TIMEQ_FOREVER;
        }
        else
        {
            userInfo.usri2_acct_expires = (UINT32)aParam.nExpiresTime;
        }

        if (aParam.nUserFlags & AccountDisable)
        {
            userInfo.usri2_flags |= UF_ACCOUNTDISABLE;
        }

        if (aParam.nUserFlags & PwdCantChange)
        {
            userInfo.usri2_flags |= UF_PASSWD_CANT_CHANGE;
        }

        if (aParam.nUserFlags & DontExpirePwd)
        {
            userInfo.usri2_flags |= UF_DONT_EXPIRE_PASSWD;
        }

        if (aParam.nUserFlags & PwdExpired)
        {
            userInfo.usri2_flags |= UF_PASSWORD_EXPIRED;
        }
        
        DWORD dwError = 0;
        DWORD dwRes = NetUserAdd(NULL, 2, (LPBYTE)&userInfo, &dwError);

        if (NERR_Success != dwRes)
        {
            return false;
        }

        return true;
    }

    bool CSysUser::DeleteUser(PCWCHAR szUserName)
    {
        DWORD dwRes = NetUserDel(NULL, szUserName);

        if (NERR_Success == dwRes)
        {
            return true;
        }

        return false;
    }

    bool CSysUser::ChangeUserPwd(PCWCHAR szUserName, PCWCHAR szOldPwd, PCWCHAR szNewPwd, UINT32* pnErrorCode)
    {
        DWORD dwRes = NetUserChangePassword(NULL, szUserName, szOldPwd, szNewPwd);

        if (pnErrorCode)
        {
            *pnErrorCode = dwRes;
        }

        if (NERR_Success == dwRes)
        {
            return true;
        }

        return false;
    }

    bool CSysUser::GetUserInfo(PCWCHAR szUserName, TSysUserInfo& infoOut)
    {
        LPBYTE pBuffer = NULL;

        DWORD dwRes = NetUserGetInfo(NULL, szUserName, 2, &pBuffer);

        if (NERR_Success != dwRes)
        {
            return false;
        }

        USER_INFO_2* pUserInfo = (USER_INFO_2*)pBuffer;

        infoOut.strUserName = szUserName;

        if (pUserInfo->usri2_full_name)
        {
            infoOut.strFullName = pUserInfo->usri2_full_name;
        }
        
        if (pUserInfo->usri2_comment)
        {
            infoOut.strDesc = pUserInfo->usri2_comment;
        }
        
        infoOut.nPwdAge = pUserInfo->usri2_password_age;
        infoOut.nPrivilege = pUserInfo->usri2_priv;
        infoOut.nUserFlags = pUserInfo->usri2_flags;
        infoOut.nLastLogonTime = pUserInfo->usri2_last_logon;
        infoOut.nExpiresTime = pUserInfo->usri2_acct_expires;
        infoOut.nBadPwdCount = pUserInfo->usri2_bad_pw_count;
        infoOut.nLogonCount = pUserInfo->usri2_num_logons;

        if (pBuffer)
        {
            NetApiBufferFree(pBuffer);
            pBuffer = NULL;
        }

        return true;
    }

    bool CSysUser::SetUserInfo(TSysUserInfo& aParam)
    {
        return CSysUser::SetUserInfo(aParam.strUserName.c_str(), aParam);
    }

    bool CSysUser::SetUserInfo(PCWCHAR szUserName, TSysUserInfo& aParam)
    {
        if (aParam.strUserName.length() <= 0)
        {
            return false;
        }

        LPBYTE pBuffer = NULL;

        DWORD dwRes = NetUserGetInfo(NULL, szUserName, 2, &pBuffer);

        USER_INFO_2* pUserInfo = (USER_INFO_2*)pBuffer;
        pUserInfo->usri2_name = (LPWSTR)aParam.strUserName.c_str();
        pUserInfo->usri2_full_name = (LPWSTR)aParam.strFullName.c_str();
        pUserInfo->usri2_comment = (LPWSTR)aParam.strDesc.c_str();

        if (0 == aParam.nExpiresTime)
        {
            pUserInfo->usri2_acct_expires = TIMEQ_FOREVER;
        }
        else
        {
            pUserInfo->usri2_acct_expires = (UINT32)aParam.nExpiresTime;
        }

        DWORD dwNewFlag = 0;

        if (aParam.nUserFlags & AccountDisable)
        {
            dwNewFlag |= UF_ACCOUNTDISABLE;
        }

        if (aParam.nUserFlags & PwdCantChange)
        {
            dwNewFlag |= UF_PASSWD_CANT_CHANGE;
        }

        if (aParam.nUserFlags & DontExpirePwd)
        {
            dwNewFlag |= UF_DONT_EXPIRE_PASSWD;
        }

        if (aParam.nUserFlags & PwdExpired)
        {
            dwNewFlag |= UF_PASSWORD_EXPIRED;
        }

        pUserInfo->usri2_flags = dwNewFlag;

        dwRes = NetUserSetInfo(NULL, szUserName, 2, (LPBYTE)pUserInfo, NULL);

        if (pBuffer)
        {
            NetApiBufferFree(pBuffer);
            pBuffer = NULL;
        }

        if (NERR_Success == dwRes)
        {
            return true;
        }

        return false;
    }

    bool CSysUser::GetUserAll(list<TSysUserInfo>& infoList)
    {
        LPBYTE pBuf = NULL;
        LPUSER_INFO_2 pUserInfo = NULL;
        DWORD dwLevel = 2;
        DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
        DWORD dwEntriesRead = 0;
        DWORD dwTotalEntries = 0;
        DWORD dwResumeHandle = 0;
        DWORD dwIndex;

        bool bSuccess = true;
        NET_API_STATUS nStatus = NERR_Success;

        do
        {
            nStatus = NetUserEnum(
                NULL,
                dwLevel,
                FILTER_NORMAL_ACCOUNT,
                &pBuf,
                dwPrefMaxLen,
                &dwEntriesRead,
                &dwTotalEntries,
                &dwResumeHandle);

            if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
            {
                if ((pUserInfo = (LPUSER_INFO_2)pBuf) != NULL)
                {
                    for (dwIndex = 0; (dwIndex < dwEntriesRead); dwIndex++)
                    {
                        TSysUserInfo userInfo;
                        userInfo.strUserName = pUserInfo->usri2_name;

                        if (pUserInfo->usri2_full_name)
                        {
                            userInfo.strFullName = pUserInfo->usri2_full_name;
                        }

                        if (pUserInfo->usri2_comment)
                        {
                            userInfo.strDesc = pUserInfo->usri2_comment;
                        }

                        userInfo.nPwdAge = pUserInfo->usri2_password_age;
                        userInfo.nPrivilege = pUserInfo->usri2_priv;
                        userInfo.nUserFlags = pUserInfo->usri2_flags;
                        userInfo.nLastLogonTime = pUserInfo->usri2_last_logon;
                        userInfo.nExpiresTime = pUserInfo->usri2_acct_expires;
                        userInfo.nBadPwdCount = pUserInfo->usri2_bad_pw_count;
                        userInfo.nLogonCount = pUserInfo->usri2_num_logons;

                        infoList.push_back(userInfo);

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

    bool CSysUser::GetGroupAllByUser(PCWCHAR szUserName, list<wstring>& groupList)
    {
        LPLOCALGROUP_USERS_INFO_0 pGroupInfo = NULL;
        DWORD dwEntriesRead = 0;
        DWORD dwTotalEntries = 0;

        DWORD dwRes = NetUserGetLocalGroups(NULL, szUserName, 0, LG_INCLUDE_INDIRECT, (LPBYTE*)&pGroupInfo, 
            MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries);

        if (NERR_Success != dwRes)
        {
            if (pGroupInfo)
            {
                NetApiBufferFree(pGroupInfo);
                pGroupInfo = NULL;
            }

            return false;
        }

        LPLOCALGROUP_USERS_INFO_0 pGroupInfoTemp = pGroupInfo;

        DWORD dwIndex = 0;
        for (dwIndex = 0; dwIndex < dwEntriesRead; ++dwIndex)
        {
            groupList.push_back(pGroupInfoTemp->lgrui0_name);

            pGroupInfoTemp++;
        }

        if (pGroupInfo)
        {
            NetApiBufferFree(pGroupInfo);
            pGroupInfo = NULL;
        }

        return true;
    }
}