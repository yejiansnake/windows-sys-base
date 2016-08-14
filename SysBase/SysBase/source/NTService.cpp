#include <windows.h>
#include "winsvc.h"
#include <stdio.h>
#include <crtdbg.h>
#include <Shellapi.h>

//#pragma warning(disable : 4081 4267 4311)

#include <io.h>			//!! TCW MOD
#include <fcntl.h>		//!! TCW MOD


#include "NTService.h"
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//

//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3

//
// MessageId: MSG_INFO_1
//
// MessageText:
//
//  %1
//
#define MSG_INFO_1                       ((DWORD)0x40000000L)

//
// MessageId: MSG_WARNING_1
//
// MessageText:
//
//  %1
//
#define MSG_WARNING_1                    ((DWORD)0x80000001L)

//
// MessageId: MSG_ERROR_1
//
// MessageText:
//
//  %1
//
#define MSG_ERROR_1                      ((DWORD)0xC0000002L)

//
// MessageId: MSG_SUCCESS_1
//
// MessageText:
//
//  %1
//
#define MSG_SUCCESS_1                    ((DWORD)0x00000003L)


#ifndef RSP_SIMPLE_SERVICE
	#define RSP_SIMPLE_SERVICE 1
#endif
#ifndef RSP_UNREGISTER_SERVICE
	#define RSP_UNREGISTER_SERVICE 0
#endif

namespace SysBase
{
    BOOL CNTService::m_bInstance = FALSE;

    static CNTService*  s_pTheServiceA = 0;			// the one and only instance

	CNTService* CNTService::GetService()
	{
		return s_pTheServiceA;
	}

    static LPCSTR gszAppRegKey = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
    static LPCSTR gszWin95ServKey= "Software\\Microsoft\\Windows\\CurrentVersion\\RunServices";	//!! TCW MOD
    //static LPCSTR gszWin95ServKey= "Software\\Microsoft\\Windows\\CurrentVersion\\Run");	//!! TCW MOD

    /////////////////////////////////////////////////////////////////////////////
    // class CNTService -- construction/destruction

    CNTService::CNTService(LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpDesc)
        : m_lpServiceName(lpServiceName)
        , m_lpDisplayName(lpDisplayName ? lpDisplayName : lpServiceName)
        , m_lpDesc(lpDesc ? lpDesc : lpServiceName)
        , m_dwCheckPoint(0)
        , m_dwErr(0)
        , m_bDebug(FALSE)
        , m_sshStatusHandle(0)
        , m_dwControlsAccepted(SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_PAUSE_CONTINUE)
        , m_pUserSID(0)
        , m_fConsoleReady(FALSE)
        // parameters to the "CreateService()" function:
        , m_dwDesiredAccess(SERVICE_ALL_ACCESS)
        , m_dwServiceType(SERVICE_WIN32_OWN_PROCESS)
        , m_dwStartType(SERVICE_AUTO_START)
        , m_dwErrorControl(SERVICE_ERROR_NORMAL)
        , m_pszLoadOrderGroup(0)
        , m_dwTagID(0)
        , m_pszDependencies(0)
        , m_pszStartName(0)
        , m_pszPassword(0)
    {
        _ASSERTE(! m_bInstance);

        OSVERSIONINFOA vi;
        vi.dwOSVersionInfoSize = sizeof(vi);  // init this.
        GetVersionExA(&vi);      //lint !e534
        m_bWinNT = (vi.dwPlatformId == VER_PLATFORM_WIN32_NT);

        m_bInstance = TRUE;
        s_pTheServiceA = this;

        // SERVICE_STATUS members that rarely change
        m_ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        m_ssStatus.dwServiceSpecificExitCode = 0;

        if (m_bWinNT) {
            /////////////////////////////////////////////////////////////////////////
            // Providing a SID (security identifier) was contributed by Victor
            // Vogelpoel (VictorV@Telic.nl).
            // The code from Victor was slightly modified.

            // Get security information of current user
            BYTE	security_identifier_buffer[4096];
            DWORD	dwSizeSecurityIdBuffer = sizeof(security_identifier_buffer);
            PSID	user_security_identifier = NULL;

            char sUserName[256];
            DWORD dwSizeUserName  =  255;

            char sDomainName[256];
            DWORD dwSizeDomainName = 255;

            SID_NAME_USE sidTypeSecurityId;

            ::ZeroMemory(sUserName, sizeof(sUserName));
            ::ZeroMemory(sDomainName, sizeof(sDomainName));
            ::ZeroMemory(security_identifier_buffer, dwSizeSecurityIdBuffer);

            ::GetUserNameA(sUserName, &dwSizeUserName);

            if (::LookupAccountNameA(
                0,
                sUserName,
                &security_identifier_buffer,
                &dwSizeSecurityIdBuffer,
                sDomainName,
                &dwSizeDomainName,
                &sidTypeSecurityId
               )) {
                    if (::IsValidSid(PSID(security_identifier_buffer))) {
                        DWORD dwSidLen = ::GetLengthSid(PSID(security_identifier_buffer));
                        m_pUserSID = PSID(new BYTE [dwSidLen]);
                        ::CopySid(dwSidLen, m_pUserSID, security_identifier_buffer);
                        _ASSERTE(::EqualSid(m_pUserSID, security_identifier_buffer));
                    }
            }
        }
        /////////////////////////////////////////////////////////////////////////
    }


