#include "../SysBase_Interface.h"

namespace SysBase
{
    CHWMemory::CHWMemory()
    {

    }

    CHWMemory::~CHWMemory()
    {

    }

    bool CHWMemory::GetMemoryInfo(THWMemoryInfo& aMemoryInfo)
    {
        MEMORYSTATUSEX statex;

        statex.dwLength = sizeof(statex);

        if (!GlobalMemoryStatusEx(&statex))
        {
            return false;
        }

        aMemoryInfo.nMemoryLoad = statex.dwMemoryLoad;
        aMemoryInfo.nllTotalPhys = statex.ullTotalPhys;
        aMemoryInfo.nllAvailPhys = statex.ullAvailPhys;
        aMemoryInfo.nllTotalPageFile = statex.ullTotalPageFile;
        aMemoryInfo.nllAvailPageFile = statex.ullAvailPageFile;
        aMemoryInfo.nllTotalVirtual = statex.ullTotalVirtual;
        aMemoryInfo.nllAvailVirtual = statex.ullAvailVirtual;
        aMemoryInfo.nllAvailExtendedVirtual = statex.ullAvailExtendedVirtual;

        return true;
    }
}