#include "../SysBase_Interface.h"
#include "iphlpapi.h"
#pragma comment(lib, "iphlpapi.lib")  

namespace SysBase
{
    bool GetPerEthernetAdaptersInfo(UINT32 nIndex, THWEthernetAdapterInfo& aEthernetAdapterInfo)
    {
        char buffer[10240] = {0};
        DWORD dwPerAdapterInfoSize = 10240;
        PIP_PER_ADAPTER_INFO pPerAdapterInfo = (PIP_PER_ADAPTER_INFO)buffer;
        PIP_ADDR_STRING pIPAddrString = NULL;

        DWORD dwError = GetPerAdapterInfo(nIndex, pPerAdapterInfo, &dwPerAdapterInfoSize);

        if(ERROR_SUCCESS == dwError)
        {
            aEthernetAdapterInfo.bAutoconfigEnabled = pPerAdapterInfo->AutoconfigEnabled == 0 ? false : true;
            aEthernetAdapterInfo.bAutoconfigActive = pPerAdapterInfo->AutoconfigActive == 0 ? false : true;

            pIPAddrString = &pPerAdapterInfo->DnsServerList;

            while (pIPAddrString)
            {
                aEthernetAdapterInfo.dnsAddrList.push_back(pIPAddrString->IpAddress.String);

                pIPAddrString = pIPAddrString->Next;
            }
        }  
        else if (ERROR_BUFFER_OVERFLOW == dwError)
        {
            char* newBuffer = new(std::nothrow) char[dwPerAdapterInfoSize];

            if (!newBuffer)
            {
                return false;
            }

            pPerAdapterInfo = (PIP_PER_ADAPTER_INFO)newBuffer;

            dwError = GetPerAdapterInfo(nIndex, pPerAdapterInfo, &dwPerAdapterInfoSize);

            if (ERROR_SUCCESS == dwError)
            {
                aEthernetAdapterInfo.bAutoconfigEnabled = pPerAdapterInfo->AutoconfigEnabled == 0 ? false : true;
                aEthernetAdapterInfo.bAutoconfigActive = pPerAdapterInfo->AutoconfigActive == 0 ? false : true;

                pIPAddrString = &pPerAdapterInfo->DnsServerList;

                while (pIPAddrString)
                {
                    aEthernetAdapterInfo.dnsAddrList.push_back(pIPAddrString->IpAddress.String);

                    pIPAddrString = pIPAddrString->Next;
                }
            }

            delete[] newBuffer;
            newBuffer = NULL;
        }

        return dwError == ERROR_SUCCESS ? true : false;
    }