    CNTService::~CNTService() {
        _ASSERTE(m_bInstance);
        delete [] LPBYTE(m_pUserSID);
        m_bInstance = FALSE;
        s_pTheServiceA = 0;
    }



    /////////////////////////////////////////////////////////////////////////////
    // class CNTService -- overridables

#define NEXT_ARG ((((*Argv)[2])== '\0')?(--Argc,*++Argv):(*Argv)+2)


    BOOL CNTService::RegisterService(int argc, char** argv) 
    {
        BOOL (CNTService::* fnc)() = &CNTService::StartDispatcher;
        int Argc;
        LPSTR* Argv;

        Argc = argc;
        Argv = argv;

        while (++Argv, --Argc) 
        {
            if (Argv[0][0] == '-') 
            {
                switch(Argv[0][1]) 
                {
				case 'i':	// install the service
					fnc = &CNTService::InstallService;
					break;
				case 'l':	// login-account (only useful with -i)
					m_pszStartName = NEXT_ARG;
					break;
				case 'p':	// password (only useful with -i)
					m_pszPassword = NEXT_ARG;
					break;
				case 'u':	// uninstall the service
					fnc = &CNTService::RemoveService;
					break;
				case 's':	// start the service
					fnc = &CNTService::StartupService;
					break;
				case 'e':	// end the service
					fnc = &CNTService::EndService;
					break;
				case 'd':	// debug the service
				case 'f':	//!! TCW MOD faceless non-service (Win95) mode

					m_bDebug = TRUE;
					// pass original parameters to DebugService()
					return DebugService(argc, argv,(Argv[0][1] == 'f')); //!! TCW MOD faceless non-service (Win95) mode
                }
            }
        }

        //!! TCW MOD START - if Win95, run as faceless app.
        if (fnc == &CNTService::InstallService)
        {
            (this->*fnc)();
            if (OsIsWin95()) 
            {
                // act as if -f was passed anyways.
                m_bDebug = TRUE;
                return DebugService(argc, argv, TRUE);
            }
            else
            {
                return this->StartupService();
            }
        }


        if (fnc == &CNTService::StartDispatcher && OsIsWin95()) 
        {
            // act as if -f was passed anyways.
            m_bDebug = TRUE;
            return DebugService(argc, argv, TRUE);
        }
        //!! TCW MOD END - if Win95, run as faceless app.

        return (this->*fnc)();
    }


    BOOL CNTService::StartDispatcher() {
        // Default implementation creates a single threaded service.
        // Override this method and provide more table entries for
        // a multithreaded service (one entry for each thread).
        SERVICE_TABLE_ENTRYA dispatchTable[] =
        {
            { LPSTR(m_lpServiceName), (LPSERVICE_MAIN_FUNCTIONA)ServiceMain },
            { 0, 0 }
        };

        BOOL bRet = StartServiceCtrlDispatcherA(dispatchTable);
        if (! bRet) {
            char szBuf[256];
            AddToMessageLog(GetLastErrorText(szBuf,255));
        }

        return bRet;
    }


