#include "../SysBase_Interface.h"

#define SYSBASE_MAX_SERVICE_SIZE 1024 * 64
#define SYSBASE_MAX_SERVICE_QUERY_SIZE 1024 * 8

namespace SysBase
{
    CServicesHelper::CServicesHelper()
    {

    }

    CServicesHelper::~CServicesHelper()
    {

    }

    bool CServicesHelper::GetServiceInfo(list<TServiceInfo>& serviceInfoList)
    {
        serviceInfoList.clear();

        SC_HANDLE hSCHandle = OpenSCManagerA(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);

        if (!hSCHandle)
        {
            return false;
        }

        char buffer[SYSBASE_MAX_SERVICE_SIZE] = {0};

        DWORD cbBytesNeeded = NULL; 
        DWORD ServicesReturned = NULL;
        DWORD ResumeHandle = NULL;

        if (!EnumServicesStatusExA(hSCHandle, 
            SC_ENUM_PROCESS_INFO, 
            SERVICE_WIN32,
            SERVICE_STATE_ALL,
            (LPBYTE)buffer,
            SYSBASE_MAX_SERVICE_SIZE,
            &cbBytesNeeded,
            &ServicesReturned,
            &ResumeHandle,
            NULL))
        {
            CloseServiceHandle(hSCHandle);

            return false;
        }

        TServiceInfo aTServiceInfo;

        ENUM_SERVICE_STATUS_PROCESSA* pServiceStatusProcess = (ENUM_SERVICE_STATUS_PROCESSA*)buffer;

        for (DWORD dwIndex = 0; dwIndex < ServicesReturned; ++dwIndex)
        {
            aTServiceInfo.nProcessID = pServiceStatusProcess->ServiceStatusProcess.dwProcessId;
            aTServiceInfo.nServicesType = pServiceStatusProcess->ServiceStatusProcess.dwCurrentState;
            aTServiceInfo.strServiceName = pServiceStatusProcess[dwIndex].lpServiceName;
            aTServiceInfo.strDisplayName = pServiceStatusProcess[dwIndex].lpDisplayName;

            switch (pServiceStatusProcess[dwIndex].ServiceStatusProcess.dwCurrentState)
            {
            case SERVICE_CONTINUE_PENDING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_CONTINUE_PENDING;
                }
                break;
            case SERVICE_PAUSE_PENDING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_PAUSE_PENDING;
                }
                break;
            case SERVICE_PAUSED:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_PAUSED;
                }
                break;
            case SERVICE_RUNNING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_RUNNING;
                }
                break;
            case SERVICE_START_PENDING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_START_PENDING;
                }
                break;
            case SERVICE_STOP_PENDING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_STOP_PENDING;
                }
                break;
            case SERVICE_STOPPED:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_STOPPED;
                }
                break;
            default:
                break;
            }

            SC_HANDLE hServiceHandle = OpenServiceA(hSCHandle, pServiceStatusProcess[dwIndex].lpServiceName, SERVICE_QUERY_CONFIG);

            if (hServiceHandle)
            {
                char szConfig[SYSBASE_MAX_SERVICE_QUERY_SIZE] = {0};

                LPQUERY_SERVICE_CONFIGA pQueryConfig = (LPQUERY_SERVICE_CONFIGA)szConfig;
                DWORD pdwBytesNeeded = 0;

                if (QueryServiceConfigA(hServiceHandle, pQueryConfig, SYSBASE_MAX_SERVICE_QUERY_SIZE, &pdwBytesNeeded))
                {
                    aTServiceInfo.strStartName = pQueryConfig->lpServiceStartName;
                    aTServiceInfo.strBinaryPathName = pQueryConfig->lpBinaryPathName;

                    switch (pQueryConfig->dwStartType)
                    {
                    case SERVICE_AUTO_START:
                        {
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_AUTO_START;
                        }
                        break;
                    case SERVICE_BOOT_START:
                        {
                            //驱动程序服务才会到该位置，因为只获取WIN32程序服务，所以不会进入这里
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_BOOT_START;
                        }
                        break;
                    case SERVICE_DEMAND_START:
                        {
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_DEMAND_START;
                        }
                        break;
                    case SERVICE_DISABLED:
                        {
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_DISABLED;
                        }
                        break;
                    case SERVICE_SYSTEM_START:
                        {
                            //驱动程序服务才会到该位置，因为只获取WIN32程序服务，所以不会进入这里
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_SYSTEM_START;
                        }
                        break;
                    default:
                        break;
                    }
                }

                CloseServiceHandle(hServiceHandle);
            }

            serviceInfoList.push_back(aTServiceInfo);
        }

        CloseServiceHandle(hSCHandle);

        return true;
    }

    bool CServicesHelper::GetServiceInfo(list<TServiceInfoW>& serviceInfoList)
    {
        serviceInfoList.clear();

        SC_HANDLE hSCHandle = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);

        if (!hSCHandle)
        {
            return false;
        }

        char buffer[SYSBASE_MAX_SERVICE_SIZE] = {0};

        DWORD cbBytesNeeded = NULL; 
        DWORD ServicesReturned = NULL;
        DWORD ResumeHandle = NULL;

        if (!EnumServicesStatusExW(hSCHandle, 
            SC_ENUM_PROCESS_INFO, 
            SERVICE_WIN32,
            SERVICE_STATE_ALL,
            (LPBYTE)buffer,
            SYSBASE_MAX_SERVICE_SIZE,
            &cbBytesNeeded,
            &ServicesReturned,
            &ResumeHandle,
            NULL))
        {
            CloseServiceHandle(hSCHandle);

            return false;
        }

        TServiceInfoW aTServiceInfo;

        ENUM_SERVICE_STATUS_PROCESSW* pServiceStatusProcess = (ENUM_SERVICE_STATUS_PROCESSW*)buffer;

        for (DWORD dwIndex = 0; dwIndex < ServicesReturned; ++dwIndex)
        {
            aTServiceInfo.nProcessID = pServiceStatusProcess->ServiceStatusProcess.dwProcessId;
            aTServiceInfo.nServicesType = pServiceStatusProcess->ServiceStatusProcess.dwServiceType;
            aTServiceInfo.strServiceName = pServiceStatusProcess[dwIndex].lpServiceName;
            aTServiceInfo.strDisplayName = pServiceStatusProcess[dwIndex].lpDisplayName;

            switch (pServiceStatusProcess[dwIndex].ServiceStatusProcess.dwCurrentState)
            {
            case SERVICE_CONTINUE_PENDING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_CONTINUE_PENDING;
                }
                break;
            case SERVICE_PAUSE_PENDING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_PAUSE_PENDING;
                }
                break;
            case SERVICE_PAUSED:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_PAUSED;
                }
                break;
            case SERVICE_RUNNING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_RUNNING;
                }
                break;
            case SERVICE_START_PENDING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_START_PENDING;
                }
                break;
            case SERVICE_STOP_PENDING:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_STOP_PENDING;
                }
                break;
            case SERVICE_STOPPED:
                {
                    aTServiceInfo.curState = SYSBASE_SERVICE_STATE_STOPPED;
                }
                break;
            default:
                break;
            }

            SC_HANDLE hServiceHandle = OpenServiceW(hSCHandle, pServiceStatusProcess[dwIndex].lpServiceName, SERVICE_QUERY_CONFIG);

            if (hServiceHandle)
            {
                char szConfig[SYSBASE_MAX_SERVICE_QUERY_SIZE] = {0};

                LPQUERY_SERVICE_CONFIGW pQueryConfig = (LPQUERY_SERVICE_CONFIGW)szConfig;
                DWORD pdwBytesNeeded = 0;

                if (QueryServiceConfigW(hServiceHandle, pQueryConfig, SYSBASE_MAX_SERVICE_QUERY_SIZE, &pdwBytesNeeded))
                {
                    aTServiceInfo.strStartName = pQueryConfig->lpServiceStartName;
                    aTServiceInfo.strBinaryPathName = pQueryConfig->lpBinaryPathName;

                    switch (pQueryConfig->dwStartType)
                    {
                    case SERVICE_AUTO_START:
                        {
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_AUTO_START;
                        }
                        break;
                    case SERVICE_BOOT_START:
                        {
                            //驱动程序服务才会到该位置，因为只获取WIN32程序服务，所以不会进入这里
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_BOOT_START;
                        }
                        break;
                    case SERVICE_DEMAND_START:
                        {
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_DEMAND_START;
                        }
                        break;
                    case SERVICE_DISABLED:
                        {
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_DISABLED;
                        }
                        break;
                    case SERVICE_SYSTEM_START:
                        {
                            //驱动程序服务才会到该位置，因为只获取WIN32程序服务，所以不会进入这里
                            aTServiceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_SYSTEM_START;
                        }
                        break;
                    default:
                        break;
                    }
                }

                CloseServiceHandle(hServiceHandle);
            }

            serviceInfoList.push_back(aTServiceInfo);
        }

        CloseServiceHandle(hSCHandle);

        return true;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::GetServiceInfo(const char* szServiceName, TServiceInfo& serviceInfo)
    {
        SC_HANDLE scmHandle = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);

        if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE scHandle=OpenServiceA(scmHandle, szServiceName, GENERIC_READ);

        if (!scHandle)
        {
            ::CloseServiceHandle(scmHandle);
            return ERROR_CODE_SYSTEM;
        }

        CHAR szDiplayName[1024] = {0};
        DWORD dwSize = 1024;

        if (!GetServiceDisplayNameA(scHandle, szServiceName, szDiplayName, &dwSize))
        {
            return ERROR_CODE_SYSTEM;
        }
        else
        {
            serviceInfo.strServiceName = szServiceName;
            serviceInfo.strDisplayName = szDiplayName;
        }

        char szConfig[SYSBASE_MAX_SERVICE_QUERY_SIZE] = {0};

        LPQUERY_SERVICE_CONFIGA pQueryConfig = (LPQUERY_SERVICE_CONFIGA)szConfig;
        DWORD pdwBytesNeeded = 0;

        if (!QueryServiceConfigA(scHandle, pQueryConfig, SYSBASE_MAX_SERVICE_QUERY_SIZE, &pdwBytesNeeded))
        {
            return ERROR_CODE_SYSTEM;
        }
        else
        {
            serviceInfo.strStartName = pQueryConfig->lpServiceStartName;
            serviceInfo.strBinaryPathName = pQueryConfig->lpBinaryPathName;
            serviceInfo.nServicesType = pQueryConfig->dwServiceType;

            switch (pQueryConfig->dwStartType)
            {
            case SERVICE_AUTO_START:
                {
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_AUTO_START;
                }
                break;
            case SERVICE_BOOT_START:
                {
                    //驱动程序服务才会到该位置，因为只获取WIN32程序服务，所以不会进入这里
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_BOOT_START;
                }
                break;
            case SERVICE_DEMAND_START:
                {
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_DEMAND_START;
                }
                break;
            case SERVICE_DISABLED:
                {
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_DISABLED;
                }
                break;
            case SERVICE_SYSTEM_START:
                {
                    //驱动程序服务才会到该位置，因为只获取WIN32程序服务，所以不会进入这里
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_SYSTEM_START;
                }
                break;
            default:
                break;
            }
        }

        SERVICE_STATUS_PROCESS srvStatusProc = {0};
        DWORD dwByteNeeded = 0;
        if (!QueryServiceStatusEx(scHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&srvStatusProc, sizeof(srvStatusProc), &dwByteNeeded))
        {
            return ERROR_CODE_SYSTEM;
        }
        else
        {
            switch (srvStatusProc.dwCurrentState)
            {
            case SERVICE_CONTINUE_PENDING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_CONTINUE_PENDING;
                }
                break;
            case SERVICE_PAUSE_PENDING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_PAUSE_PENDING;
                }
                break;
            case SERVICE_PAUSED:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_PAUSED;
                }
                break;
            case SERVICE_RUNNING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_RUNNING;
                }
                break;
            case SERVICE_START_PENDING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_START_PENDING;
                }
                break;
            case SERVICE_STOP_PENDING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_STOP_PENDING;
                }
                break;
            case SERVICE_STOPPED:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_STOPPED;
                }
                break;
            default:
                break;
            }

            serviceInfo.nProcessID = srvStatusProc.dwProcessId;
        }

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);

        return ERROR_CODE_SUCCESS;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::GetServiceInfo(const WCHAR* szServiceName, TServiceInfoW& serviceInfo)
    {
        SC_HANDLE scmHandle = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);

        if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE scHandle=OpenServiceW(scmHandle, szServiceName, GENERIC_READ);

        if (!scHandle)
        {
            ::CloseServiceHandle(scmHandle);
            return ERROR_CODE_SYSTEM;
        }

        WCHAR szDiplayName[1024] = {0};
        DWORD dwSize = 1024;

        if (!GetServiceDisplayNameW(scHandle, szServiceName, szDiplayName, &dwSize))
        {
            return ERROR_CODE_SYSTEM;
        }
        else
        {
            serviceInfo.strServiceName = szServiceName;
            serviceInfo.strDisplayName = szDiplayName;
        }

        char szConfig[SYSBASE_MAX_SERVICE_QUERY_SIZE] = {0};

        LPQUERY_SERVICE_CONFIGW pQueryConfig = (LPQUERY_SERVICE_CONFIGW)szConfig;
        DWORD pdwBytesNeeded = 0;

        if (!QueryServiceConfigW(scHandle, pQueryConfig, SYSBASE_MAX_SERVICE_QUERY_SIZE, &pdwBytesNeeded))
        {
            return ERROR_CODE_SYSTEM;
        }
        else
        {
            serviceInfo.strStartName = pQueryConfig->lpServiceStartName;
            serviceInfo.strBinaryPathName = pQueryConfig->lpBinaryPathName;
            serviceInfo.nServicesType = pQueryConfig->dwServiceType;

            switch (pQueryConfig->dwStartType)
            {
            case SERVICE_AUTO_START:
                {
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_AUTO_START;
                }
                break;
            case SERVICE_BOOT_START:
                {
                    //驱动程序服务才会到该位置，因为只获取WIN32程序服务，所以不会进入这里
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_BOOT_START;
                }
                break;
            case SERVICE_DEMAND_START:
                {
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_DEMAND_START;
                }
                break;
            case SERVICE_DISABLED:
                {
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_DISABLED;
                }
                break;
            case SERVICE_SYSTEM_START:
                {
                    //驱动程序服务才会到该位置，因为只获取WIN32程序服务，所以不会进入这里
                    serviceInfo.runType = SYSBASE_SERVICE_RUN_TYPE_SYSTEM_START;
                }
                break;
            default:
                break;
            }
        }

        SERVICE_STATUS_PROCESS srvStatusProc = {0};
        DWORD dwByteNeeded = 0;
        if (!QueryServiceStatusEx(scHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&srvStatusProc, sizeof(srvStatusProc), &dwByteNeeded))
        {
            return ERROR_CODE_SYSTEM;
        }
        else
        {
            switch (srvStatusProc.dwCurrentState)
            {
            case SERVICE_CONTINUE_PENDING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_CONTINUE_PENDING;
                }
                break;
            case SERVICE_PAUSE_PENDING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_PAUSE_PENDING;
                }
                break;
            case SERVICE_PAUSED:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_PAUSED;
                }
                break;
            case SERVICE_RUNNING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_RUNNING;
                }
                break;
            case SERVICE_START_PENDING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_START_PENDING;
                }
                break;
            case SERVICE_STOP_PENDING:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_STOP_PENDING;
                }
                break;
            case SERVICE_STOPPED:
                {
                    serviceInfo.curState = SYSBASE_SERVICE_STATE_STOPPED;
                }
                break;
            default:
                break;
            }

            serviceInfo.nProcessID = srvStatusProc.dwProcessId;
        }

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);

        return ERROR_CODE_SUCCESS;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::InstallService(const char* szServiceName,const char* szDisplayName, const char* szExePath)
    {
	    SC_HANDLE scmHandle = OpenSCManagerA(NULL,NULL,SC_MANAGER_ALL_ACCESS);

	    if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE scHandle= ::CreateServiceA(
            scmHandle,
            szServiceName, 
            szDisplayName,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START, 
            SERVICE_ERROR_NORMAL, 
            szExePath, 
            NULL, NULL, NULL, NULL, NULL);

	    if (!scHandle)
	    {
		    ::CloseServiceHandle(scmHandle);
		    return ERROR_CODE_SYSTEM;
	    }

        SERVICE_FAILURE_ACTIONS failure_action = {0};
        failure_action.dwResetPeriod = 0;				//reset failure count to zero  的时间，单位为秒
        failure_action.lpRebootMsg = NULL;				//Message to broadcast to server users before rebooting  
        failure_action.lpCommand = NULL;				//Command line of the process for the CreateProcess function to execute in response 
        failure_action.cActions = 3;					//action数组的个数
        SC_ACTION actionarray[3];
        actionarray[0].Type = SC_ACTION_RESTART;		//重新启动服务
        actionarray[0].Delay = 60000;					//单位为毫秒
        actionarray[1].Type = SC_ACTION_RESTART;
        actionarray[1].Delay = 60000;
        actionarray[2].Type = SC_ACTION_RESTART;
        actionarray[2].Delay = 60000;
        failure_action.lpsaActions = actionarray;

        if(!ChangeServiceConfig2A(scHandle, SERVICE_CONFIG_FAILURE_ACTIONS, &failure_action))
        {
            //_tprintf(TEXT("Set Service Restart failed\n"));
        }

	    CloseServiceHandle(scHandle);
	    CloseServiceHandle(scmHandle);

	    return ERROR_CODE_SUCCESS;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::InstallService(const WCHAR* szServiceName,const WCHAR* szDisplayName, const WCHAR* szExePath)
    {
        SC_HANDLE scmHandle = OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);

        if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE scHandle= ::CreateServiceW(
            scmHandle,
            szServiceName, 
            szDisplayName,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START, 
            SERVICE_ERROR_NORMAL, 
            szExePath, 
            NULL, NULL, NULL, NULL, NULL);

        if (!scHandle)
        {
            ::CloseServiceHandle(scmHandle);
            return ERROR_CODE_SYSTEM;
        }

        SERVICE_FAILURE_ACTIONSW failure_action = {0};
        failure_action.dwResetPeriod = 0;				//reset failure count to zero  的时间，单位为秒
        failure_action.lpRebootMsg = NULL;				//Message to broadcast to server users before rebooting  
        failure_action.lpCommand = NULL;				//Command line of the process for the CreateProcess function to execute in response 
        failure_action.cActions = 3;					//action数组的个数
        SC_ACTION actionarray[3];
        actionarray[0].Type = SC_ACTION_RESTART;		//重新启动服务
        actionarray[0].Delay = 60000;					//单位为毫秒
        actionarray[1].Type = SC_ACTION_RESTART;
        actionarray[1].Delay = 60000;
        actionarray[2].Type = SC_ACTION_RESTART;
        actionarray[2].Delay = 60000;
        failure_action.lpsaActions = actionarray;

        if(!ChangeServiceConfig2W(scHandle, SERVICE_CONFIG_FAILURE_ACTIONS, &failure_action))
        {
            //_tprintf(TEXT("Set Service Restart failed\n"));
        }

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);

        return ERROR_CODE_SUCCESS;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::UnInstallService(const char* szServiceName)
    {
	    SC_HANDLE scmHandle = OpenSCManagerA(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	    if (!scmHandle)
        {
		    return ERROR_CODE_INVALID_HANDLE;
        }

	    SC_HANDLE scHandle = ::OpenServiceA(scmHandle, szServiceName, SERVICE_ALL_ACCESS);
	    if (!scHandle)
	    {
		    ::CloseServiceHandle(scmHandle);
		    return ERROR_CODE_SYSTEM;
	    }

	    BOOL bRet = ::DeleteService(scHandle);

	    CloseServiceHandle(scHandle);
	    CloseServiceHandle(scmHandle);

	    if (!bRet)
	    {
		    return ERROR_CODE_SYSTEM;
	    }

	    return ERROR_CODE_SUCCESS;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::UnInstallService(const WCHAR* szServiceName)
    {
        SC_HANDLE scmHandle = OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);
        if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE scHandle = ::OpenServiceW(scmHandle, szServiceName, SERVICE_ALL_ACCESS);
        if (!scHandle)
        {
            ::CloseServiceHandle(scmHandle);
            return ERROR_CODE_SYSTEM;
        }

        BOOL bRet = ::DeleteService(scHandle);

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);

        if (!bRet)
        {
            return ERROR_CODE_SYSTEM;
        }

        return ERROR_CODE_SUCCESS;
    }

    bool CServicesHelper::IsServiceExist(const char* szServiceName)
    {
        SC_HANDLE scmHandle = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!scmHandle)
        {
            return false;
        }

        SC_HANDLE scHandle = ::OpenServiceA(scmHandle, szServiceName, SERVICE_ALL_ACCESS);
        if (!scHandle)
        {
            ::CloseServiceHandle(scmHandle);
            return false;
        }

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);

        return true;
    }

    bool CServicesHelper::IsServiceExist(const WCHAR* szServiceName)
    {
        SC_HANDLE scmHandle = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!scmHandle)
        {
            return false;
        }

        SC_HANDLE scHandle = ::OpenServiceW(scmHandle, szServiceName, SERVICE_ALL_ACCESS);
        if (!scHandle)
        {
            ::CloseServiceHandle(scmHandle);
            return false;
        }

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);

        return true;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::RunService(const char* szServiceName)
    {
	    SC_HANDLE scmHandle = OpenSCManagerA(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	    if (!scmHandle)
        {
		    return ERROR_CODE_INVALID_HANDLE;
        }

	    SC_HANDLE sHandle = OpenServiceA(scmHandle, szServiceName, SERVICE_START);
	    if (!sHandle)
	    {
		    CloseServiceHandle(scmHandle);
		    return ERROR_CODE_SYSTEM;
	    }	

        ERROR_CODE errorCode = ERROR_CODE_SUCCESS;
	
	    if (!::StartServiceA(sHandle, 0, NULL))
	    {
		    errorCode =  ERROR_CODE_SYSTEM;
	    }

        CloseServiceHandle(sHandle);
        CloseServiceHandle(scmHandle);
	
        return errorCode;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::RunService(const WCHAR* szServiceName)
    {
        SC_HANDLE scmHandle = OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);
        if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE sHandle = OpenServiceW(scmHandle, szServiceName, SERVICE_START);
        if (!sHandle)
        {
            CloseServiceHandle(scmHandle);
            return ERROR_CODE_SYSTEM;
        }	

        ERROR_CODE errorCode = ERROR_CODE_SUCCESS;

        if (!::StartServiceW(sHandle, 0, NULL))
        {
            errorCode =  ERROR_CODE_SYSTEM;
        }

        CloseServiceHandle(sHandle);
        CloseServiceHandle(scmHandle);

        return errorCode;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::StopService(const char* szServiceName)
    {
        SERVICE_STATUS ServiceStatus = {0};

	    SC_HANDLE scmHandle = OpenSCManagerA(NULL,NULL,SC_MANAGER_ALL_ACCESS);

	    if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE scHandle = OpenServiceA(scmHandle, szServiceName, SERVICE_STOP|SERVICE_QUERY_STATUS);

	    if (!scHandle)
	    {
		    ::CloseServiceHandle(scmHandle);
		    return ERROR_CODE_SYSTEM;
	    }
	
        ERROR_CODE errorCode = ERROR_CODE_SUCCESS;

	    if (QueryServiceStatus(scHandle, &ServiceStatus))
	    {
            if (SERVICE_STOPPED != ServiceStatus.dwCurrentState)
            {
                if(SERVICE_RUNNING == ServiceStatus.dwCurrentState || SERVICE_PAUSED == ServiceStatus.dwCurrentState)
                {
                    if (ControlService(scHandle, SERVICE_CONTROL_STOP, &ServiceStatus))
                    {
                        UINT32 ulTry = 0;
                        for (ulTry = 0; ulTry < 100; ++ulTry)
                        {
                            if (QueryServiceStatus(scHandle, &ServiceStatus))
                            {
                                if (SERVICE_STOPPED == ServiceStatus.dwCurrentState)
                                {
                                    break;
                                }
                                else
                                {
                                    Sleep(100);
                                }
                            }
                            else
                            {
                                errorCode = ERROR_CODE_SYSTEM;
                                break;
                            }
                        }

                        Sleep(200);
                    }
                    else
                    {
                        if (QueryServiceStatus(scHandle,&ServiceStatus))
                        {
                            if (SERVICE_STOPPED != ServiceStatus.dwCurrentState)
                            {
                                errorCode = ERROR_CODE_SYSTEM;
                            }
                        }
                        else
                        {
                            errorCode =  ERROR_CODE_SYSTEM;
                        }
                    }
                }
                else
                {
                    errorCode = ERROR_CODE_SYSTEM;
                }
            }
	    }
        else
        {
            errorCode =  ERROR_CODE_SYSTEM;
        }

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);
        return errorCode;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::StopService(const WCHAR* szServiceName)
    {
        SERVICE_STATUS ServiceStatus = {0};

        SC_HANDLE scmHandle = OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);

        if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE scHandle = OpenServiceW(scmHandle, szServiceName, SERVICE_STOP|SERVICE_QUERY_STATUS);

        if (!scHandle)
        {
            ::CloseServiceHandle(scmHandle);
            return ERROR_CODE_SYSTEM;
        }

        ERROR_CODE errorCode = ERROR_CODE_SUCCESS;

        if (QueryServiceStatus(scHandle, &ServiceStatus))
        {
            if (SERVICE_STOPPED != ServiceStatus.dwCurrentState)
            {
                if(SERVICE_RUNNING == ServiceStatus.dwCurrentState || SERVICE_PAUSED == ServiceStatus.dwCurrentState)
                {
                    if (ControlService(scHandle, SERVICE_CONTROL_STOP, &ServiceStatus))
                    {
                        UINT32 ulTry = 0;
                        for (ulTry = 0; ulTry < 100; ++ulTry)
                        {
                            if (QueryServiceStatus(scHandle, &ServiceStatus))
                            {
                                if (SERVICE_STOPPED == ServiceStatus.dwCurrentState)
                                {
                                    break;
                                }
                                else
                                {
                                    Sleep(100);
                                }
                            }
                            else
                            {
                                errorCode = ERROR_CODE_SYSTEM;
                                break;
                            }
                        }

                        Sleep(200);
                    }
                    else
                    {
                        if (QueryServiceStatus(scHandle,&ServiceStatus))
                        {
                            if (SERVICE_STOPPED != ServiceStatus.dwCurrentState)
                            {
                                errorCode = ERROR_CODE_SYSTEM;
                            }
                        }
                        else
                        {
                            errorCode =  ERROR_CODE_SYSTEM;
                        }
                    }
                }
                else
                {
                    errorCode = ERROR_CODE_SYSTEM;
                }
            }
        }
        else
        {
            errorCode =  ERROR_CODE_SYSTEM;
        }

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);
        return errorCode;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::PauseService(const char* szServiceName)
    {
	    //暂停Service
	    SERVICE_STATUS ServiceStatus = {0};

	    SC_HANDLE scmHandle = OpenSCManagerA(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	    if (!scmHandle)
        {
		    return ERROR_CODE_INVALID_HANDLE;
        }

	    SC_HANDLE sHandle=OpenServiceA(scmHandle, szServiceName,SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);

	    if(sHandle)
        {
		    if (QueryServiceStatus(sHandle, &ServiceStatus))
            {
                if(SERVICE_RUNNING == ServiceStatus.dwCurrentState)
                {
                    ControlService(sHandle,SERVICE_CONTROL_PAUSE,&ServiceStatus);
                }
            }
            else
            {
                return ERROR_CODE_SYSTEM;
            }
        }

	    return ERROR_CODE_SUCCESS;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::PauseService(const WCHAR* szServiceName)
    {
        //暂停Service
        SERVICE_STATUS ServiceStatus = {0};

        SC_HANDLE scmHandle = OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);
        if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE sHandle=OpenServiceW(scmHandle, szServiceName,SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);

        if(sHandle)
        {
            if (QueryServiceStatus(sHandle, &ServiceStatus))
            {
                if(SERVICE_RUNNING == ServiceStatus.dwCurrentState)
                {
                    ControlService(sHandle,SERVICE_CONTROL_PAUSE,&ServiceStatus);
                }
            }
            else
            {
                return ERROR_CODE_SYSTEM;
            }
        }

        return ERROR_CODE_SUCCESS;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::ContinueService(const char* szServiceName)
    {
	    //继续Service
	    SERVICE_STATUS ServiceStatus = {0};

	    SC_HANDLE scmHandle = OpenSCManagerA(NULL,NULL,SC_MANAGER_ALL_ACCESS);

	    if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

	    SC_HANDLE scHandle = OpenServiceA(scmHandle, szServiceName, SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);

	    if(scHandle)
        {
		    if (QueryServiceStatus(scHandle, &ServiceStatus))
            {
                if(SERVICE_PAUSED == ServiceStatus.dwCurrentState)
                {
                    ControlService(scHandle,SERVICE_CONTROL_CONTINUE,&ServiceStatus);
                }
            }
            else
            {
                return ERROR_CODE_SYSTEM;
            }
        }

	    return ERROR_CODE_SUCCESS;
    }

    CServicesHelper::ERROR_CODE CServicesHelper::ContinueService(const WCHAR* szServiceName)
    {
        //继续Service
        SERVICE_STATUS ServiceStatus = {0};

        SC_HANDLE scmHandle = OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);

        if (!scmHandle)
        {
            return ERROR_CODE_INVALID_HANDLE;
        }

        SC_HANDLE scHandle = OpenServiceW(scmHandle, szServiceName, SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);

        if(scHandle)
        {
            if (QueryServiceStatus(scHandle, &ServiceStatus))
            {
                if(SERVICE_PAUSED == ServiceStatus.dwCurrentState)
                {
                    ControlService(scHandle,SERVICE_CONTROL_CONTINUE,&ServiceStatus);
                }
            }
            else
            {
                return ERROR_CODE_SYSTEM;
            }
        }

        return ERROR_CODE_SUCCESS;
    }
}