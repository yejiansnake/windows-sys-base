#include "../SysBase_Interface.h"

#include <Lm.h>

#pragma comment(lib, "Netapi32.lib")

namespace SysBase
{
    CSystemInfo::SYSTEM_VER CSystemInfo::GetSystemVersion()
    {
        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);

        OSVERSIONINFOEX osVerInfo = {0};

        osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        if (!GetVersionExA((POSVERSIONINFO)&osVerInfo))
        {
            return SYSTEM_VER_NONSUPPORT;
        }

        if (osVerInfo.dwMajorVersion < 5)
        {
            return SYSTEM_VER_NONSUPPORT;
        }

        switch (osVerInfo.dwMajorVersion)
        {
        case 5:
            {
                switch (osVerInfo.dwMinorVersion)
                {
                case 0: //WIN 2000
                    {
                        return SYSTEM_VER_NONSUPPORT;
                    }
                case 1:
                    {
                        return SYSTEM_VER_XP;
                    }
                    break;
                case 2:
                    {
                        if (VER_NT_WORKSTATION == osVerInfo.wProductType
                            && PROCESSOR_ARCHITECTURE_AMD64 == si.wProcessorArchitecture)
                        {
                            return SYSTEM_VER_XP_64;
                        }

                        if (osVerInfo.wSuiteMask & VER_SUITE_WH_SERVER)
                        {
                            return SYSTEM_VER_HOME_SRV;
                        }

                        if (0 == GetSystemMetrics(SM_SERVERR2))
                        {
                            return SYSTEM_VER_2003;
                        }
                        else
                        {
                            return SYSTEM_VER_2003_R2;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case 6:
            {
                switch (osVerInfo.dwMinorVersion)
                {
                case 0:
                    {
                        if (VER_NT_WORKSTATION == osVerInfo.wProductType)
                        {
                            return SYSTEM_VER_VISTA;
                        }
                        else
                        {
                            return SYSTEM_VER_2008;
                        }
                    }
                    break;
                case 1:
                    {
                        if (VER_NT_WORKSTATION == osVerInfo.wProductType)
                        {
                            return SYSTEM_VER_WIN7;
                        }
                        else
                        {
                            return SYSTEM_VER_2008_R2;
                        }
                    }
                case 2:
                    {
                        if (VER_NT_WORKSTATION == osVerInfo.wProductType)
                        {
                            return SYSTEM_VER_WIN8;
                        }
                        else
                        {
                            return SYSTEM_VER_2012;
                        }
                    }
                default:
                    break;
                }
            }
        default:
            break;
        }
        
        return SYSTEM_VER_NONSUPPORT;
    }

    CSystemInfo::SYSTEM_VER_SUITE CSystemInfo::GetSystemVerSuite()
    {
        OSVERSIONINFOEX osVerInfo = {0};

        osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        if (!GetVersionExA((POSVERSIONINFO)&osVerInfo))
        {
            return SYSTEM_VER_SUITE_UNKNOWN;
        }

        if (osVerInfo.wSuiteMask & VER_SUITE_BLADE)
        {
            return SYSTEM_VER_SUITE_WEB;
        }
        else if (osVerInfo.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
        {
            return SYSTEM_VER_SUITE_CLUSTER;
        }
        else if (osVerInfo.wSuiteMask & VER_SUITE_DATACENTER)
        {
            return SYSTEM_VER_SUITE_DATACENTER;
        }
        else if (osVerInfo.wSuiteMask & VER_SUITE_ENTERPRISE)
        {
            return SYSTEM_VER_SUITE_ENTERPRISE;
        }
        else if (osVerInfo.wSuiteMask & VER_SUITE_EMBEDDEDNT)
        {
            return SYSTEM_VER_SUITE_EMBEDDEDNT;
        }
        else if (osVerInfo.wSuiteMask & VER_SUITE_PERSONAL)
        {
            return SYSTEM_VER_SUITE_PERSONAL;
        }
        else if (osVerInfo.wSuiteMask & VER_SUITE_STORAGE_SERVER)
        {
            return SYSTEM_VER_SUITE_STORAGE;
        }
        else if (osVerInfo.wSuiteMask & VER_SUITE_WH_SERVER)
        {
            return SYSTEM_VER_SUITE_HOME_SRV;
        }
        else
        {
            return SYSTEM_VER_SUITE_UNKNOWN;
        }

        return SYSTEM_VER_SUITE_UNKNOWN;
    }

    void CSystemInfo::GetCurComputerName(string& strName)
    {
        CHAR wzName[1024] = {0};
        DWORD dwSize = 2048;

        if (::GetComputerNameA(wzName, &dwSize))
        {
            strName = wzName;
        }
    }

    void CSystemInfo::GetCurComputerName(wstring& strName)
    {
        WCHAR wzName[1024] = {0};
        DWORD dwSize = 2048;

        if (::GetComputerNameW(wzName, &dwSize))
        {
            strName = wzName;
        }
    }

    CSystemInfo::SYSTEM_NET_JOIN_TYPE CSystemInfo::GetNetJoinName(wstring& strMame)
    {
        SYSTEM_NET_JOIN_TYPE netJoinType = SYSTEM_NET_JOIN_TYPE_UN_JOIN;

        WCHAR* wzName = NULL;

        NETSETUP_JOIN_STATUS joinStatus;

        if ( NERR_Success != NetGetJoinInformation(NULL, &wzName, &joinStatus))
        {
            return netJoinType;
        }

        switch (joinStatus)
        {
        case NetSetupWorkgroupName:
            {
                netJoinType = SYSTEM_NET_JOIN_TYPE_WORK_GROUP;
            }
            break;
        case NetSetupDomainName:
            {
                netJoinType = SYSTEM_NET_JOIN_TYPE_DOMAIN;
            }
            break;
        default:
            netJoinType = SYSTEM_NET_JOIN_TYPE_UN_JOIN;
            break;
        }

        if (wzName)
        {
            strMame = wzName;

            NetApiBufferFree(wzName);
            wzName = NULL;
        }
        
        return netJoinType;
    }

    bool CSystemInfo::Is64bitSystem()
    {
        //////////////////////////////////////////////////////////////////////////
        //还有一种方法，通过 GetSystemWow64Directory 函数获取64位操作系统的目录，
        //如果为32位系统，则该函数永远返回 0 （失败）,
        //GetLastError() = ERROR_CALL_NOT_IMPLEMENTED
        //////////////////////////////////////////////////////////////////////////

        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);

        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||    
            si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
        {
            return true;
        }
        else
        {
            return false;
        } 
    }

    bool CSystemInfo::IsRDPRunOn()
    {
        //////////////////////////////////////////////////////////////////////////
        //获取3389端口是否有效

        if (!CSocketLoader::InitSocket())
        {
            return false;
        }

        SOCKET aSocket = socket(AF_INET, SOCK_STREAM, NULL);

        if (INVALID_SOCKET == aSocket)
        {
            return false;
        }

        SOCKADDR_IN sockAddr = {0};
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
        sockAddr.sin_port = htons(3389);

        int nErrorCode = connect(aSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));

        closesocket(aSocket);
        aSocket = INVALID_SOCKET;

        return nErrorCode == SOCKET_ERROR ? false : true;
    }
}