    BOOL CNTService::InstallService()
    {
        BOOL bRet = FALSE;

        char szPath[1024];
        static LPCSTR gszWin95ServKey = "Software\\Microsoft\\Windows\\CurrentVersion\\RunServices";	//!! TCW MOD

        if (GetModuleFileNameA(0, szPath, 1023) == 0) 
        {
            char szErr[256];
            printf("Unable to install %s - %s\n", m_lpDisplayName, GetLastErrorText(szErr, 256));
            return FALSE;
        }

        if (OsIsWin95()) {	//!! TCW MOD - code added to install as Win95 service
            // Create a key for that application and insert values for
            // "EventMessageFile" and "TypesSupported"
            HKEY hKey = 0;
            LONG lRet = ERROR_SUCCESS;
            if (::RegCreateKeyA(HKEY_LOCAL_MACHINE, gszWin95ServKey , &hKey) == ERROR_SUCCESS) 
            {
                lRet =	::RegSetValueExA(
                    hKey,				    // handle of key to set value for
                    m_lpServiceName,	    // address of value to set (NAME OF SERVICE)
                    0,					    // reserved
                    REG_SZ,		            // flag for value type
                    (CONST BYTE*)szPath,    // address of value data
                    (DWORD)(strlen(szPath) + 1)	// size of value data
                   );
                ::RegCloseKey(hKey);
                bRet=TRUE;
            }
        } else {
            // Real NT services go here.
            SC_HANDLE schSCManager =	OpenSCManagerA(
                0,						// machine (NULL == local)
                0,						// database (NULL == default)
                SC_MANAGER_ALL_ACCESS	// access required
               );
            if (schSCManager) {
                SC_HANDLE schService =	CreateServiceA(
                    schSCManager,
                    m_lpServiceName,
                    m_lpDisplayName,
                    m_dwDesiredAccess,
                    m_dwServiceType,
                    m_dwStartType,
                    m_dwErrorControl,
                    szPath,
                    m_pszLoadOrderGroup,
                    ((m_dwServiceType == SERVICE_KERNEL_DRIVER ||
                    m_dwServiceType == SERVICE_FILE_SYSTEM_DRIVER) &&
                    (m_dwStartType == SERVICE_BOOT_START ||
                    m_dwStartType == SERVICE_SYSTEM_START)) ?
                    &m_dwTagID : 0,
                    m_pszDependencies,
                    m_pszStartName,
                    m_pszPassword
                   );

                if (schService) 
                {
                    // add by sch 2007-08-16
                    SERVICE_FAILURE_ACTIONS  failure_action;
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

                    if (!ChangeServiceConfig2A(schService,SERVICE_CONFIG_FAILURE_ACTIONS,&failure_action))
                    {
                        printf("Set Service Restart failed\n");
                    }

                    SERVICE_DESCRIPTIONA server_desc;
                    server_desc.lpDescription = (LPSTR)m_lpDesc;
                    
                    if (!ChangeServiceConfig2A(schService,SERVICE_CONFIG_DESCRIPTION,&server_desc))
                    {
                        printf("Set Service Desc failed\n");
                    }

                    CloseServiceHandle(schService);
                    bRet = TRUE;
                } 
                else 
                {
                    char szErr[256];
                    GetLastErrorText(szErr, 256);
                    MessageBoxA(NULL, szErr, "Prompt", MB_ICONSTOP);
                }

                CloseServiceHandle(schSCManager);
            } 
            else 
            {
                char szErr[256];
                printf("OpenSCManager failed - %s\n", GetLastErrorText(szErr,256));
            }

            if (bRet) {
                // installation succeeded. Now register the message file
                RegisterApplicationLog(
                    szPath,		// the path to the application itself
                    EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE // supported types
                   );

                AddToMessageLog("Service installed",EVENTLOG_INFORMATION_TYPE);
            }
        }	//!! TCW MOD

        return bRet;
    }


    BOOL CNTService::RemoveService() {
        BOOL bRet = FALSE;

        SetupConsole();	//!! TCW MOD - have to show the console here for the
        // diagnostic or error reason: orignal class assumed
        // that we were using _main for entry (a console app).
        // This particular usage is a Windows app (no console),
        // so we need to create it. Using SetupConsole with _main
        // is ok - does nothing, since you only get one console.


        if (OsIsWin95()) {	//!! TCW MOD - code added to install as Win95 service
            HKEY hKey = 0;
            LONG lRet = ERROR_SUCCESS;
            if (::RegCreateKeyA(HKEY_LOCAL_MACHINE, gszWin95ServKey , &hKey) == ERROR_SUCCESS) {
                lRet = ::RegDeleteValueA(hKey, m_lpServiceName);
                ::RegCloseKey(hKey);
                bRet=TRUE;
            }
        } else {
            // Real NT services go here.
            SC_HANDLE schSCManager = OpenSCManagerA(
                0,						// machine (NULL == local)
                0,						// database (NULL == default)
                SC_MANAGER_ALL_ACCESS	// access required
               );
            if (schSCManager) {
                SC_HANDLE schService =	OpenServiceA(
                    schSCManager,
                    m_lpServiceName,
                    SERVICE_ALL_ACCESS
                   );

                if (schService) {
                    // try to stop the service
                    if (ControlService(schService, SERVICE_CONTROL_STOP, &m_ssStatus)) {
                        printf("Stopping %s.", m_lpDisplayName);
                        Sleep(1000);

                        while(QueryServiceStatus(schService, &m_ssStatus)) {
                            if (m_ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                                printf(".");
                                Sleep(1000);
                            } else
                                break;
                        }

                        if (m_ssStatus.dwCurrentState == SERVICE_STOPPED)
                            printf("\n%s stopped.\n", m_lpDisplayName);
                        else
                            printf("\n%s failed to stop.\n", m_lpDisplayName);
                    }

                    // now remove the service
                    if (DeleteService(schService)) 
                    {
                        printf("%s removed.\n", m_lpDisplayName);
                        bRet = TRUE;
                    } else {
                        char szErr[256];
                        printf("DeleteService failed - %s\n", GetLastErrorText(szErr,256));
                    }

                    CloseServiceHandle(schService);
                } else {
                    char szErr[256];
                    printf("OpenService failed - %s\n", GetLastErrorText(szErr,256));
                }

                CloseServiceHandle(schSCManager);
            } else {
                char szErr[256];
                MessageBoxA(NULL, GetLastErrorText(szErr,256), "Prompt", MB_OK);
            }

            if (bRet)
                DeregisterApplicationLog();
        }

        return bRet;
    }


