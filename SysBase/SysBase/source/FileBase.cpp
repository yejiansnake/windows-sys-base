#include "../SysBase_Interface.h"

namespace SysBase
{
    bool CFileBase::IsExist(const char* szPath)
    {
        if (0 == _access(szPath, 0))
        {
            return true;
        }

        return false;
    }

    bool CFileBase::IsExist(const WCHAR* szPath)
    {
        if (0 == _waccess(szPath, 0))
        {
            return true;
        }

        return false;
    }

    bool CFileBase::Rename(const char* szOldName, const char* szNewName)
    {
        return rename(szOldName, szNewName) == 0 ? true : false; 
    }

    bool CFileBase::Rename(const WCHAR* szOldName, const WCHAR* szNewName)
    {
        return _wrename(szOldName, szNewName) == 0 ? true : false; 
    }
}