    bool GetEthernetAdaptersInfo(PIP_ADAPTER_INFO pAdapterInfo, THWEthernetAdapterInfo& aEthernetAdapterInfo)
    {
        if (!pAdapterInfo)
        {
            return false;
        }

        char szTemp[1024] = {0};
        PIP_ADDR_STRING pIPAddrString = NULL;

        aEthernetAdapterInfo.Name = pAdapterInfo->AdapterName;
        aEthernetAdapterInfo.Description = pAdapterInfo->Description;

        sprintf_s(szTemp, "%02X-%02X-%02X-%02X-%02X-%02X",
            pAdapterInfo->Address[0],  
            pAdapterInfo->Address[1],  
            pAdapterInfo->Address[2],  
            pAdapterInfo->Address[3],  
            pAdapterInfo->Address[4],  
            pAdapterInfo->Address[5]);
        aEthernetAdapterInfo.macAddr = szTemp;

        aEthernetAdapterInfo.nIndex = pAdapterInfo->Index;
        aEthernetAdapterInfo.bDhcpEnabled = pAdapterInfo->DhcpEnabled == 0 ? false : true;

        aEthernetAdapterInfo.nLeaseObtained = pAdapterInfo->LeaseObtained;
        aEthernetAdapterInfo.nLeaseExpires = pAdapterInfo->LeaseExpires;

        //////////////////////////////////////////////////////////////////////////
        //取IP地址

        pIPAddrString = &pAdapterInfo->IpAddressList;

        while (pIPAddrString)
        {
            aEthernetAdapterInfo.ipAddrList[pIPAddrString->IpAddress.String] = pIPAddrString->IpMask.String;

            pIPAddrString = pIPAddrString->Next;
        }

        //////////////////////////////////////////////////////////////////////////
        //取网关地址

        pIPAddrString = &pAdapterInfo->GatewayList;

        while (pIPAddrString)
        {
            aEthernetAdapterInfo.gateWayList.push_back(pIPAddrString->IpAddress.String);

            pIPAddrString = pIPAddrString->Next;
        }

        //////////////////////////////////////////////////////////////////////////
        //取DHCP服务地址

        pIPAddrString = &pAdapterInfo->DhcpServer;

        while (pIPAddrString)
        {
            aEthernetAdapterInfo.dhcpSrvList.push_back(pIPAddrString->IpAddress.String);

            pIPAddrString = pIPAddrString->Next;
        }

        //////////////////////////////////////////////////////////////////////////
        //取WINS服务地址

        aEthernetAdapterInfo.bHaveWins = pAdapterInfo->HaveWins ? true : false;

        if (aEthernetAdapterInfo.bHaveWins)
        {
            //主WINS
            pIPAddrString = &pAdapterInfo->PrimaryWinsServer;

            while (pIPAddrString)
            {
                aEthernetAdapterInfo.primaryWinsSrvList.push_back(pIPAddrString->IpAddress.String);

                pIPAddrString = pIPAddrString->Next;
            }

            //次WINS
            pIPAddrString = &pAdapterInfo->SecondaryWinsServer;

            while (pIPAddrString)
            {
                aEthernetAdapterInfo.secondaryWinsSrvList.push_back(pIPAddrString->IpAddress.String);

                pIPAddrString = pIPAddrString->Next;
            }
        }

        //////////////////////////////////////////////////////////////////////////
        //取DNS地址

        GetPerEthernetAdaptersInfo(aEthernetAdapterInfo.nIndex, aEthernetAdapterInfo);

        //////////////////////////////////////////////////////////////////////////
        //获取网卡但前的连接状态

        MIB_IFROW ifrow;
        memset(&ifrow,0,sizeof(MIB_IFROW));
        ifrow.dwIndex = pAdapterInfo->Index;
        PMIB_IFROW pIfRow(&ifrow);
        GetIfEntry(&ifrow);

        aEthernetAdapterInfo.bConnected = ifrow.dwOperStatus == IF_OPER_STATUS_OPERATIONAL ? true : false;

        return true;
    }

    //////////////////////////////////////////////////////////////////////////

    CHWEthernetAdapter::CHWEthernetAdapter()
    {
    }

    CHWEthernetAdapter::~CHWEthernetAdapter()
    {
    }

    bool CHWEthernetAdapter::GetEthernetAdaptersInfoList(list<THWEthernetAdapterInfo>& ethernetAdapterInfoList)
    {
        char buffer[10240] = {0};
        DWORD dwAdapterInfoSize = 10240;
        PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)buffer;

        DWORD dwError = GetAdaptersInfo(pAdapterInfo, &dwAdapterInfoSize);

        if(ERROR_SUCCESS == dwError)
        {
            while (pAdapterInfo)
            {
                //if (MIB_IF_TYPE_ETHERNET ==  pAdapterInfo->Type)
                {
                    THWEthernetAdapterInfo aEthernetAdapterInfo;

                    if (GetEthernetAdaptersInfo(pAdapterInfo, aEthernetAdapterInfo))
                    {
                        ethernetAdapterInfoList.push_back(aEthernetAdapterInfo);
                    }
                }

                pAdapterInfo = pAdapterInfo->Next;
            }
        }  
        else if (ERROR_BUFFER_OVERFLOW == dwError)
        {
            char* newBuffer = new(std::nothrow) char[dwAdapterInfoSize];

            if (!newBuffer)
            {
                return false;
            }

            pAdapterInfo = (PIP_ADAPTER_INFO)newBuffer;

            dwError = GetAdaptersInfo(pAdapterInfo, &dwAdapterInfoSize);

            if (ERROR_SUCCESS == dwError)
            {
                while (pAdapterInfo)
                {
                    if (MIB_IF_TYPE_ETHERNET ==  pAdapterInfo->Type)
                    {
                        THWEthernetAdapterInfo aEthernetAdapterInfo;

                        if (GetEthernetAdaptersInfo(pAdapterInfo, aEthernetAdapterInfo))
                        {
                            ethernetAdapterInfoList.push_back(aEthernetAdapterInfo);
                        }
                    }

                    pAdapterInfo = pAdapterInfo->Next;
                }
            }

            delete[] newBuffer;
            newBuffer = NULL;
        }