    BOOL CNTService::EndService() {
        BOOL bRet = FALSE;

        SC_HANDLE schSCManager = ::OpenSCManagerA(
            0,						// machine (NULL == local)
            0,						// database (NULL == default)
            SC_MANAGER_ALL_ACCESS	// access required
           );
        if (schSCManager) {
            SC_HANDLE schService =	::OpenServiceA(
                schSCManager,
                m_lpServiceName,
                SERVICE_ALL_ACCESS
               );

            if (schService) {
                // try to stop the service
                if (::ControlService(schService, SERVICE_CONTROL_STOP, &m_ssStatus)) {
                    printf("Stopping %s.", m_lpDisplayName);
                    ::Sleep(1000);

                    while(::QueryServiceStatus(schService, &m_ssStatus)) {
                        if (m_ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                            printf(".");
                            ::Sleep(1000);
                        } else
                            break;
                    }

                    if (m_ssStatus.dwCurrentState == SERVICE_STOPPED)
                        bRet = TRUE, printf("\n%s stopped.\n", m_lpDisplayName);
                    else
                        printf("\n%s failed to stop.\n", m_lpDisplayName);
                }

                ::CloseServiceHandle(schService);
            } else {
                char szErr[256];
                printf("OpenService failed - %s\n", GetLastErrorText(szErr,256));
            }

            ::CloseServiceHandle(schSCManager);
        } else {
            char szErr[256];
            printf("OpenSCManager failed - %s\n", GetLastErrorText(szErr,256));
        }

        return bRet;
    }


    BOOL CNTService::StartupService() {
        BOOL bRet = FALSE;

        SC_HANDLE schSCManager = ::OpenSCManagerA(
            0,						// machine (NULL == local)
            0,						// database (NULL == default)
            SC_MANAGER_ALL_ACCESS	// access required
           );
        if (schSCManager) {
            SC_HANDLE schService =	::OpenServiceA(
                schSCManager,
                m_lpServiceName,
                SERVICE_ALL_ACCESS
               );

            if (schService) {
                // try to start the service
                printf("Starting up %s.", m_lpDisplayName);
                if (::StartService(schService, 0, 0)) {
                    Sleep(1000);

                    while(::QueryServiceStatus(schService, &m_ssStatus)) {
                        if (m_ssStatus.dwCurrentState == SERVICE_START_PENDING) {
                            printf(".");
                            Sleep(1000);
                        } else
                            break;
                    }

                    if (m_ssStatus.dwCurrentState == SERVICE_RUNNING)
                        bRet = TRUE, printf("\n%s started.\n", m_lpDisplayName);
                    else
                        printf("\n%s failed to start.\n", m_lpDisplayName);
                } else {
                    // StartService failed
                    char szErr[256];
                    printf("\n%s failed to start: %s\n", m_lpDisplayName, GetLastErrorText(szErr,256));
                }

                ::CloseServiceHandle(schService);
            } else {
                char szErr[256];
                printf("OpenService failed - %s\n", GetLastErrorText(szErr,256));
            }

            ::CloseServiceHandle(schSCManager);
        } else {
            char szErr[256];
            printf("OpenSCManager failed - %s\n", GetLastErrorText(szErr,256));
        }

        return bRet;
    }


