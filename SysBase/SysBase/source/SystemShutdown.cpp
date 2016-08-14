#include "../SysBase_Interface.h"

namespace SysBase
{
    


    bool CSystemShutdown::ExitSystem(UINT32 nFlags)
    {
        UINT32 sourceFlags = 0;

        if (nFlags & EXIT_FLAG_LOGOFF)
        {
            sourceFlags |= EWX_LOGOFF;
        }

        if (nFlags & EXIT_FLAG_POWER_OFF)
        {
            sourceFlags |= EWX_POWEROFF;
        }

        if (nFlags & EXIT_FLAG_REBOOT)
        {
            sourceFlags |= EWX_REBOOT;
        }

        if (nFlags & EXIT_FLAG_SHUTDOWN)
        {
            sourceFlags |= EWX_SHUTDOWN;
        }

        sourceFlags |= EWX_FORCE;

        HANDLE hToken = INVALID_HANDLE_VALUE; 
        TOKEN_PRIVILEGES tokenPrivileges = {0}; 

        // Get a token for this process. 
        if (!OpenProcessToken(GetCurrentProcess(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
        {
            return false;
        }

        // Get the LUID for the shutdown privilege. 
        if (!LookupPrivilegeValueA(NULL, SE_SHUTDOWN_NAME, 
            &tokenPrivileges.Privileges[0].Luid))
        {
            return false;
        }

        tokenPrivileges.PrivilegeCount = 1;  // one privilege to set    
        tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

        // Get the shutdown privilege for this process. 
        if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, 
            (PTOKEN_PRIVILEGES)NULL, 0)) 
        {
            return false; 
        }

        return ExitWindowsEx(nFlags, SHTDN_REASON_FLAG_PLANNED) ? true : false;
    }

    bool CSystemShutdown::ShutdownSystem(PCWCHAR szMsg, UINT32 nTimeout, bool bForceAppsClosed, bool bRebootAfterShutdown)
    {
        HANDLE hToken = INVALID_HANDLE_VALUE; 
        TOKEN_PRIVILEGES tokenPrivileges = {0}; 

        // Get a token for this process. 
        if (!OpenProcessToken(GetCurrentProcess(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
        {
            return false;
        }

        // Get the LUID for the shutdown privilege. 
        if (!LookupPrivilegeValueA(NULL, SE_SHUTDOWN_NAME, 
            &tokenPrivileges.Privileges[0].Luid))
        {
            return false;
        }

        tokenPrivileges.PrivilegeCount = 1;  // one privilege to set    
        tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

        // Get the shutdown privilege for this process. 
        if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, 
            (PTOKEN_PRIVILEGES)NULL, 0)) 
        {
            return false; 
        }

        return InitiateSystemShutdownExW(NULL, (LPWSTR)szMsg, nTimeout, bForceAppsClosed, bRebootAfterShutdown, SHTDN_REASON_FLAG_PLANNED) ? true : false;
    }

    bool CSystemShutdown::CancelShutdown()
    {
        return AbortSystemShutdown(NULL) ? true : false;
    }

    bool CSystemShutdown::LockSystem()
    {
        return LockWorkStation() ? true : false;
    }
}