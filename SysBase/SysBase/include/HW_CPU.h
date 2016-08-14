#pragma once

#include "SysBase.h"

namespace SysBase
{
    struct THWCpuInfo 
    {
        UINT32 nProcessorCount;
        UINT32 nNumaNodeCount;
        UINT32 nProcessorCoreCount;
        UINT32 nLogicalProcessorCount;
    };

    struct THWCpuDesc
    {
        UINT32 nSpeed;
        string strName;
        string strIdentify;
        string strProductor;
    };

    class CHWCpu
    {
    public:

        static bool GetCpuInfo(THWCpuInfo& aCpuInfo);

        static bool GetCpuDesc(THWCpuDesc& aCpuDesc);

    private:

        CHWCpu();

        virtual ~CHWCpu();
    };
}
