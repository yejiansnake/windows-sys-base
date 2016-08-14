#include "../SysBase_Interface.h"
#include <WinIoCtl.h>
#include <Setupapi.h>
#pragma comment(lib, "Setupapi.lib")

#define MAX_DEVICE 256  
#define INTERFACE_DETAIL_SIZE    (1024)  

namespace SysBase
{
    CHWDisk::CHWDisk()
    {
    
    }

    CHWDisk::~CHWDisk()
    {

    }

    void CHWDisk::GetDiskNumberList(list<UINT32>& diskNumberList)
    {
        diskNumberList.clear();

        //////////////////////////////////////////////////////////////////////////
        //获取设备路径

        list<string> devicePathList;

        const GUID diskGUID = GUID_DEVINTERFACE_DISK;

        HDEVINFO hDevInfoSet;
        SP_DEVICE_INTERFACE_DATA ifdata;

        char tempBuffer[1024] = {0};
        PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)tempBuffer;
        DWORD nCount;
        BOOL bRes;

        hDevInfoSet = SetupDiGetClassDevsA(
            &diskGUID,                                  // class GUID
            NULL,                                       // Enumerator
            NULL,                                       // hwndParent
            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);     // present devices


        //fail...
        if (hDevInfoSet == INVALID_HANDLE_VALUE)
        {
            return;
        }

        nCount = 0;
        bRes = TRUE;

        // device index = 0, 1, 2... test the device interface one by one
        while (bRes)
        {
            ifdata.cbSize = sizeof(ifdata);

            //enumerates the device interfaces that are contained in a device information set
            bRes = SetupDiEnumDeviceInterfaces(
                hDevInfoSet,    // DeviceInfoSet
                NULL,           // DeviceInfoData
                &diskGUID,      // GUID
                nCount,         // MemberIndex
                &ifdata);       // DeviceInterfaceData

            if (bRes)
            {
                ZeroMemory(tempBuffer, sizeof(tempBuffer));
                pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                // get details about a device interface
                bRes = SetupDiGetDeviceInterfaceDetailA(
                    hDevInfoSet,                // DeviceInfoSet
                    &ifdata,                    // DeviceInterfaceData
                    pDetail,                   // DeviceInterfaceDetailData
                    INTERFACE_DETAIL_SIZE,      // DeviceInterfaceDetailDataSize
                    NULL,                       // RequiredSize
                    NULL);                      // DeviceInfoData

                if (bRes)
                {
                    // copy the path to output buffer
                    devicePathList.push_back(pDetail->DevicePath);
                    nCount++;
                }
            }
        }

        SetupDiDestroyDeviceInfoList(hDevInfoSet);

        //////////////////////////////////////////////////////////////////////////
        //获取设备编号

        HANDLE hDevice;
        STORAGE_DEVICE_NUMBER number;
        DWORD readed;

        list<string>::iterator entity = devicePathList.begin();
        list<string>::iterator endEntity = devicePathList.end();

        for (; entity != endEntity; ++entity)
        {
            hDevice = INVALID_HANDLE_VALUE;

            hDevice = CreateFileA(
                entity->c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            if (hDevice == INVALID_HANDLE_VALUE)
            {
                continue;
            }

            bRes = DeviceIoControl(
                hDevice,                            // handle to device
                IOCTL_STORAGE_GET_DEVICE_NUMBER,    // dwIoControlCode
                NULL,                               // lpInBuffer
                0,                                  // nInBufferSize
                &number,                            // output buffer
                sizeof(number),                     // size of output buffer
                &readed,                            // number of bytes returned
                NULL);                              // OVERLAPPED structure

            CloseHandle(hDevice);

            if (!bRes) // fail
            {
                continue;
            }

            diskNumberList.push_back(number.DeviceNumber);
        }
    }

    bool CHWDisk::GetDiskSize(UINT32 diskNumber, UINT64& nDiskSize)
    {
        GET_LENGTH_INFORMATION aLengthInfo = {0};

        char szFilePath[MAX_DEVICE] = {0};
        sprintf(szFilePath, "\\\\.\\PHYSICALDRIVE%d", diskNumber);

        HANDLE hDevice = INVALID_HANDLE_VALUE;
        hDevice = ::CreateFileA(szFilePath, 
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

        DWORD dwBytesReturned = 0;

        BOOL bRes =DeviceIoControl(
            hDevice,
            IOCTL_DISK_GET_LENGTH_INFO,
            NULL,
            0,
            &aLengthInfo,
            sizeof(GET_LENGTH_INFORMATION),
            &dwBytesReturned,
            NULL);

        CloseHandle(hDevice);
        hDevice = NULL;

        if (!bRes)
        {
            return false;
        }

        nDiskSize = aLengthInfo.Length.QuadPart;

        return true;
    }

    bool CHWDisk::GetDiskPerformance(UINT32 diskNumber, TDiskPerforance& aTDiskPerforance)
    {
        char szFilePath[MAX_DEVICE] = {0};
        sprintf(szFilePath, "\\\\.\\PHYSICALDRIVE%d", diskNumber);

        HANDLE hDevice = INVALID_HANDLE_VALUE;
        hDevice = ::CreateFileA(szFilePath, 
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

        DWORD dwBytesReturned = 0;

        DISK_PERFORMANCE diskPerformance = {0};

        BOOL bRes  =DeviceIoControl(
            hDevice,
            IOCTL_DISK_PERFORMANCE,
            NULL,
            0,
            &diskPerformance,
            sizeof(DISK_PERFORMANCE),
            &dwBytesReturned,
            NULL);

        CloseHandle(hDevice);
        hDevice = NULL;

        if (!bRes)
        {
            return false;
        }

        aTDiskPerforance.nBytesRead = diskPerformance.BytesRead.QuadPart;
        aTDiskPerforance.nBytesWritten = diskPerformance.BytesWritten.QuadPart;
        aTDiskPerforance.nReadTime = diskPerformance.ReadTime.QuadPart;
        aTDiskPerforance.nWriteTime = diskPerformance.WriteTime.QuadPart;
        aTDiskPerforance.nIdleTime = diskPerformance.IdleTime.QuadPart;
        aTDiskPerforance.nReadCount = diskPerformance.ReadCount;
        aTDiskPerforance.nWriteCount = diskPerformance.WriteCount;
        aTDiskPerforance.nQueueDepth = diskPerformance.QueueDepth;
        aTDiskPerforance.nSplitCount = diskPerformance.SplitCount;

        return true;
    }
}