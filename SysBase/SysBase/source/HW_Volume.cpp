#include "../SysBase_Interface.h"
#include <WinIoCtl.h>
#include <Setupapi.h>
#pragma comment(lib, "Setupapi.lib")
namespace SysBase
{
    CHWVolume::CHWVolume()
    {

    }

    CHWVolume::~CHWVolume()
    {

    }

    void CHWVolume::GetVolumeNameList(VolumeNameList& volumeNameList)
    {
        char szBuffer[10240] = {0};
        DWORD dwSize = 10240;
        DWORD dwLength = GetLogicalDriveStringsA(dwSize, szBuffer);

        if (0 == dwLength)
        {
            return;
        }

        char* pStart = szBuffer;
        char* pEnd = szBuffer + dwLength;
        int nLen = 0;

        while (pStart <= pEnd)
        {
            nLen = (int)strlen(pStart);

            if (nLen <= 0)
            {
                break;
            }

            UINT nFlag = GetDriveType(pStart);

            if (nFlag == DRIVE_FIXED)
            {
                volumeNameList.push_back(pStart);
            }

            pStart = pStart + nLen + 1;
        }
    }

    bool CHWVolume::GetDiskNumber(const char* szVolumeName, UINT32& nDiskNumber)
    {
        if (!szVolumeName)
        {
            return false;
        }

        HANDLE hDevice = INVALID_HANDLE_VALUE;
        BOOL bRes = FALSE;
        DWORD readed = 0;
        STORAGE_DEVICE_NUMBER number = {0};

        CHAR szPath[128] = {0};
        sprintf(szPath, "\\\\.\\%s", szVolumeName);
        char* ptr = strrchr(szPath, ':');
        if (ptr)
        {
            ptr += 1;

            if (*ptr == '\\')
            {
                *ptr = NULL;
            }
        }

        hDevice = CreateFile(szPath,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hDevice == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        
        bRes = DeviceIoControl(
            hDevice,
            IOCTL_STORAGE_GET_DEVICE_NUMBER,
            NULL,
            0,
            &number,
            sizeof(number),
            &readed,
            NULL);

        CloseHandle(hDevice);

        if (!bRes)
        {
            return false;
        }

        nDiskNumber = number.DeviceNumber;

        return true;
    }

    bool CHWVolume::GetVolumeSpace(const char* szVolumeName, TVolumeSpace& volumeSpace)
    {
        if (!szVolumeName)
        {
            return false;
        }

        ULARGE_INTEGER FreeBytesAvailable = {0};
        ULARGE_INTEGER TotalNumberOfBytes = {0};
        ULARGE_INTEGER TotalNumberOfFreeBytes = {0};

        if (!GetDiskFreeSpaceExA(szVolumeName,
            &FreeBytesAvailable,  
            &TotalNumberOfBytes,
            &TotalNumberOfFreeBytes))
        {
            return false;
        }

        volumeSpace.nFreeBytesAvailable = FreeBytesAvailable.QuadPart;
        volumeSpace.nTotalNumberOfBytes = TotalNumberOfBytes.QuadPart;
        volumeSpace.nTotalNumberOfFreeBytes = TotalNumberOfFreeBytes.QuadPart;

        return true;
    }
}