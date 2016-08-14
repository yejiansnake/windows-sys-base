#include "../SysBase_Interface.h"

#pragma comment(lib, "Advapi32.lib")

namespace SysBase
{
    static UINT32 GetLogicalProcessorCount(UINT64 nProcessMask)
    {
        if (1 == nProcessMask)
        {
            return 1;
        }

        DWORD LSHIFT = sizeof(UINT64)*8 - 1;
        DWORD bitSetCount = 0;
        UINT64 bitTest = (UINT64)1 << LSHIFT;    
        DWORD i;

        for (i = 0; i <= LSHIFT; ++i)
        {
            bitSetCount += ((nProcessMask & bitTest)?1:0);
            bitTest/=2;
        }

        return bitSetCount;
    }

    CHWCpu::CHWCpu()
    {

    }

    CHWCpu::~CHWCpu()
    {

    }

    bool CHWCpu::GetCpuInfo(THWCpuInfo& aCpuInfo)
    {
        ZeroMemory(&aCpuInfo, sizeof(THWCpuInfo));

        char buffer[10240] = {0};
        DWORD dwSize = 10240;
        DWORD dwCount = 0;
        DWORD dwIndex = 0;

        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pSysProcessInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)buffer;

        if (GetLogicalProcessorInformation(pSysProcessInfo, &dwSize))
        {
            dwCount = dwSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

            for (dwIndex = 0; dwIndex < dwCount; ++dwIndex)
            {
                switch (pSysProcessInfo[dwIndex].Relationship)
                {
                case RelationProcessorCore:
                    {
                        aCpuInfo.nProcessorCoreCount++;

                        UINT64 nProcessMask = pSysProcessInfo[dwIndex].ProcessorMask;

                        aCpuInfo.nLogicalProcessorCount += GetLogicalProcessorCount(nProcessMask);
                    }
                    break;
                case RelationProcessorPackage:
                    {
                        aCpuInfo.nProcessorCount++;
                    }
                    break;
                case RelationNumaNode:
                    {
                        aCpuInfo.nNumaNodeCount++;
                    }
                    break;
                default:
                    break;
                } 
            }
        }
        else
        {
            return false;
        }

        if (0 ==aCpuInfo.nProcessorCount)
        {
            aCpuInfo.nProcessorCount = 1;
        }

        return true;
    }

    bool CHWCpu::GetCpuDesc(THWCpuDesc& aCpuDesc)
    {
        char szRegPath[] = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
        char szBuffer[1024] = {0};
        DWORD dwBufferSize = 1024;
        HKEY RegKey = NULL;

        ZeroMemory(&aCpuDesc, sizeof(aCpuDesc));

        // 由注册表中取得各项 CPU 信息
        if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_QUERY_VALUE, &RegKey))
        {
            return false;
        }

        // 取得 CPU 名称
        ZeroMemory(szBuffer, 1024);
        dwBufferSize = 1024;
        if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "ProcessorNameString", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
        {
            aCpuDesc.strName = szBuffer;
        }

        // 取得 CPU 速度
        dwBufferSize = sizeof(UINT32);
        RegQueryValueExA(RegKey,"~MHz",NULL,NULL,(LPBYTE)&aCpuDesc.nSpeed, &dwBufferSize);

        // 取得 CPU 标识讯息
        ZeroMemory(szBuffer, 1024);
        dwBufferSize = 1024;
        if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "Identifier", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
        {
            aCpuDesc.strIdentify = szBuffer;
        }

        // 取得生产厂商信息
        ZeroMemory(szBuffer, 1024);
        dwBufferSize = 1024;
        if (ERROR_SUCCESS == RegQueryValueExA(RegKey, "VendorIdentifier", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
        {
            aCpuDesc.strProductor = szBuffer;
        }

        RegCloseKey(RegKey);

        RegKey = NULL;

        return true;
    }
}