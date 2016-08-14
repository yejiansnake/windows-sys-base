#include "../SysBase_Interface.h"

#include <psapi.h>
#include <Tlhelp32.h>
#include <winternl.h>
#pragma comment( lib, "PSAPI.LIB" )

typedef BOOL (WINAPI *PQueryFullProcessImageNameA)(HANDLE hProcess, DWORD dwFlags, LPSTR lpExeName, PDWORD lpdwSize);

typedef BOOL (WINAPI *PQueryFullProcessImageNameW)(HANDLE hProcess, DWORD dwFlags, LPWSTR lpExeName, PDWORD lpdwSize);

typedef NTSTATUS (WINAPI *NtQueryInfoProc)(HANDLE,PROCESSINFOCLASS,LPVOID,ULONG,PULONG);

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //

    class CProcessImp;

    //////////////////////////////////////////////////////////////////////////
    //CProcessImpHelper

    class CProcessImpHelper
    {
    public:

        virtual ~CProcessImpHelper();

        static CProcessImpHelper& Instance();

        bool GetFullProcessImageName(HANDLE hProcess, char* lpExeName, UINT32* pnSize);

        bool GetFullProcessImageNameW(HANDLE hProcess, WCHAR* lpExeName, UINT32* pnSize);

        bool GetProcessCommandLineW(HANDLE hProcess, wstring& wstrcommandLineContents);

    protected:
    private:

        CProcessImpHelper();

        HMODULE m_hKernel32Module;

		HMODULE m_hNtDllModule;

        PQueryFullProcessImageNameA m_PQueryFullProcessImageNameA;

        PQueryFullProcessImageNameW m_PQueryFullProcessImageNameW;

		NtQueryInfoProc				m_pNtQueryInfoProc;

        static CProcessImpHelper s_CProcessImpHelper;
    };

    CProcessImpHelper CProcessImpHelper::s_CProcessImpHelper;

    CProcessImpHelper& CProcessImpHelper::Instance()
    {
        return s_CProcessImpHelper;
    }

    CProcessImpHelper::CProcessImpHelper()
    {
        m_hKernel32Module = LoadLibraryA("Kernel32.dll");

        if (m_hKernel32Module)
        {
            m_PQueryFullProcessImageNameA = (PQueryFullProcessImageNameA)GetProcAddress(m_hKernel32Module, "QueryFullProcessImageNameA");

            if (!m_PQueryFullProcessImageNameA)
            {
                FreeLibrary(m_hKernel32Module);

                m_hKernel32Module = NULL;
            }

            m_PQueryFullProcessImageNameW = (PQueryFullProcessImageNameW)GetProcAddress(m_hKernel32Module, "QueryFullProcessImageNameW");

            if (!m_PQueryFullProcessImageNameW)
            {
                FreeLibrary(m_hKernel32Module);

                m_hKernel32Module = NULL;
            }
        }

		m_hNtDllModule = LoadLibraryA("ntdll.dll");
		if (m_hNtDllModule)
		{
		    m_pNtQueryInfoProc =
			(NtQueryInfoProc)GetProcAddress(m_hNtDllModule, "NtQueryInformationProcess");
			if (!m_pNtQueryInfoProc)
			{
				FreeLibrary(m_hNtDllModule);

                m_pNtQueryInfoProc = NULL;
			}
		}

		//m_pNtQueryInfoProc

    }

    CProcessImpHelper::~CProcessImpHelper()
    {
        m_PQueryFullProcessImageNameA = NULL;

        FreeLibrary(m_hKernel32Module);

		 FreeLibrary(m_hNtDllModule);

        m_hKernel32Module = NULL;
    }

    bool CProcessImpHelper::GetFullProcessImageName(HANDLE hProcess, char* lpExeName, UINT32* pnSize)
    {
        if (!m_PQueryFullProcessImageNameA)
        {
            return false;
        }

        DWORD dwSize = *pnSize;

        if (!m_PQueryFullProcessImageNameA(hProcess, 0, lpExeName, &dwSize))
        {
            return false;
        }
        
        *pnSize = dwSize;

        return true;
    }

    bool CProcessImpHelper::GetFullProcessImageNameW(HANDLE hProcess, WCHAR* lpExeName, UINT32* pnSize)
    {
        if (!m_PQueryFullProcessImageNameW)
        {
            return false;
        }

        DWORD dwSize = *pnSize;

        if (!m_PQueryFullProcessImageNameW(hProcess, 0, lpExeName, &dwSize))
        {
            return false;
        }

        *pnSize = dwSize;

        return true;
    }
  
    bool CProcessImpHelper::GetProcessCommandLineW(HANDLE hProcess, wstring& wstrcommandLineContents)
	{
		PROCESS_BASIC_INFORMATION pbi;
		NTSTATUS ret = m_pNtQueryInfoProc(hProcess
				, ProcessBasicInformation
				, &pbi
				, sizeof(pbi)
				, NULL);
		if (ret != ERROR_SUCCESS)
		{
			return false;
		}

		
		PVOID rtlUserProcParamsAddress;
	#ifdef X64
		if (!ReadProcessMemory(hProcess, (PCHAR)pbi.PebBaseAddress + 32,
							   &rtlUserProcParamsAddress, sizeof(PVOID), NULL))
	#else
		if (!ReadProcessMemory(hProcess, (PCHAR)pbi.PebBaseAddress + 0x10,
							   &rtlUserProcParamsAddress, sizeof(PVOID), NULL))
	#endif
		{
			////printf("Could not read the address of ProcessParameters!\n");
			return false;
		}

         // read the CommandLine UNICODE_STRING structure
		UNICODE_STRING commandLine;
	#ifdef X64
		
		if (!ReadProcessMemory(hProcess, (PCHAR)rtlUserProcParamsAddress + 112,
							   &commandLine, sizeof(commandLine), NULL))
	#else
		if (!ReadProcessMemory(hProcess, (PCHAR)rtlUserProcParamsAddress + 0x40,
							   &commandLine, sizeof(commandLine), NULL))
	#endif
		{
			return false;
		}
		WCHAR *commandLineContents = NULL;

		    // allocate memory to hold the command line
		commandLineContents =new WCHAR[commandLine.Length + 1]/* (WCHAR *)malloc(commandLine.Length + 1)*/;
		memset(commandLineContents,0,commandLine.Length + 1);
		if (commandLineContents == NULL) 
		{
			//PyErr_NoMemory();
			delete[]commandLineContents;
		}

		// read the command line
		if (!ReadProcessMemory(hProcess, commandLine.Buffer,
							   commandLineContents, commandLine.Length, NULL))
		{
			delete[]commandLineContents;
			return false;
		}

		commandLineContents[(commandLine.Length / sizeof(WCHAR))] = '\0';
		wstring wstrAllcommandLineContents = commandLineContents;
		if (commandLineContents)
		{
			delete[]commandLineContents;
		}
		wstring temp = wstrAllcommandLineContents;
		int nCount = 0;
		while (true)
		{
			if (nCount == 2)
			{
				size_t nSize = temp.size();
				if (nSize > 0)
				{
					temp = temp.substr(1);
				}
				break;
			}

			wstring space = L"\"";
			size_t pos =  temp.find( L"\"");
			if (pos==string::npos)
			{
				break;
			}
			temp =  temp.substr(pos+1);
			nCount++;
		}

		wstrcommandLineContents =temp;

		return true;
	}
    //////////////////////////////////////////////////////////////////////////
    //CProcessImp 进程类

    class CProcessImp
    {
    public:

        friend class CProcessImpHelper;

        CProcessImp();

        virtual ~CProcessImp();

        CProcess::ERROR_CODE Create(char* lpCommandLine, bool bRun = true, bool bShowWindow = true);

        CProcess::ERROR_CODE Create(WCHAR* lpCommandLine, bool bRun = true, bool bShowWindow = true);

        CProcess::ERROR_CODE Resume();

        CProcess::ERROR_CODE Open(UINT32 nProcessID, CProcess::OPEN_RIGHT_TYPE openRightType = CProcess::OPEN_RIGHT_TYPE_ALL);

        CProcess::ERROR_CODE OpenByName(const char* szProcessName, CProcess::OPEN_RIGHT_TYPE openRightType = CProcess::OPEN_RIGHT_TYPE_ALL);

        CProcess::ERROR_CODE OpenCurrent();

        void Close();

        CProcess::ERROR_CODE Terminate(UINT32 nExitCode);

        CProcess::ERROR_CODE Wait(UINT32 nWaitTime);

        CProcess::ERROR_CODE GetExitCode(UINT32* pnExitCode);

        CProcess::ERROR_CODE ReleaseMemory();

        CProcess::ERROR_CODE GetProcessID(UINT32& nProcessID);

        CProcess::ERROR_CODE GetProcessNormal(TProcessNormal& aTProcessNormal);

        CProcess::ERROR_CODE GetProcessMemory(TProcessMemory& aTProcessMemory);

        CProcess::ERROR_CODE GetProcessIOCounter(TProcessIOCounter& aTProcessIOCounter);

        CProcess::ERROR_CODE GetProcessResource(TProcessResource& aTProcessResource);

        CProcess::ERROR_CODE GetProcessAdvanced(TProcessAdvanced& aTProcessAdvanced);

        CProcess::ERROR_CODE GetProcessTime(TProcessTime& aTProcessTime);

    protected:

        ///////////////////////////////
        //数据成员

        PROCESS_INFORMATION m_processInfo;
    };


	//////////////////////////////////////////////////////////////////////////
	//CProcessImp 实现

	CProcessImp::CProcessImp()
	{
		ZeroMemory( &m_processInfo, sizeof(PROCESS_INFORMATION));
	}
	CProcessImp::~CProcessImp()
	{
		this->Close();
	}

	CProcess::ERROR_CODE CProcessImp::Create(char* lpCommandLine, bool bRun, bool bShowWindow)
	{
		if (m_processInfo.hProcess)
		{
			return CProcess::ERROR_CODE_HAS_INIT;
		}

		STARTUPINFOA si;
		ZeroMemory( &m_processInfo, sizeof(PROCESS_INFORMATION));

        UINT32 nCreationFlags = 0;

        if (!bRun)
        {
            nCreationFlags = CREATE_SUSPENDED;
        }

        if (!bShowWindow)
        {
            si.dwFlags |= STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
        }

		ZeroMemory( &si, sizeof(STARTUPINFOW));
		si.cb = sizeof(si);

		if (!CreateProcessA(NULL, lpCommandLine, NULL, NULL, TRUE, nCreationFlags, NULL, NULL, &si, &m_processInfo))
		{
			return CProcess::ERROR_CODE_SYSTEM;
		}

        return CProcess::ERROR_CODE_SUCCESS;
	}

    CProcess::ERROR_CODE CProcessImp::Create(WCHAR* lpCommandLine, bool bRun, bool bShowWindow)
    {
        if (m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_HAS_INIT;
        }

        STARTUPINFOW si;
        ZeroMemory( &m_processInfo, sizeof(PROCESS_INFORMATION));

        UINT32 nCreationFlags = 0;

        if (!bRun)
        {
            nCreationFlags = CREATE_SUSPENDED;
        }

        if (!bShowWindow)
        {
            si.dwFlags |= STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
        }

        ZeroMemory( &si, sizeof(STARTUPINFOW));
        si.cb = sizeof(si);

        if (!CreateProcessW(NULL, lpCommandLine, NULL, NULL, TRUE, nCreationFlags, NULL, NULL, &si, &m_processInfo))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        return CProcess::ERROR_CODE_SUCCESS;
    }

    CProcess::ERROR_CODE CProcessImp::Resume()
    {
        if (!m_processInfo.hProcess || !m_processInfo.hThread)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        if (((DWORD)-1) == ResumeThread(m_processInfo.hThread))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        return CProcess::ERROR_CODE_SUCCESS;
    }

	CProcess::ERROR_CODE CProcessImp::Open(UINT32 nProcessID, CProcess::OPEN_RIGHT_TYPE openRightType)
	{
		if (m_processInfo.hProcess)
		{
			return CProcess::ERROR_CODE_HAS_INIT;
		}

        CSystemInfo::SYSTEM_VER systemVer = CSystemInfo::GetSystemVersion();

        DWORD dwDesiredAccess = 0;

        switch (openRightType)
        {
        case CProcess::OPEN_RIGHT_TYPE_ALL:
            {
                dwDesiredAccess = PROCESS_ALL_ACCESS;
            }
            break;
        case CProcess::OPEN_RIGHT_TYPE_QUERY_LIMITED:
            {
                if (systemVer >= CSystemInfo::SYSTEM_VER_VISTA)
                {
                    dwDesiredAccess = PROCESS_QUERY_LIMITED_INFORMATION;
                }
                else
                {
                    dwDesiredAccess = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
                }
            }
            break;
        case CProcess::OPEN_RIGHT_TYPE_QUERY_ALL:
            {
                dwDesiredAccess = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
            }
            break;
        default:
            return CProcess::ERROR_CODE_PARAM;
        }

		HANDLE hProcess = OpenProcess(dwDesiredAccess, FALSE, nProcessID);

		if (!hProcess)
		{
			return CProcess::ERROR_CODE_NO_EXIST;
		}
       
		m_processInfo.dwProcessId = nProcessID;

		m_processInfo.hProcess = hProcess;

		m_processInfo.dwThreadId = NULL;

		m_processInfo.hThread = NULL;

        return CProcess::ERROR_CODE_SUCCESS;
	}

	CProcess::ERROR_CODE CProcessImp::OpenByName(const char* szProcessName, CProcess::OPEN_RIGHT_TYPE openRightType)
	{
        if (!szProcessName)
        {
            return CProcess::ERROR_CODE_PARAM;
        }

		if (m_processInfo.hProcess)
		{
			return CProcess::ERROR_CODE_HAS_INIT;
		}

        HANDLE hProcessSnap;
        PROCESSENTRY32 processEntry = {0};
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (hProcessSnap == INVALID_HANDLE_VALUE)
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        if (!Process32First(hProcessSnap, &processEntry))
        {
            CloseHandle(hProcessSnap);

            return CProcess::ERROR_CODE_SYSTEM;
        }

        UINT32 nProcessID = 0;

        do
        {
            if (0 == strcmp(processEntry.szExeFile, szProcessName))
            {
                nProcessID = processEntry.th32ProcessID;

                break;
            }
        }
        while(Process32Next(hProcessSnap, &processEntry));

        CloseHandle(hProcessSnap);

        if (0 == nProcessID)
        {
            return CProcess::ERROR_CODE_PROC_NOT_FIND;
        }

        return this->Open(nProcessID, openRightType);
	}

    CProcess::ERROR_CODE CProcessImp::OpenCurrent()
    {
        if (m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_HAS_INIT;
        }

        m_processInfo.dwProcessId = GetCurrentProcessId();

        m_processInfo.hProcess = GetCurrentProcess();

        m_processInfo.dwThreadId = 0;

        m_processInfo.hThread = NULL;

        return CProcess::ERROR_CODE_SUCCESS;
    }

	void CProcessImp::Close()
	{
		if (m_processInfo.hProcess)
		{
			CloseHandle(m_processInfo.hProcess);
		}

		//打开的进程获取不了主线程，所以不需要关闭，只有创建的线程可以获取主线程并且需要关闭
		if (m_processInfo.hThread)
		{
			CloseHandle(m_processInfo.hThread);
		}

		ZeroMemory(&m_processInfo, sizeof(PROCESS_INFORMATION));
	}

    CProcess::ERROR_CODE CProcessImp::Terminate(UINT32 nExitCode)
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        if (!TerminateProcess(m_processInfo.hProcess, nExitCode))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        return CProcess::ERROR_CODE_SUCCESS;
    }

    CProcess::ERROR_CODE CProcessImp::Wait(UINT32 nWaitTime)
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        if (WAIT_OBJECT_0 != WaitForSingleObject(m_processInfo.hProcess, nWaitTime))
        {
            return CProcess::ERROR_CODE_TIME_OUT;
        }

        return CProcess::ERROR_CODE_SUCCESS;
    }

    CProcess::ERROR_CODE CProcessImp::GetExitCode(UINT32* pnExitCode)
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        DWORD dwExitCode = 0;

        if (!GetExitCodeProcess(m_processInfo.hProcess, &dwExitCode))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        *pnExitCode = dwExitCode;

        return CProcess::ERROR_CODE_SUCCESS;
    }

    CProcess::ERROR_CODE CProcessImp::ReleaseMemory()
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }
        
        if (!EmptyWorkingSet(m_processInfo.hProcess))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        return CProcess::ERROR_CODE_SUCCESS;
    }

	CProcess::ERROR_CODE CProcessImp::GetProcessID(UINT32& nProcessID)
	{
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        nProcessID = m_processInfo.dwProcessId;

		return CProcess::ERROR_CODE_SUCCESS;
	}

    CProcess::ERROR_CODE CProcessImp::GetProcessNormal(TProcessNormal& aTProcessNormal)
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        HANDLE hProcessSnap;
        PROCESSENTRY32W processEntry = {0};
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (hProcessSnap == INVALID_HANDLE_VALUE)
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        if (!Process32FirstW(hProcessSnap, &processEntry))
        {
            CloseHandle(hProcessSnap);

            return CProcess::ERROR_CODE_SYSTEM;
        }

        do 
        {
            if (processEntry.th32ProcessID == m_processInfo.dwProcessId)
            {
                aTProcessNormal.nProcessID = m_processInfo.dwProcessId;
                aTProcessNormal.nParentProcessID = processEntry.th32ParentProcessID;
                aTProcessNormal.nThreadCount = processEntry.cntThreads;
                aTProcessNormal.nPriClassBase = processEntry.pcPriClassBase;
                memcpy(aTProcessNormal.szExeFile, processEntry.szExeFile, MAX_PATH);

                break;
            }
        }while(Process32NextW(hProcessSnap, &processEntry));

        CloseHandle(hProcessSnap);

        return CProcess::ERROR_CODE_SUCCESS;
    }

    CProcess::ERROR_CODE CProcessImp::GetProcessMemory(TProcessMemory& aTProcessMemory)
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        PROCESS_MEMORY_COUNTERS processMemoryCounters = {0};

        if (!GetProcessMemoryInfo(m_processInfo.hProcess, &processMemoryCounters, sizeof(processMemoryCounters)))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        aTProcessMemory.nPageFaultCount = processMemoryCounters.PageFaultCount;
        aTProcessMemory.nPagefileUsage = processMemoryCounters.PagefileUsage;
        aTProcessMemory.nPeakPagefileUsage = processMemoryCounters.PeakPagefileUsage;
        aTProcessMemory.nPeakWorkingSetSize = processMemoryCounters.PeakWorkingSetSize;
        aTProcessMemory.nQuotaNonPagedPoolUsage = processMemoryCounters.QuotaNonPagedPoolUsage;
        aTProcessMemory.nQuotaPagedPoolUsage = processMemoryCounters.QuotaPagedPoolUsage;
        aTProcessMemory.nQuotaPeakNonPagedPoolUsage = processMemoryCounters.QuotaPeakNonPagedPoolUsage;
        aTProcessMemory.nQuotaPeakPagedPoolUsage = processMemoryCounters.QuotaPeakPagedPoolUsage;
        aTProcessMemory.nWorkingSetSize = processMemoryCounters.WorkingSetSize;

        return CProcess::ERROR_CODE_SUCCESS;
    }

    CProcess::ERROR_CODE CProcessImp::GetProcessIOCounter(TProcessIOCounter& aTProcessIOCounter)
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        IO_COUNTERS IoCounters = {0};

        if (!GetProcessIoCounters(m_processInfo.hProcess, &IoCounters))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        aTProcessIOCounter.nOtherOperationCount = IoCounters.OtherOperationCount;
        aTProcessIOCounter.nOtherTransferCount = IoCounters.OtherTransferCount;
        aTProcessIOCounter.nReadOperationCount = IoCounters.ReadOperationCount;
        aTProcessIOCounter.nReadTransferCount = IoCounters.ReadTransferCount;
        aTProcessIOCounter.nWriteOperationCount = IoCounters.WriteOperationCount;
        aTProcessIOCounter.nWriteTransferCount = IoCounters.WriteTransferCount;

        return CProcess::ERROR_CODE_SUCCESS;
    }

    CProcess::ERROR_CODE CProcessImp::GetProcessResource(TProcessResource& aTProcessResource)
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        DWORD dwHandleCount = 0;

        if (!GetProcessHandleCount(m_processInfo.hProcess, &dwHandleCount))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        aTProcessResource.nHandleCount = (UINT32)dwHandleCount;

        aTProcessResource.nGDICount = GetGuiResources(m_processInfo.hProcess, GR_GDIOBJECTS);

        aTProcessResource.nUserObjectCount = GetGuiResources(m_processInfo.hProcess, GR_USEROBJECTS);
        
        return CProcess::ERROR_CODE_SUCCESS;
    }

    CProcess::ERROR_CODE CProcessImp::GetProcessAdvanced(TProcessAdvanced& aTProcessAdvanced)
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        //////////////////////////////////////////////////////////////////////////
        //进程优先级

        DWORD dwPriority = GetPriorityClass(m_processInfo.hProcess);

        switch (dwPriority)
        {
        case REALTIME_PRIORITY_CLASS:
            {
                aTProcessAdvanced.nPriority = 6;
            }
            break;
        case HIGH_PRIORITY_CLASS:
            {
                aTProcessAdvanced.nPriority = 5;
            }
            break;
        case ABOVE_NORMAL_PRIORITY_CLASS:
            {
                aTProcessAdvanced.nPriority = 4;
            }
            break;
        case NORMAL_PRIORITY_CLASS:
            {
                aTProcessAdvanced.nPriority = 3;
            }
            break;
        case BELOW_NORMAL_PRIORITY_CLASS:
            {
                aTProcessAdvanced.nPriority = 2;
            }
            break;
        case IDLE_PRIORITY_CLASS:
            {
                aTProcessAdvanced.nPriority = 1;
            }
            break;
        default:
            aTProcessAdvanced.nPriority = 0;
        }

        //////////////////////////////////////////////////////////////////////////
        //判断系统是否为64位

        if (CSystemInfo::Is64bitSystem())
        {
            BOOL bIsWow64 = FALSE;

            if (!IsWow64Process(m_processInfo.hProcess, &bIsWow64))
            {
                return CProcess::ERROR_CODE_SYSTEM;
            }

            aTProcessAdvanced.bIs64 = bIsWow64 ? false : true;
        }
        else
        {
            aTProcessAdvanced.bIs64 = false;
        }

        //////////////////////////////////////////////////////////////////////////
        //进程镜像名称

        aTProcessAdvanced.szImageFileName[0] = NULL;

        if (CSystemInfo::GetSystemVersion() >= CSystemInfo::SYSTEM_VER_VISTA)
        {
            UINT32 nSize = MAX_PATH * sizeof(WCHAR);
            if (!CProcessImpHelper::Instance().GetFullProcessImageNameW(m_processInfo.hProcess, aTProcessAdvanced.szImageFileName, &nSize))
            {
                aTProcessAdvanced.szImageFileName[0] = NULL;
            }
        }
        else
        {
            if (0 == GetModuleFileNameExW(m_processInfo.hProcess, NULL, aTProcessAdvanced.szImageFileName, MAX_PATH * sizeof(WCHAR)))
            {
                aTProcessAdvanced.szImageFileName[0] = NULL;
            }
        }

		//进程的命令行
		wstring commandLine;
		if (CProcessImpHelper::Instance().GetProcessCommandLineW(m_processInfo.hProcess, commandLine))
        {
            size_t nStrLen = COMAND_MAX_PATH>commandLine.size()?commandLine.size():COMAND_MAX_PATH;
			memcpy(aTProcessAdvanced.szCommandLine ,commandLine.c_str(), nStrLen*sizeof(WCHAR));
		}else
		{
			 aTProcessAdvanced.szCommandLine[0] = NULL;
		}

        //////////////////////////////////////////////////////////////////////////
        //进程用户名

        HANDLE hToken = NULL; 
        if(!OpenProcessToken(m_processInfo.hProcess, TOKEN_QUERY,&hToken))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        WCHAR szDomain[256] = {0};
        DWORD dwDomainSize = 256;
        DWORD dwNameSize = 41; 
        SID_NAME_USE SNU; 

        const DWORD dwMinUserNameSize = 64;
        char bufferUserName[dwMinUserNameSize] = {0};

        PTOKEN_USER pTokenUser = NULL; 
        unsigned long dwSize = 0; 

        if (GetTokenInformation(hToken, TokenUser, bufferUserName, dwMinUserNameSize, &dwSize))
        {
            pTokenUser = (PTOKEN_USER)bufferUserName;

            LookupAccountSidW(NULL, pTokenUser->User.Sid, aTProcessAdvanced.szUserName, &dwNameSize, szDomain, &dwDomainSize, &SNU);
        }
        else
        {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
            {
                CloseHandle(hToken);
                return CProcess::ERROR_CODE_SYSTEM;
            }

            pTokenUser = (PTOKEN_USER)malloc(dwSize); 

            if (!pTokenUser)
            {
                return CProcess::ERROR_CODE_SYSTEM;
            }

            if(!GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize))
            {
                free(pTokenUser);
                CloseHandle(hToken);
                return CProcess::ERROR_CODE_SYSTEM;
            }

            LookupAccountSidW(NULL, pTokenUser->User.Sid, aTProcessAdvanced.szUserName, &dwNameSize, szDomain, &dwDomainSize, &SNU);

            free(pTokenUser);
        }

        //////////////////////////////////////////////////////////////////////////
        //获取SessionID

        if (!GetTokenInformation(hToken, TokenSessionId, &aTProcessAdvanced.nSessionID, sizeof(DWORD), &dwSize))
        {
            CloseHandle(hToken);
            return CProcess::ERROR_CODE_SYSTEM;
        }
        
        CloseHandle(hToken);

        return CProcess::ERROR_CODE_SUCCESS;
    }

    CProcess::ERROR_CODE CProcessImp::GetProcessTime(TProcessTime& aTProcessTime)
    {
        if (!m_processInfo.hProcess)
        {
            return CProcess::ERROR_CODE_NOT_CREATE_OR_OPEN;
        }

        FILETIME aCreationTime = {0};
        FILETIME aExitTime = {0};
        FILETIME aKernelTime = {0};
        FILETIME aUserTime = {0};

        if (!::GetProcessTimes(m_processInfo.hProcess, &aCreationTime, &aExitTime, &aKernelTime, &aUserTime))
        {
            return CProcess::ERROR_CODE_SYSTEM;
        }

        {
            SYSTEMTIME stUTC = {0};
            SYSTEMTIME stLocal = {0};
            FileTimeToSystemTime(&aCreationTime, &stUTC);
            SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

            CDateTime aDateTime(stLocal);
            aTProcessTime.nCreateTime = aDateTime.GetTime();
        }

        {
            SYSTEMTIME stUTC = {0};
            SYSTEMTIME stLocal = {0};
            FileTimeToSystemTime(&aExitTime, &stUTC);
            SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

            CDateTime aDateTime(stLocal);
            aTProcessTime.nExitTime = aDateTime.GetTime();
        }

        {
            SYSTEMTIME stUTC = {0};
            SYSTEMTIME stLocal = {0};
            FileTimeToSystemTime(&aKernelTime, &stUTC);
            SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

            CDateTime aDateTime(stLocal);
            aTProcessTime.nKernelTime = aDateTime.GetTime();
        }

        {
            SYSTEMTIME stUTC = {0};
            SYSTEMTIME stLocal = {0};
            FileTimeToSystemTime(&aUserTime, &stUTC);
            SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

            CDateTime aDateTime(stLocal);
            aTProcessTime.nUserTime = aDateTime.GetTime();
        }

        return CProcess::ERROR_CODE_SUCCESS;
    }

    //////////////////////////////////////////////////////////////////////////
    //CProcess

    CProcess::CProcess()
    {
        m_pImp = new(std::nothrow) CProcessImp();
    }

    CProcess::~CProcess()
    {
        if (!m_pImp)
        {
            return;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        delete pImp;

        m_pImp = NULL;
    }

    CProcess::ERROR_CODE CProcess::Create(char* lpCommandLine, bool bRun, bool bShowWindow)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->Create(lpCommandLine, bRun, bShowWindow);
    }

    CProcess::ERROR_CODE CProcess::Create(WCHAR* lpCommandLine, bool bRun, bool bShowWindow)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->Create(lpCommandLine, bRun, bShowWindow);
    }

    CProcess::ERROR_CODE CProcess::Resume()
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->Resume();
    }

    CProcess::ERROR_CODE CProcess::Open(UINT32 nProcessID, OPEN_RIGHT_TYPE openRightType)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->Open(nProcessID, openRightType);
    }

    CProcess::ERROR_CODE CProcess::OpenByName(const char* szProcessName, OPEN_RIGHT_TYPE openRightType)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->OpenByName(szProcessName, openRightType);
    }

    CProcess::ERROR_CODE CProcess::OpenCurrent()
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->OpenCurrent();
    }

    void CProcess::Close()
    {
        if (!m_pImp)
        {
            return;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        pImp->Close();
    }

    CProcess::ERROR_CODE CProcess::Terminate(UINT32 nExitCode)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->Terminate(nExitCode);
    }

    CProcess::ERROR_CODE CProcess::Wait(UINT32 nWaitTime)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->Wait(nWaitTime);
    }

    CProcess::ERROR_CODE CProcess::GetExitCode(UINT32* pnExitCode)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->GetExitCode(pnExitCode);
    }

    CProcess::ERROR_CODE CProcess::ReleaseMemory()
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->ReleaseMemory();
    }

    CProcess::ERROR_CODE CProcess::GetProcessID(UINT32& nProcessID)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->GetProcessID(nProcessID);
    }

    CProcess::ERROR_CODE CProcess::GetProcessNormal(TProcessNormal& aTProcessNormal)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->GetProcessNormal(aTProcessNormal);
    }

    CProcess::ERROR_CODE CProcess::GetProcessMemory(TProcessMemory& aTProcessMemory)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->GetProcessMemory(aTProcessMemory);
    }

    CProcess::ERROR_CODE CProcess::GetProcessIOCounter(TProcessIOCounter& aTProcessIOCounter)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->GetProcessIOCounter(aTProcessIOCounter);
    }

    CProcess::ERROR_CODE CProcess::GetProcessResource(TProcessResource& aTProcessResource)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->GetProcessResource(aTProcessResource);
    }

    CProcess::ERROR_CODE CProcess::GetProcessAdvanced(TProcessAdvanced& aTProcessAdvanced)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->GetProcessAdvanced(aTProcessAdvanced);
    }

    CProcess::ERROR_CODE CProcess::GetProcessTime(TProcessTime& aTProcessTime)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CProcessImp* pImp = (CProcessImp*)m_pImp;

        return pImp->GetProcessTime(aTProcessTime);
    }

    //////////////////////////////////////////////////////////////////////////
    //静态函数

    const char* CProcess::GetCurrentCommandLine()
    {
        return GetCommandLineA();
    }

    UINT32 CProcess::GetCurProcessID()
    {
        return GetCurrentProcessId();
    }

    bool CProcess::KillProcess(UINT32 nProcessID, UINT32 nExitCode)
    {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, nProcessID);

        if (!hProcess)
        {
            return false;
        }

        bool bRes = true;

        if (!TerminateProcess(hProcess, nExitCode))
        {
            bRes = false;
        }

        CloseHandle(hProcess);
        hProcess = NULL;

        return bRes;
    }

    bool CProcess::RunProcesss(const char* szCmdLine,UINT32& nProcessID,const char* strCurrentDir, bool bShowWindow)
    {
        PROCESS_INFORMATION procInfo = {0};

        STARTUPINFOA si = {0};
        si.cb = sizeof(si);

        if (!bShowWindow)
        {
            si.dwFlags |= STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
        }

        if (!CreateProcessA(NULL, (LPSTR)szCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, strCurrentDir, &si, &procInfo))
        {
            return false;
        }

		nProcessID = procInfo.dwProcessId;
        CloseHandle(procInfo.hThread);
        CloseHandle(procInfo.hProcess);

        return true;
    }

    bool CProcess::RunProcesss(const WCHAR* szCmdLine,UINT32& nProcessID,const WCHAR* strCurrentDir, bool bShowWindow)
    {
        PROCESS_INFORMATION procInfo = {0};

        STARTUPINFOW si = {0};
        si.cb = sizeof(si);

        if (!bShowWindow)
        {
            si.dwFlags |= STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
        }

        if (!CreateProcessW(NULL, (LPWSTR)szCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, strCurrentDir, &si, &procInfo))
        {

            return false;
        }
		nProcessID = procInfo.dwProcessId;
        CloseHandle(procInfo.hThread);
        CloseHandle(procInfo.hProcess);

        return true;
    }
}