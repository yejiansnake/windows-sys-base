#pragma once

#include "SysBase.h"

namespace SysBase
{
    struct TDiskPerforance
    {
        UINT64 nBytesRead;       //总读字节数
        UINT64 nBytesWritten;   //总写字节数
        UINT64 nReadTime;       //读总共消耗的时间
        UINT64 nWriteTime;      //写总共消耗的时间
        UINT64 nIdleTime;       //总空闲时间
        UINT32 nReadCount;      //总读次数
        UINT32 nWriteCount;     //总写次数
        UINT32 nQueueDepth;     //队列深度（暂时不知道用途）
        UINT32 nSplitCount;     //IO相关IO 累计计数（暂时不知道用途）
    };

    class CHWDisk
    {
    public:

        static void GetDiskNumberList(list<UINT32>& diskNumberList);

        static bool GetDiskSize(UINT32 diskNumber, UINT64& nDiskSize);

        static bool GetDiskPerformance(UINT32 diskNumber, TDiskPerforance& aTDiskPerforance);

    private:

        CHWDisk();

        virtual ~CHWDisk();
    };
}
