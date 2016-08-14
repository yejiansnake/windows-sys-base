#include "../SysBase_Interface.h"

#pragma comment(lib, "Advapi32.lib")

namespace SysBase
{
    CSqlServerHelper::CSqlServerHelper()
    {

    }

    CSqlServerHelper::~CSqlServerHelper()
    {

    }

    bool CSqlServerHelper::GetDBInstanceInfoList(list<TSqlServerInstanceInfo>& aSqlServerInstanceInfoList)
    {
        aSqlServerInstanceInfoList.clear();

        char szRegPath[1024] = {0};
        char szBuffer[1024] = {0};
        DWORD dwBufferSize = 1024;

        HKEY RegKey = NULL;

        //////////////////////////////////////////////////////////////////////////
        //获取数据库实例名称

        sprintf_s(szRegPath, 1023, "SOFTWARE\\Microsoft\\Microsoft SQL Server");

        if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_QUERY_VALUE, &RegKey))
        {
            return false;
        }

        ZeroMemory(szBuffer, 1024);
        dwBufferSize = 1024;
        if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "InstalledInstances", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
        {
            //获取的实例名称为一个多字符串类型的字符串信息，其中每个字符串用 \0 分割

            char* pSource = szBuffer;
            int nStrLen = (int)strlen(pSource);

            while (nStrLen > 0)
            {
                char szTemp[1024] = {0};

                strncpy_s(szTemp, pSource, nStrLen);

                if (NULL != szTemp[0])
                {
                    TSqlServerInstanceInfo aTSqlServerInstanceInfo;
                    aTSqlServerInstanceInfo.strName = szTemp;
                    aSqlServerInstanceInfoList.push_back(aTSqlServerInstanceInfo);
                }

                pSource = pSource + nStrLen + 1;

                nStrLen = (int)strlen(pSource);
            }
        }

        RegCloseKey(RegKey);
        RegKey = NULL;

        //////////////////////////////////////////////////////////////////////////
        //获取实例的相关信息

        list<TSqlServerInstanceInfo>::iterator entity = aSqlServerInstanceInfoList.begin();
        list<TSqlServerInstanceInfo>::iterator endEntity = aSqlServerInstanceInfoList.end();

        for (; entity != endEntity; ++entity)
        {
            //////////////////////////////////////////////////////////////////////////
            //取实例对应的注册表路径名称

            sprintf_s(szRegPath, 1023, "SOFTWARE\\Microsoft\\Microsoft SQL Server\\Instance Names\\SQL");

            if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_QUERY_VALUE, &RegKey))
            {
                return false;
            }

            char szInstancePathName[1024] = {0};
            dwBufferSize = 1024;
            if (ERROR_SUCCESS != RegQueryValueExA(RegKey, entity->strName.c_str(), NULL, NULL, (LPBYTE)szInstancePathName, &dwBufferSize))
            {
                RegCloseKey(RegKey);
                RegKey = NULL;

                return false;
            }

            RegCloseKey(RegKey);
            RegKey = NULL;

            //////////////////////////////////////////////////////////////////////////
            //取实例基础信息

            sprintf_s(szRegPath, 1023, "SOFTWARE\\Microsoft\\Microsoft SQL Server\\%s\\Setup", szInstancePathName);

            if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_QUERY_VALUE, &RegKey))
            {
                return false;
            }

            ZeroMemory(szBuffer, 1024);
            dwBufferSize = 1024;
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "Version", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
            {
                if (NULL != szBuffer[0])
                {
                    entity->strVersion = szBuffer;
                }
            }

            ZeroMemory(szBuffer, 1024);
            dwBufferSize = 1024;
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "Edition", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
            {
                if (NULL != szBuffer[0])
                {
                    entity->strEdition = szBuffer;
                }
            }

            ZeroMemory(szBuffer, 1024);
            dwBufferSize = 1024;
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "Collation", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
            {
                if (NULL != szBuffer[0])
                {
                    entity->strCollation = szBuffer;
                }
            }

            ZeroMemory(szBuffer, 1024);
            dwBufferSize = 1024;
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "SQLBinRoot", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
            {
                if (NULL != szBuffer[0])
                {
                    entity->strSQLBinRoot = szBuffer;
                }
            }

            ZeroMemory(szBuffer, 1024);
            dwBufferSize = 1024;
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "SQLDataRoot", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
            {
                if (NULL != szBuffer[0])
                {
                    entity->strSQLDataRoot = szBuffer;
                }
            }

            ZeroMemory(szBuffer, 1024);
            dwBufferSize = 1024;
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "SQLPath", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
            {
                if (NULL != szBuffer[0])
                {
                    entity->strSQLPath = szBuffer;
                }
            }

            ZeroMemory(szBuffer, 1024);
            dwBufferSize = 1024;
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "SqlProgramDir", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
            {
                if (NULL != szBuffer[0])
                {
                    entity->strSqlProgramDir = szBuffer;
                }
            }

            dwBufferSize = sizeof(UINT32);
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "Language", NULL, NULL, (LPBYTE)&entity->nLanguageCode, &dwBufferSize))
            {
            }

            RegCloseKey(RegKey);
            RegKey = NULL;

            //////////////////////////////////////////////////////////////////////////
            //取端口号

            sprintf_s(szRegPath, 1023, "SOFTWARE\\Microsoft\\Microsoft SQL Server\\%s\\MSSQLServer\\SuperSocketNetLib\\Tcp\\IPAll", szInstancePathName);

            if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_QUERY_VALUE, &RegKey))
            {
                return false;
            }

            ZeroMemory(szBuffer, 1024);
            dwBufferSize = 1024;
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "TcpPort", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
            {
                entity->nTcpPort = (UINT16)atol(szBuffer);
            }

            ZeroMemory(szBuffer, 1024);
            dwBufferSize = 1024;
            if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "TcpDynamicPorts", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
            {
                entity->nTcpDynamicPort = (UINT16)atol(szBuffer);
            }

            RegCloseKey(RegKey);
            RegKey = NULL;
        }

        return true;
    }
}