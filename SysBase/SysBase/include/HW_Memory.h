#pragma once

#include "SysBase.h"

namespace SysBase
{
    struct THWMemoryInfo 
    {  
        UINT32 nMemoryLoad;                 //内存使用率（0~100）
        UINT64 nllTotalPhys;                //总共的物理内存数（字节）
        UINT64 nllAvailPhys;                //可用的物理内存数（字节）
        UINT64 nllTotalPageFile;            //总共的提交的物理内存数（字节）
        UINT64 nllAvailPageFile;            //可用的提交的物理内存数(字节)
        UINT64 nllTotalVirtual;             //总共的虚拟内存数（字节）
        UINT64 nllAvailVirtual;             //可用的虚拟内存数（字节）
        UINT64 nllAvailExtendedVirtual;     //额外可用的虚拟内存数（字节）
    };

    class CHWMemory
    {
    public:

        static bool GetMemoryInfo(THWMemoryInfo& aMemoryInfo);

    protected:
    private:

        CHWMemory();

        virtual ~CHWMemory();
    };
}
