#include "../SysBase_Interface.h"

#include <psapi.h>
#include <Tlhelp32.h>

#pragma comment( lib, "PSAPI.LIB")

namespace SysBase
{
    CProcessHelper CProcessHelper::s_CProcessHelper;

    //////////////////////////////////////////////////////////////////////////
    //CProcessHelper

    CProcessHelper::CProcessHelper()
    {
    }

    CProcessHelper::~CProcessHelper()
    {
    }

    bool CProcessHelper::GetAllProcessInfo(list<TProcessInfo>& processInfoList, UINT32 nProcessInfoTypes)
    {
        processInfoList.clear();

        HANDLE hProcessSnap;
        PROCESSENTRY32W processEntry = {0};
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (hProcessSnap == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        if (!Process32FirstW(hProcessSnap, &processEntry))
        {
            CloseHandle(hProcessSnap);

            return false;
        }

        do
        {
            if (0 == processEntry.th32ProcessID)
            {
                continue;
            }

            TProcessInfo aTProcessInfo = {0};

            //一般信息
            {
                aTProcessInfo.aNormal.nProcessID = processEntry.th32ProcessID;
                aTProcessInfo.aNormal.nParentProcessID = processEntry.th32ParentProcessID;
                aTProcessInfo.aNormal.nThreadCount = processEntry.cntThreads;
                aTProcessInfo.aNormal.nPriClassBase = processEntry.pcPriClassBase;
                memcpy(aTProcessInfo.aNormal.szExeFile, processEntry.szExeFile, MAX_PATH);
            }

            if (nProcessInfoTypes > 0)
            {
                CProcess aCProcess;

                if (CProcess::ERROR_CODE_SUCCESS != aCProcess.Open(processEntry.th32ProcessID,
                    CProcess::OPEN_RIGHT_TYPE_QUERY_ALL))
                {
                    //如果无法打开进程，则继续运行下一个
                    continue;
                }

                //内存
                if (nProcessInfoTypes & CProcessHelper::PROCESS_INFO_TYPE_MEMORY)
                {
                    aCProcess.GetProcessMemory(aTProcessInfo.aMemory);
                }

                //IO计数
                if (nProcessInfoTypes & CProcessHelper::PROCESS_INFO_TYPE_IO)
                {
                    aCProcess.GetProcessIOCounter(aTProcessInfo.aIOCounter);
                }

                //资源数
                if (nProcessInfoTypes & CProcessHelper::PROCESS_INFO_TYPE_RESOURCE)
                {
                    aCProcess.GetProcessResource(aTProcessInfo.aResource);
                }

                //高级信息
                if (nProcessInfoTypes & CProcessHelper::PROCESS_INFO_TYPE_ADVANCED)
                {
                    aCProcess.GetProcessAdvanced(aTProcessInfo.aAdvanced);
                }

                //时间
                if (nProcessInfoTypes & CProcessHelper::PROCESS_INFO_TYPE_TIME)
                {
                    aCProcess.GetProcessTime(aTProcessInfo.aTime);
                }

                processInfoList.push_back(aTProcessInfo);
            }
        }
        while(Process32NextW(hProcessSnap, &processEntry));

        CloseHandle(hProcessSnap);

        return true;
    }

    bool CProcessHelper::GetProcessInfo(UINT32 nProcessID, TProcessInfo& processInfo, UINT32 nProcessInfoTypes)
    {
        CProcess aCProcess;

		memset(&processInfo,0,sizeof(TProcessInfo));
        if (CProcess::ERROR_CODE_SUCCESS != aCProcess.Open(nProcessID, CProcess::OPEN_RIGHT_TYPE_QUERY_ALL))
        {
            return false;
        }

        aCProcess.GetProcessNormal(processInfo.aNormal);

        //////////////////////////////////////////////////////////////////////////

        //内存
        if (nProcessInfoTypes & CProcessHelper::PROCESS_INFO_TYPE_MEMORY)
        {
            aCProcess.GetProcessMemory(processInfo.aMemory);
        }

        //IO计数
        if (nProcessInfoTypes & CProcessHelper::PROCESS_INFO_TYPE_IO)
        {
            aCProcess.GetProcessIOCounter(processInfo.aIOCounter);
        }

        //而外
        if (nProcessInfoTypes & CProcessHelper::PROCESS_INFO_TYPE_ADVANCED)
        {
            aCProcess.GetProcessAdvanced(processInfo.aAdvanced);
        }

        //时间
        if (nProcessInfoTypes & CProcessHelper::PROCESS_INFO_TYPE_TIME)
        {
            aCProcess.GetProcessTime(processInfo.aTime);
        }

        return true;
    }

    bool CProcessHelper::GetProcessIDListByName(const char* szProcessName, list<UINT32>& processIDList)
    {
        if (!szProcessName)
        {
            return false;
        }

        processIDList.clear();

        HANDLE hProcessSnap;
        PROCESSENTRY32 processEntry = {0};
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (hProcessSnap == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        if (!Process32First(hProcessSnap, &processEntry))
        {
            CloseHandle(hProcessSnap);

            return false;
        }

        do
        {
            if (0 == processEntry.th32ProcessID)
            {
                continue;
            }

            if (0 == strcmp(processEntry.szExeFile, szProcessName))
            {
                processIDList.push_back(processEntry.th32ProcessID);
            }
        }
        while(Process32Next(hProcessSnap, &processEntry));

        CloseHandle(hProcessSnap);

        return true;
    }

    bool CProcessHelper::GetProcessIDListByNameW(const WCHAR* szProcessName, list<UINT32>& processIDList)
    {
        if (!szProcessName)
        {
            return false;
        }

        processIDList.clear();

        HANDLE hProcessSnap;
        PROCESSENTRY32W processEntry = {0};
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (hProcessSnap == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        if (!Process32FirstW(hProcessSnap, &processEntry))
        {
            CloseHandle(hProcessSnap);

            return false;
        }

        do
        {
            if (0 == processEntry.th32ProcessID)
            {
                continue;
            }

            if (0 == wcscmp(processEntry.szExeFile, szProcessName))
            {
                processIDList.push_back(processEntry.th32ProcessID);
            }
        }
        while(Process32NextW(hProcessSnap, &processEntry));

        CloseHandle(hProcessSnap);

        return true;
    }

    UINT32 CProcessHelper::GetProcessCount()
    {
        PERFORMANCE_INFORMATION performanceInformation = {0};

        GetPerformanceInfo(&performanceInformation, sizeof(PERFORMANCE_INFORMATION));

        return performanceInformation.ProcessCount;
    }

    bool CProcessHelper::EnableDebugPrivilege(bool bEnable)
    {
        bool bRet = FALSE;      
        HANDLE hToken = NULL;        

        if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
        {      
            TOKEN_PRIVILEGES tp;      
            tp.PrivilegeCount = 1;      
            LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);      
            tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;      
            AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);     
            bRet = (GetLastError()==ERROR_SUCCESS);      
            CloseHandle(hToken);      
        }      

        return bRet;   
    }
}