    ////////////////////////////////////////////////////////////////////////////
    //!! TCW MOD - faceless window procedure for usage within Win95 (mostly),
    // but can be invoked under NT by using -f
    LRESULT CALLBACK _FacelessWndProc_(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg==WM_QUERYENDSESSION || uMsg==WM_ENDSESSION || uMsg==WM_QUIT) {
            if (lParam==NULL || uMsg==WM_QUIT) {
                DestroyWindow(hwnd);	// kill me
                if (CNTService::GetService()!=NULL)
                    CNTService::GetService()->Stop();	// stop me.
                return TRUE;
            }
        }
        return DefWindowProc(hwnd,uMsg,wParam,lParam);
    }
    ////////////////////////////////////////////////////////////////////////////


    BOOL CNTService::DebugService(int argc, char** argv, BOOL faceless) 
    {
        DWORD dwArgc = (DWORD)argc;
        LPSTR* lpszArgv = argv;

        if (!faceless) {	//!! TCW MOD - no faceless, so give it a face.
            SetupConsole();	//!! TCW MOD - make the console for debugging
            printf("Debugging %s.\n", m_lpDisplayName);

            SetConsoleCtrlHandler(ControlHandler, TRUE);
        }

        //!! TCW MOD START - if Win95, register server
        typedef DWORD (WINAPI* fp_RegServProc)(DWORD dwProcessId,DWORD dwType);
        fp_RegServProc fncptr=NULL;

        if (faceless) 
        {
            WNDCLASSA wndclass;
            memset(&wndclass,0,sizeof(WNDCLASS));
            wndclass.lpfnWndProc = _FacelessWndProc_;
            wndclass.hInstance = HINSTANCE(::GetModuleHandle(0));
            wndclass.lpszClassName = "RRL__FacelessWndProc_";
            ATOM atom = ::RegisterClassA(&wndclass);
            HWND hwnd = ::CreateWindowA(wndclass.lpszClassName,"",0,0,0,0,0,0,0,wndclass.hInstance,0);
            HMODULE hModule = ::GetModuleHandleA("kernel32.dll");
            // punch F1 on "RegisterServiceProcess" for what it does and when to use it.
            fncptr=(fp_RegServProc)::GetProcAddress(hModule, "RegisterServiceProcess");
            if (fncptr!=NULL)
                (*fncptr)(0, RSP_SIMPLE_SERVICE);
        }
        //!! TCW MOD END - if Win95, register server


        Run(dwArgc, lpszArgv);

        if (fncptr!=NULL)		//!! TCW MOD - if it's there, remove it: our run is over
            (*fncptr)(0, RSP_UNREGISTER_SERVICE);

        return TRUE;
    }


    void CNTService::Pause() {

        SC_HANDLE schSCManager = ::OpenSCManagerA(
            0,						// machine (NULL == local)
            0,						// database (NULL == default)
            SC_MANAGER_ALL_ACCESS	// access required
           );
        if (schSCManager) {
            SC_HANDLE schService =	::OpenServiceA(
                schSCManager,
                m_lpServiceName,
                SERVICE_ALL_ACCESS
               );

            if (schService) {
                // try to pause the service
                if (::ControlService(schService, SERVICE_CONTROL_PAUSE, &m_ssStatus)) {
                    printf("pausing %s.", m_lpDisplayName);
                    ::Sleep(1000);

                    while(::QueryServiceStatus(schService, &m_ssStatus)) {
                        if (m_ssStatus.dwCurrentState == SERVICE_PAUSE_PENDING) {
                            printf(".");
                            ::Sleep(1000);
                        } else
                            break;
                    }

                    if (m_ssStatus.dwCurrentState == SERVICE_PAUSED)
                        printf("\n%s paused.\n", m_lpDisplayName);
                    else
                        printf("\n%s failed to pause.\n", m_lpDisplayName);
                }

                ::CloseServiceHandle(schService);
            } else {
                char szErr[256];
                printf("OpenService failed - %s\n", GetLastErrorText(szErr,256));
            }

            ::CloseServiceHandle(schSCManager);
        } else {
            char szErr[256];
            printf("OpenSCManager failed - %s\n", GetLastErrorText(szErr,256));
        }
    }


    void CNTService::Continue() {

        SC_HANDLE schSCManager = ::OpenSCManagerA(
            0,						// machine (NULL == local)
            0,						// database (NULL == default)
            SC_MANAGER_ALL_ACCESS	// access required
           );
        if (schSCManager) {
            SC_HANDLE schService =	::OpenServiceA(
                schSCManager,
                m_lpServiceName,
                SERVICE_ALL_ACCESS
               );

            if (schService) {
                // try to continue the service
                if (::ControlService(schService, SERVICE_CONTROL_CONTINUE, &m_ssStatus)) {
                    printf("Stopping %s.", m_lpDisplayName);
                    ::Sleep(1000);

                    while(::QueryServiceStatus(schService, &m_ssStatus)) {
                        if (m_ssStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) {
                            printf(".");
                            ::Sleep(1000);
                        } else
                            break;
                    }

                    if (m_ssStatus.dwCurrentState == SERVICE_RUNNING)
                        printf("\n%s started.\n", m_lpDisplayName);
                    else
                        printf("\n%s failed to continue.\n", m_lpDisplayName);
                }

                ::CloseServiceHandle(schService);
            } else {
                char szErr[256];
                printf("OpenService failed - %s\n", GetLastErrorText(szErr,256));
            }

            ::CloseServiceHandle(schSCManager);
        } else {
            char szErr[256];
            printf("OpenSCManager failed - %s\n", GetLastErrorText(szErr,256));
        }
    }


    void CNTService::Shutdown() {
    }


    /////////////////////////////////////////////////////////////////////////////
    // class CNTService -- default handlers

    void WINAPI CNTService::ServiceMain(DWORD dwArgc, LPSTR* lpszArgv) {
        _ASSERTE(s_pTheServiceA != 0);

        // register our service control handler:
        s_pTheServiceA->m_sshStatusHandle =	RegisterServiceCtrlHandlerA(
            s_pTheServiceA->m_lpServiceName,
            CNTService::ServiceCtrl
           );

        if (s_pTheServiceA->m_sshStatusHandle)

            // report the status to the service control manager.
            if (s_pTheServiceA->ReportStatus(SERVICE_START_PENDING)){
                s_pTheServiceA->Run(dwArgc, lpszArgv);}

            // try to report the stopped status to the service control manager.
            if (s_pTheServiceA->m_sshStatusHandle)
                s_pTheServiceA->ReportStatus(SERVICE_STOPPED);
    }


    void WINAPI CNTService::ServiceCtrl(DWORD dwCtrlCode) {
        _ASSERTE(s_pTheServiceA != 0);

        // Handle the requested control code.
        switch(dwCtrlCode) {
    case SERVICE_CONTROL_STOP:
        // Stop the service.
        s_pTheServiceA->m_ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
        s_pTheServiceA->Stop();
        break;

    case SERVICE_CONTROL_PAUSE:
        s_pTheServiceA->m_ssStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
        s_pTheServiceA->Pause();
        break;

    case SERVICE_CONTROL_CONTINUE:
        s_pTheServiceA->m_ssStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
        s_pTheServiceA->Continue();
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        s_pTheServiceA->Shutdown();
        break;

    case SERVICE_CONTROL_INTERROGATE:
        // Update the service status.
        s_pTheServiceA->ReportStatus(s_pTheServiceA->m_ssStatus.dwCurrentState);
        break;

    default:
        // invalid control code
        break;
        }

    }


    BOOL WINAPI CNTService::ControlHandler(DWORD dwCtrlType) {
        _ASSERTE(s_pTheServiceA != 0);
        switch(dwCtrlType) {
    case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
    case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
        printf("Stopping %s.\n", s_pTheServiceA->m_lpDisplayName);
        s_pTheServiceA->Stop();
        return TRUE;
        }
        return FALSE;
    }


    /////////////////////////////////////////////////////////////////////////////
    // class CNTService -- helpers

    //!! TCW MOD - added DWORD dwErrExit for error exit value. Defaults to zero
    BOOL CNTService::ReportStatus(
        DWORD dwCurrentState,
        DWORD dwWaitHint,
        DWORD dwErrExit) {
            BOOL fResult = TRUE;

            if (!m_bDebug) { // when debugging we don't report to the SCM
                if (dwCurrentState == SERVICE_START_PENDING)
                    m_ssStatus.dwControlsAccepted = 0;
                else
                    m_ssStatus.dwControlsAccepted = m_dwControlsAccepted;

                m_ssStatus.dwCurrentState = dwCurrentState;
                m_ssStatus.dwWin32ExitCode = NO_ERROR;
                m_ssStatus.dwWaitHint = dwWaitHint;

                //!! TCW MOD START - added code to support error exiting
                m_ssStatus.dwServiceSpecificExitCode = dwErrExit;
                if (dwErrExit!=0)
                    m_ssStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
                //!! TCW MOD END - added code to support error exiting

                if (dwCurrentState == SERVICE_RUNNING ||
                    dwCurrentState == SERVICE_STOPPED)
                    m_ssStatus.dwCheckPoint = 0;
                else
                    m_ssStatus.dwCheckPoint = ++m_dwCheckPoint;

                // Report the status of the service to the service control manager.
                if (!(fResult = SetServiceStatus(m_sshStatusHandle, &m_ssStatus))) {
                    AddToMessageLog("SetServiceStatus() failed");
                }
            }

            return fResult;
    }


    void CNTService::AddToMessageLog(LPSTR lpszMsg, WORD wEventType, DWORD dwEventID) {
        m_dwErr = GetLastError();

        // use default message-IDs
        if (dwEventID == DWORD(-1)) {
            switch(wEventType) {
    case EVENTLOG_ERROR_TYPE:
        dwEventID = MSG_ERROR_1;
        break;
    case EVENTLOG_WARNING_TYPE:
        dwEventID = MSG_WARNING_1;
        break;
    case EVENTLOG_INFORMATION_TYPE:
        dwEventID = MSG_INFO_1;
        break;
    case EVENTLOG_AUDIT_SUCCESS:
        dwEventID = MSG_INFO_1;
        break;
    case EVENTLOG_AUDIT_FAILURE:
        dwEventID = MSG_INFO_1;
        break;
    default:
        dwEventID = MSG_INFO_1;
        break;
            }
        }

        // Use event logging to log the error.
        HANDLE hEventSource = RegisterEventSourceA(0, m_lpServiceName);

        if (hEventSource != 0) {
            LPCSTR lpszMessage = lpszMsg;

            ReportEventA(
                hEventSource,	// handle of event source
                wEventType,		// event type
                0,				// event category
                dwEventID,		// event ID
                m_pUserSID,		// current user's SID
                1,				// strings in lpszStrings
                0,				// no bytes of raw data
                &lpszMessage,	// array of error strings
                0				// no raw data
               );

            ::DeregisterEventSource(hEventSource);
        }
    }


    LPSTR CNTService::GetLastErrorText(LPSTR lpszBuf, DWORD dwSize) {
        LPSTR lpszTemp = 0;

        DWORD dwRet =	::FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
            0,
            GetLastError(),
            LANG_NEUTRAL,
            (LPSTR)&lpszTemp,
            0,
            0
           );

        if (!dwRet || (dwSize < dwRet+14))
        {
            lpszBuf[0] = '\0';
        }
        else 
        {
            lpszTemp[strlen(lpszTemp)-2] = '\0';  //remove cr/nl characters
            strcpy(lpszBuf, lpszTemp);
        }

        if (lpszTemp)
            LocalFree(HLOCAL(lpszTemp));

        return lpszBuf;
    }

    /////////////////////////////////////////////////////////////////////////////
    // class CNTService -- implementation

    void CNTService::RegisterApplicationLog(LPCSTR lpszFileName, DWORD dwTypes) {
        char szKey[256];
        strcpy(szKey, gszAppRegKey);
        strcat(szKey, m_lpServiceName);
        HKEY hKey = 0;
        LONG lRet = ERROR_SUCCESS;

        // Create a key for that application and insert values for
        // "EventMessageFile" and "TypesSupported"
        if (::RegCreateKeyA(HKEY_LOCAL_MACHINE, szKey, &hKey) == ERROR_SUCCESS) {
            lRet =	::RegSetValueExA(
                hKey,						        // handle of key to set value for
                "EventMessageFile",	        // address of value to set
                0,							        // reserved
                REG_EXPAND_SZ,				        // flag for value type
                (CONST BYTE*)lpszFileName,	        // address of value data
                (WORD)(strlen(lpszFileName) + 1)	// size of value data
               );

            // Set the supported types flags.
            lRet =	::RegSetValueExA(
                hKey,					// handle of key to set value for
                "TypesSupported",	// address of value to set
                0,						// reserved
                REG_DWORD,				// flag for value type
                (CONST BYTE*)&dwTypes,	// address of value data
                sizeof(DWORD)			// size of value data
               );
            ::RegCloseKey(hKey);
        }

        // Add the service to the "Sources" value

        lRet =	::RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,	// handle of open key 
            gszAppRegKey,		// address of name of subkey to open 
            0,					// reserved 
            KEY_ALL_ACCESS,		// security access mask 
            &hKey				// address of handle of open key 
           );
        if (lRet == ERROR_SUCCESS) {
            DWORD dwSize;

            // retrieve the size of the needed value
            lRet =	::RegQueryValueExA(
                hKey,			// handle of key to query 
                "Sources",		// address of name of value to query 
                0,				// reserved 
                0,				// address of buffer for value type 
                0,				// address of data buffer 
                &dwSize			// address of data buffer size 
               );

            if (lRet == ERROR_SUCCESS) {
                DWORD dwType;
                DWORD dwNewSize = dwSize+ (DWORD)strlen(m_lpServiceName)+1;
                LPBYTE Buffer = LPBYTE(::GlobalAlloc(GPTR, dwNewSize));

                lRet =	::RegQueryValueExA(
                    hKey,			// handle of key to query 
                    "Sources",		// address of name of value to query 
                    0,				// reserved 
                    &dwType,		// address of buffer for value type 
                    Buffer,			// address of data buffer 
                    &dwSize			// address of data buffer size 
                   );
                if (lRet == ERROR_SUCCESS) {
                    _ASSERTE(dwType == REG_MULTI_SZ);

                    // check whether this service is already a known source
                    register LPSTR p = LPSTR(Buffer);
                    for(;* p; p += strlen(p)+1) {
                        if (strcmp(p, m_lpServiceName) == 0)
                            break;
                    }
                    if (!*  p) {
                        // We're standing at the end of the stringarray
                        // and the service does still not exist in the "Sources".
                        // Now insert it at this point.
                        // Note that we have already enough memory allocated
                        // (see GlobalAlloc() above). We also don't need to append
                        // an additional '\0'. This is done in GlobalAlloc() above
                        // too.
                        strcpy(p, m_lpServiceName);

                        // OK - now store the modified value back into the
                        // registry.
                        lRet =	::RegSetValueExA(
                            hKey,			// handle of key to set value for
                            "Sources",		// address of value to set
                            0,				// reserved
                            dwType,			// flag for value type
                            Buffer,			// address of value data
                            dwNewSize		// size of value data
                           );
                    }
                }

                ::GlobalFree(HGLOBAL(Buffer));
            }

            ::RegCloseKey(hKey);
        }
    }


    void CNTService::DeregisterApplicationLog() {
        char szKey[256];
        strcpy(szKey, gszAppRegKey);
        strcat(szKey, m_lpServiceName);
        HKEY hKey = 0;
        LONG lRet = ERROR_SUCCESS;

        lRet = ::RegDeleteKeyA(HKEY_LOCAL_MACHINE, szKey);

        // now we have to delete the application from the "Sources" value too.
        lRet =	::RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,	// handle of open key 
            gszAppRegKey,		// address of name of subkey to open 
            0,					// reserved 
            KEY_ALL_ACCESS,		// security access mask 
            &hKey				// address of handle of open key 
           );
        if (lRet == ERROR_SUCCESS) {
            DWORD dwSize;

            // retrieve the size of the needed value
            lRet =	::RegQueryValueExA(
                hKey,			// handle of key to query 
                "Sources",		// address of name of value to query 
                0,				// reserved 
                0,				// address of buffer for value type 
                0,				// address of data buffer 
                &dwSize			// address of data buffer size 
               );

            if (lRet == ERROR_SUCCESS) {
                DWORD dwType;
                LPBYTE Buffer = LPBYTE(::GlobalAlloc(GPTR, dwSize));
                LPBYTE NewBuffer = LPBYTE(::GlobalAlloc(GPTR, dwSize));

                lRet =	::RegQueryValueExA(
                    hKey,			// handle of key to query 
                    "Sources",		// address of name of value to query 
                    0,				// reserved 
                    &dwType,		// address of buffer for value type 
                    Buffer,			// address of data buffer 
                    &dwSize			// address of data buffer size 
                   );
                if (lRet == ERROR_SUCCESS) {
                    _ASSERTE(dwType == REG_MULTI_SZ);

                    // check whether this service is already a known source
                    register LPSTR p = LPSTR(Buffer);
                    register LPSTR pNew = LPSTR(NewBuffer);
                    BOOL bNeedSave = FALSE;	// assume the value is already correct
                    for(;* p; p += strlen(p)+1, pNew += strlen(pNew)+1) {
                        // except ourself: copy the source string into the destination
                        if (strcmp(p, m_lpServiceName) != 0)
                            strcpy(pNew, p);
                        else {
                            bNeedSave = TRUE;		    //* this* application found
                            dwSize -= (DWORD)(strlen(p) + 1);	// new size of value
                        }
                    }
                    if (bNeedSave) {
                        // OK - now store the modified value back into the
                        // registry.
                        lRet =	::RegSetValueExA(
                            hKey,			// handle of key to set value for
                            "Sources",		// address of value to set
                            0,				// reserved
                            dwType,			// flag for value type
                            NewBuffer,		// address of value data
                            dwSize			// size of value data
                           );
                    }
                }

                ::GlobalFree(HGLOBAL(Buffer));
                ::GlobalFree(HGLOBAL(NewBuffer));
            }

            ::RegCloseKey(hKey);
        }
    }

    ////////////////////////////////////////////////////////

    //!! TCW MOD - function to create console for faceless apps if not already there
    void CNTService::SetupConsole() {
        if (!m_fConsoleReady) {
            AllocConsole();	// you only get 1 console.

            // lovely hack to get the standard io (printf, getc, etc) to the new console. Pretty much does what the
            // C lib does for us, but when we want it, and inside of a Window'd app.
            // The ugly look of this is due to the error checking (bad return values. Remove the if xxx checks if you like it that way.
            DWORD astds[3]={STD_OUTPUT_HANDLE,STD_ERROR_HANDLE,STD_INPUT_HANDLE};
            FILE* atrgs[3]={stdout,stderr,stdin};
            for (register int i=0; i<3; i++) 
            {
                LONG_PTR hand=(LONG_PTR)GetStdHandle(astds[i]);

                if (hand!=(LONG_PTR)INVALID_HANDLE_VALUE) 
                {
                    int osf=_open_osfhandle(hand, _O_TEXT);

                    if (osf!=-1) 
                    {
                        FILE* fp= _fdopen(osf, (astds[i] == STD_INPUT_HANDLE) ? "r" : "w");

                        if (fp != NULL) 
                        {
                           * (atrgs[i]) =* fp;

                            setvbuf(fp, NULL,_IONBF, 0);
                        }
                    }
                }
            }
            m_fConsoleReady=TRUE;
        }
    }
}