        return dwError == ERROR_SUCCESS ? true : false;
    }

    bool CHWEthernetAdapter::GertEthernetAdaptersIndexList(list<UINT32>& ethernetAdapterIndexList)
    {
        char buffer[10240] = {0};
        DWORD dwAdapterInfoSize = 10240;
        PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)buffer;

        DWORD dwError = GetAdaptersInfo(pAdapterInfo, &dwAdapterInfoSize);

        if(ERROR_SUCCESS == dwError)
        {
            while (pAdapterInfo)
            {
                if (MIB_IF_TYPE_ETHERNET ==  pAdapterInfo->Type)
                {
                    ethernetAdapterIndexList.push_back(pAdapterInfo->Index);
                }

                pAdapterInfo = pAdapterInfo->Next;
            }
        }  
        else if (ERROR_BUFFER_OVERFLOW == dwError)
        {
            char* newBuffer = new(std::nothrow) char[dwAdapterInfoSize];

            if (!newBuffer)
            {
                return false;
            }

            pAdapterInfo = (PIP_ADAPTER_INFO)newBuffer;

            dwError = GetAdaptersInfo(pAdapterInfo, &dwAdapterInfoSize);

            if (ERROR_SUCCESS == dwError)
            {
                while (pAdapterInfo)
                {
                    if (MIB_IF_TYPE_ETHERNET ==  pAdapterInfo->Type)
                    {
                        ethernetAdapterIndexList.push_back(pAdapterInfo->Index);
                    }

                    pAdapterInfo = pAdapterInfo->Next;
                }
            }

            delete[] newBuffer;
            newBuffer = NULL;
        }

        return dwError == ERROR_SUCCESS ? true : false;
    }

    bool CHWEthernetAdapter::GetEthernetAdaptersPerformance(
        UINT32 ethernetAdapterIndex, 
        TEthernetAdaptersPerformance& ethernetAdapterPerformanceOut)
    {
        MIB_IFROW IfRow = {0};
        IfRow.dwIndex = ethernetAdapterIndex;

        ZeroMemory(&ethernetAdapterPerformanceOut, sizeof(ethernetAdapterPerformanceOut));

        if (NO_ERROR != GetIfEntry(&IfRow))
        {
            return false;
        }

        ethernetAdapterPerformanceOut.nIndex = ethernetAdapterIndex;
        ethernetAdapterPerformanceOut.nMtu = IfRow.dwMtu;
        ethernetAdapterPerformanceOut.nSpeed = IfRow.dwSpeed;
        ethernetAdapterPerformanceOut.nAdminStatus = IfRow.dwAdminStatus;
        ethernetAdapterPerformanceOut.nLastChange = IfRow.dwLastChange;
        ethernetAdapterPerformanceOut.nInOctets = IfRow.dwInOctets;
        ethernetAdapterPerformanceOut.nInUcastPkts = IfRow.dwInUcastPkts;
        ethernetAdapterPerformanceOut.nInNUcastPkts = IfRow.dwInNUcastPkts; 
        ethernetAdapterPerformanceOut.nInDiscards = IfRow.dwInDiscards;
        ethernetAdapterPerformanceOut.nInErrors = IfRow.dwInErrors;
        ethernetAdapterPerformanceOut.nInUnknownProtos = IfRow.dwInUnknownProtos;
        ethernetAdapterPerformanceOut.nOutOctets = IfRow.dwOutOctets;
        ethernetAdapterPerformanceOut.nOutUcastPkts = IfRow.dwOutUcastPkts;
        ethernetAdapterPerformanceOut.nOutNUcastPkts = IfRow.dwOutNUcastPkts;
        ethernetAdapterPerformanceOut.nOutDiscards = IfRow.dwOutDiscards;
        ethernetAdapterPerformanceOut.nOutErrors = IfRow.dwOutErrors;
        ethernetAdapterPerformanceOut.nOutQueueLen = IfRow.dwOutQLen;

        return true;
    }
}