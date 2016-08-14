#define _CRT_RAND_S		// for rand_s().
#include <stdlib.h>
#include "../SysBase_Interface.h"

typedef ULONGLONG (WINAPI *PGetTickCount64)();

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CSystemTimeImp

    class CSystemTimeImp
    {
    public:

        static CSystemTimeImp& Instance();

        CSystemTimeImp();

        virtual ~CSystemTimeImp();

        UINT64 GetTickCount();

    private:

        static CSystemTimeImp s_CSystemTimeImp;

        UINT32 m_nCurTickCount;

        UINT64 m_nCurTickCount64;

        CThreadMutex m_CThreadMutex;

        HMODULE m_hModule;

        PGetTickCount64 m_pGetTickCount64;
    };

    //////////////////////////////////////////////////////////////////////////

    CSystemTimeImp CSystemTimeImp::s_CSystemTimeImp;

    CSystemTimeImp& CSystemTimeImp::Instance()
    {
        return s_CSystemTimeImp;
    }

    CSystemTimeImp::CSystemTimeImp()
    {
        m_nCurTickCount = 0;

        m_nCurTickCount64 = 0;

        m_hModule = LoadLibraryA("Kernel32.dll");

        if (m_hModule)
        {
            m_pGetTickCount64 = (PGetTickCount64)GetProcAddress(m_hModule, "GetTickCount64");

            if (!m_pGetTickCount64)
            {
                FreeLibrary(m_hModule);

                m_hModule = NULL;
            }
        }
    }

    CSystemTimeImp::~CSystemTimeImp()
    {
        m_pGetTickCount64 = NULL;

        FreeLibrary(m_hModule);

        m_hModule = NULL;
    }

    UINT64 CSystemTimeImp::GetTickCount()
    {
        if (!m_pGetTickCount64)
        {
            CREATE_SMART_MUTEX(m_CThreadMutex);

            UINT32 nCurTickCount = ::GetTickCount();

            if (nCurTickCount < m_nCurTickCount)
            {
                m_nCurTickCount64 += UINT_MAX;
            }

            m_nCurTickCount = nCurTickCount;

            return m_nCurTickCount64 + m_nCurTickCount;
        }

        return m_pGetTickCount64();
    }

    //////////////////////////////////////////////////////////////////////////
    //CSystemTime

    CSystemTime::CSystemTime()
    {

    }

    CSystemTime::~CSystemTime()
    {

    }

    UINT64 CSystemTime::GetTickCount()
    {
        return CSystemTimeImp::Instance().GetTickCount();
    }

    INT64 CSystemTime::GetStartTime()
    {
        CDateTime aCDateTime;

        return aCDateTime.GetTime() - (INT64)(CSystemTimeImp::Instance().GetTickCount() / 1000);
    }

    INT64 CSystemTime::GetLastShutdownTime()
    {
        HKEY hKey = NULL;
        INT64 nShutdownTime = 0;
        FILETIME lastFileTime = {0};
        DWORD dwSize = sizeof(lastFileTime);

        if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Windows", 0, KEY_READ, &hKey))
        {
            return 0;
        }

        if (ERROR_SUCCESS == RegQueryValueExA(hKey, "ShutdownTime", NULL, NULL, (LPBYTE)&lastFileTime, &dwSize))
        {
            FILETIME localFileTime = {0};
            SYSTEMTIME systime = {0};

            FileTimeToLocalFileTime(&lastFileTime, &localFileTime);

            FileTimeToSystemTime(&localFileTime, &systime);

            CDateTime aCDateTime(systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);

            nShutdownTime = aCDateTime.GetTime();
        }

        RegCloseKey(hKey);

        return nShutdownTime;
    }

	//获取系统安装时间
	INT64 CSystemTime::GetInstallTime()
	{
		HKEY hKey = NULL;
		DWORD nInstallTime = 0;
	
		DWORD dwSize = sizeof(nInstallTime);
		DWORD dwType = REG_DWORD; 

		if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey))
		{
			return 0;
		}

		if (ERROR_SUCCESS == RegQueryValueExA(hKey, "InstallDate", NULL, &dwType, (LPBYTE)&nInstallTime, &dwSize))
		{

		}else
		{
			nInstallTime = 0;
		}

		RegCloseKey(hKey);

		return nInstallTime;
	}
	//获取上次补丁的安装时间
	bool CSystemTime::GetPatchInstallTime(WindowsPatchResult& result)
	{
		HKEY hKey = NULL;
		INT64 nInstallTime = 0;
		char szTime[32]={0};
		DWORD dwSize =32;
		DWORD dwType = REG_SZ; 
	
		if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\Results\\Install", 0, KEY_READ, &hKey))
		{
			return false;
		}

		if (ERROR_SUCCESS == RegQueryValueExA(hKey, "LastSuccessTime", NULL, &dwType, (LPBYTE)&szTime, &dwSize))
		{

		}else
		{
			RegCloseKey(hKey);
			return false;
		}
		DWORD LastError = 0;
		dwType = REG_DWORD;
		dwSize  = sizeof(DWORD);
		if (ERROR_SUCCESS == RegQueryValueExA(hKey, "LastError", NULL, &dwType, (LPBYTE)&LastError, &dwSize))
		{

		}else
		{
			RegCloseKey(hKey);
			return false;
		}
		
		RegCloseKey(hKey);
		int year = 0;
		int month = 0;
		int day = 0;
		int hour = 0;
		int min = 0;
		int sec = 0;
		sscanf(szTime,"%4d-%2d-%2d %2d:%2d:%2d",&year,&month,&day,&hour,&min,&sec);
		CDateTime tempTime1(year,month,day,hour,min,sec);  
		

		INT64 time = tempTime1.GetTime() + 60*60*8;
		CDateTime tempTime2(time,0); 
		nInstallTime = tempTime2.GetTime();

		result.Time = nInstallTime;
		result.Ret = LastError;


		return true;
	}

	//获取上次补丁的检测时间
	bool CSystemTime::GetPatchDectectTime(WindowsPatchResult& result)
	{
		HKEY hKey = NULL;
		INT64 nInstallTime = 0;
		char szTime[32]={0};
		DWORD dwSize =32;
		DWORD dwType = REG_SZ; 

		if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\Results\\Detect", 0, KEY_READ, &hKey))
		{
			return false;
		}

		if (ERROR_SUCCESS == RegQueryValueExA(hKey, "LastSuccessTime", NULL, &dwType, (LPBYTE)&szTime, &dwSize))
		{

		}else
		{
			RegCloseKey(hKey);
			return false;
		}

		DWORD LastError = 0;
		dwType = REG_DWORD;
		dwSize  = sizeof(DWORD);
		if (ERROR_SUCCESS == RegQueryValueExA(hKey, "LastError", NULL, &dwType, (LPBYTE)&LastError, &dwSize))
		{

		}else
		{
			RegCloseKey(hKey);
			return false;
		}

		RegCloseKey(hKey);
		int year = 0;
		int month = 0;
		int day = 0;
		int hour = 0;
		int min = 0;
		int sec = 0;
		sscanf(szTime,"%4d-%2d-%2d %2d:%2d:%2d",&year,&month,&day,&hour,&min,&sec);
		CDateTime tempTime1(year,month,day,hour,min,sec);  


		INT64 time = tempTime1.GetTime() + 60*60*8;
		CDateTime tempTime2(time,0); 
		nInstallTime = tempTime2.GetTime();

		result.Ret = LastError;
		result.Time = nInstallTime;
		return true;
	}

	//获取上次补丁的下载时间
	bool CSystemTime::GetPatchDownloadTime(WindowsPatchResult& result)
	{
		HKEY hKey = NULL;
		INT64 nInstallTime = 0;
		char szTime[32]={0};
		DWORD dwSize =32;
		DWORD dwType = REG_SZ; 

		if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\Results\\Download", 0, KEY_READ, &hKey))
		{
			return false;
		}

		if (ERROR_SUCCESS == RegQueryValueExA(hKey, "LastSuccessTime", NULL, &dwType, (LPBYTE)&szTime, &dwSize))
		{

		}else
		{
			RegCloseKey(hKey);
			return false;
		}

		DWORD LastError = 0;
		dwType = REG_DWORD;
		dwSize  = sizeof(DWORD);
		if (ERROR_SUCCESS == RegQueryValueExA(hKey, "LastError", NULL, &dwType, (LPBYTE)&LastError, &dwSize))
		{

		}else
		{
			RegCloseKey(hKey);
			return false;
		}

		RegCloseKey(hKey);
		int year = 0;
		int month = 0;
		int day = 0;
		int hour = 0;
		int min = 0;
		int sec = 0;
		sscanf(szTime,"%4d-%2d-%2d %2d:%2d:%2d",&year,&month,&day,&hour,&min,&sec);
		CDateTime tempTime1(year,month,day,hour,min,sec);  


		INT64 time = tempTime1.GetTime() + 60*60*8;
		CDateTime tempTime2(time,0); 
		nInstallTime = tempTime2.GetTime();

		result.Ret = LastError;
		result.Time = nInstallTime;

		return true;
	}


	bool CSystemTime::GetRandom(unsigned char* buf ,int iCount)
	{
			// 填充数组
		unsigned int rndNumber = 0;
		for(int i = 0; i < iCount; ++i)
		{
			rand_s(&rndNumber);
			//buf[i] = (unsigned char)(rndNumber % 0xFF);
			buf[i] = (unsigned char)((double)rndNumber / (double) UINT_MAX * 0xFF);
		}
		return true;
	}

}