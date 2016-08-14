#include "../SysBase_Interface.h"
#include <iphlpapi.h>
#include <tcpmib.h>

#pragma comment(lib, "Iphlpapi.lib")

namespace SysBase
{
    CSystemNetInfo::CSystemNetInfo()
    {

    }

    CSystemNetInfo::~CSystemNetInfo()
    {

    }

    bool CSystemNetInfo::GetTcpNetInfoList(list<TTcpNetInfo>& tcpNetInfoList )
    {
        char szBuffer[1024 * 128] = {0};
        DWORD dwSize = 1024 * 128;

        DWORD dwError = GetExtendedTcpTable(szBuffer, &dwSize, TRUE, AF_INET, 
            TCP_TABLE_OWNER_PID_ALL, NULL);

        if (NO_ERROR == dwError)
        {
            PMIB_TCPTABLE_OWNER_PID pTcpTable = (PMIB_TCPTABLE_OWNER_PID)szBuffer;

            for (DWORD dwIndex = 0; dwIndex < pTcpTable->dwNumEntries; ++dwIndex)
            {
                PMIB_TCPROW_OWNER_PID pTcpRow = &pTcpTable->table[dwIndex];

                if (pTcpRow->dwOwningPid > 0)
                {
                    TTcpNetInfo aTTcpNetInfo = {0};
                    aTTcpNetInfo.nProcessID = pTcpRow->dwOwningPid;
                    aTTcpNetInfo.nStatus = pTcpRow->dwState;
                    aTTcpNetInfo.nLocalIP = pTcpRow->dwLocalAddr;
                    aTTcpNetInfo.nLocalPort = ntohs((UINT16)pTcpRow->dwLocalPort);
                    aTTcpNetInfo.nRemoteIP = pTcpRow->dwRemoteAddr;
                    aTTcpNetInfo.nRemotePort = ntohs((UINT16)pTcpRow->dwRemotePort);

                    tcpNetInfoList.push_back(aTTcpNetInfo);
                }
            }
        }
        else if (ERROR_INSUFFICIENT_BUFFER == dwError)
        {
            char* pBuffer = new(std::nothrow) char[dwSize];

            if (!pBuffer)
            {
                return false;
            }

            DWORD dwError = GetExtendedTcpTable(pBuffer, &dwSize, TRUE, AF_INET, 
                TCP_TABLE_OWNER_PID_ALL, NULL);

            PMIB_TCPTABLE_OWNER_PID pTcpTable = (PMIB_TCPTABLE_OWNER_PID)pBuffer;

            for (DWORD dwIndex = 0; dwIndex < pTcpTable->dwNumEntries; ++dwIndex)
            {
                PMIB_TCPROW_OWNER_PID pTcpRow = &pTcpTable->table[dwIndex];

                if (pTcpRow->dwOwningPid > 0)
                {
                    TTcpNetInfo aTTcpNetInfo = {0};
                    aTTcpNetInfo.nProcessID = pTcpRow->dwOwningPid;
                    aTTcpNetInfo.nStatus = pTcpRow->dwState;
                    aTTcpNetInfo.nLocalIP = pTcpRow->dwLocalAddr;
                    aTTcpNetInfo.nLocalPort = ntohs((UINT16)pTcpRow->dwLocalPort);
                    aTTcpNetInfo.nRemoteIP = pTcpRow->dwRemoteAddr;
                    aTTcpNetInfo.nRemotePort = ntohs((UINT16)pTcpRow->dwRemotePort);

                    tcpNetInfoList.push_back(aTTcpNetInfo);
                }
            }

            delete[] pBuffer;
            pBuffer = NULL;
        }

        return dwError == NO_ERROR ? true : false;
    }

    bool CSystemNetInfo::GetUdpNetInfoList(list<TUdpNetInfo>& udpNetInfoList)
    {
        char szBuffer[1024 * 64] = {0};
        DWORD dwSize = 1024 * 64;

        DWORD dwError = GetExtendedUdpTable(szBuffer, &dwSize, TRUE, AF_INET, 
            UDP_TABLE_OWNER_PID, NULL);

        if (NO_ERROR == dwError)
        {
            PMIB_UDPTABLE_OWNER_PID pUdpTable = (PMIB_UDPTABLE_OWNER_PID)szBuffer;

            for (DWORD dwIndex = 0; dwIndex < pUdpTable->dwNumEntries; ++dwIndex)
            {
                PMIB_UDPROW_OWNER_PID pUdpRow = &pUdpTable->table[dwIndex];

                if (pUdpRow->dwOwningPid > 0)
                {
                    TUdpNetInfo aTUdpNetInfo = {0};
                    aTUdpNetInfo.nProcessID = pUdpRow->dwOwningPid;
                    aTUdpNetInfo.nLocalIP = pUdpRow->dwLocalAddr;
                    aTUdpNetInfo.nLocalPort = ntohs((UINT16)pUdpRow->dwLocalPort);

                    udpNetInfoList.push_back(aTUdpNetInfo);
                }
            }
        }
        else if (ERROR_INSUFFICIENT_BUFFER == dwError)
        {
            char* pBuffer = new(std::nothrow) char[dwSize];

            if (!pBuffer)
            {
                return false;
            }

            DWORD dwError = GetExtendedUdpTable(pBuffer, &dwSize, TRUE, AF_INET, 
                UDP_TABLE_OWNER_PID, NULL);

            PMIB_UDPTABLE_OWNER_PID pUdpTable = (PMIB_UDPTABLE_OWNER_PID)pBuffer;

            for (DWORD dwIndex = 0; dwIndex < pUdpTable->dwNumEntries; ++dwIndex)
            {
                PMIB_UDPROW_OWNER_PID pUdpRow = &pUdpTable->table[dwIndex];

                if (pUdpRow->dwOwningPid > 0)
                {
                    TUdpNetInfo aTUdpNetInfo = {0};
                    aTUdpNetInfo.nProcessID = pUdpRow->dwOwningPid;
                    aTUdpNetInfo.nLocalIP = pUdpRow->dwLocalAddr;
                    aTUdpNetInfo.nLocalPort = ntohs((UINT16)pUdpRow->dwLocalPort);

                    udpNetInfoList.push_back(aTUdpNetInfo);
                }
            }

            delete[] pBuffer;
            pBuffer = NULL;
        }

        return dwError == NO_ERROR ? true : false;
    }
}