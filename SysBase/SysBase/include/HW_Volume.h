#pragma once

#include "SysBase.h"

namespace SysBase
{
    typedef list<string> VolumeNameList;

    struct TVolumeSpace
    {
        UINT64 nFreeBytesAvailable;     //有效空闲字节
        UINT64 nTotalNumberOfBytes;     //总字节
        UINT64 nTotalNumberOfFreeBytes; //总空闲字节
    };

    class CHWVolume
    {
    public:

        static void GetVolumeNameList(VolumeNameList& volumeNameList);

        static bool GetDiskNumber(const char* szVolumeName, UINT32& nDiskNumber);

        static bool GetVolumeSpace(const char* szVolumeName, TVolumeSpace& volumeSpace);

    private:

        CHWVolume();

        virtual ~CHWVolume();
    };

}