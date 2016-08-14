#include "../SysBase_Interface.h"

namespace SysBase
{
    bool CDirectory::Create(const char* szPathName, bool bRecurrence)
    {
        if (CDirectory::IsDirectory(szPathName))
        {
            return true;
        }

        if (CreateDirectoryA(szPathName, NULL))
        {
            return true;
        }
        else if (!bRecurrence)
        {
            return false;    
        }

        if (SHCreateDirectoryExA(NULL, szPathName, NULL))
        {
            return false;
        }

        return true;
    }

    bool CDirectory::Create(const WCHAR* wzPathName, bool bRecurrence)
    {
        if (CDirectory::IsDirectory(wzPathName))
        {
            return true;
        }

        if (CreateDirectoryW(wzPathName, NULL))
        {
            return true;
        }
        else if (!bRecurrence)
        {
            return false;    
        }

        if (SHCreateDirectoryExW(NULL, wzPathName, NULL))
        {
            return false;
        }

        return true;
    }

    bool CDirectory::IsDirectory(const char* szPathName)
    {
        UINT32 nAttri = GetFileAttributesA(szPathName);

        if (INVALID_FILE_ATTRIBUTES == nAttri)
        {
            return false;
        }
        else if (FILE_ATTRIBUTE_DIRECTORY & nAttri)
        {
            return true;
        }

        return false;
    }

    bool CDirectory::IsDirectory(const WCHAR* wzPathName)
    {
        UINT32 nAttri = GetFileAttributesW(wzPathName);

        if (INVALID_FILE_ATTRIBUTES == nAttri)
        {
            return false;
        }
        else if (FILE_ATTRIBUTE_DIRECTORY & nAttri)
        {
            return true;
        }

        return false;
    }

    bool CDirectory::Copy(const char* szFromPath, const char* szToPath)
    {
        if (!szFromPath || !szToPath)
        {
            return false;
        }

        if (NULL == szFromPath[0] || NULL == szToPath[0])
        {
            return false;
        }

        if (!CDirectory::IsDirectory(szFromPath))
        {
            return false;
        }

        int nFromPathLen = (int)strlen(szFromPath);
        int nToPathLen = (int)strlen(szToPath);

        if (0 == nFromPathLen || nFromPathLen >= MAX_PATH || 0 == nToPathLen || nToPathLen >= MAX_PATH)
        {
            return false;
        }

        if (nFromPathLen == nToPathLen)
        {
            if (0 == strncmp(szFromPath, szToPath, nFromPathLen))
            {
                return true;
            }
        }

        if (!CDirectory::Create(szToPath, true))
        {
            return NULL;
        }

        WIN32_FIND_DATAA findFileData = {0};
        CHAR szFindPath[MAX_PATH] = {0};
        sprintf_s(szFindPath, "%s\\*", szFromPath);

        HANDLE hFindFile = FindFirstFileA(szFindPath, &findFileData);

        if (INVALID_HANDLE_VALUE == hFindFile)
        {
            return false;
        }

        bool bFinish = true;

        do
        {
            if (strcmp(findFileData.cFileName, ".") == 0
                || strcmp(findFileData.cFileName, "..") == 0)
            {
                continue;
            }

            CHAR szSubDeskPath[MAX_PATH] = {0};  
            sprintf_s(szSubDeskPath, "%s\\%s", szToPath, findFileData.cFileName);

            CHAR szSubFromPath[MAX_PATH] = {0};
            sprintf_s(szSubFromPath, "%s\\%s", szFromPath, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                //创建目标文件夹
                CDirectory::Create(szSubDeskPath);

                //拷贝源文件夹的内容到目标文件夹
                if (!CDirectory::Copy(szSubFromPath, szSubDeskPath))
                {
                    bFinish = false;
                    break;
                }
            }
            else if (!CopyFileA(szSubFromPath, szSubDeskPath, false))
            {
                //删除文件
                bFinish = false;
                break;
            }
        }
        while(FindNextFileA(hFindFile, &findFileData));

        FindClose(hFindFile);

        return bFinish;
    }

    bool CDirectory::Copy(const WCHAR* szFromPath, const WCHAR* szToPath)
    {
        if (!szFromPath || !szToPath)
        {
            return false;
        }

        if (NULL == szFromPath[0] || NULL == szToPath[0])
        {
            return false;
        }

        if (!CDirectory::IsDirectory(szFromPath))
        {
            return false;
        }

        int nFromPathLen = (int)wcslen(szFromPath);
        int nToPathLen = (int)wcslen(szToPath);

        if (0 == nFromPathLen || nFromPathLen >= MAX_PATH || 0 == nToPathLen || nToPathLen >= MAX_PATH)
        {
            return false;
        }

        if (nFromPathLen == nToPathLen)
        {
            if (0 == wcsncmp(szFromPath, szToPath, nFromPathLen))
            {
                return true;
            }
        }

        if (!CDirectory::Create(szToPath, true))
        {
            return NULL;
        }

        WIN32_FIND_DATAW findFileData = {0};
        WCHAR szFindPath[MAX_PATH] = {0};
        swprintf_s(szFindPath, L"%s\\*", szFromPath);

        HANDLE hFindFile = FindFirstFileW(szFindPath, &findFileData);

        if (INVALID_HANDLE_VALUE == hFindFile)
        {
            return false;
        }

        bool bFinish = true;

        do
        {
            if (wcscmp(findFileData.cFileName, L".") == 0
                || wcscmp(findFileData.cFileName, L"..") == 0)
            {
                continue;
            }

            WCHAR szSubDeskPath[MAX_PATH] = {0};  
            swprintf_s(szSubDeskPath, L"%s\\%s", szToPath, findFileData.cFileName);

            WCHAR szSubFromPath[MAX_PATH] = {0};
            swprintf_s(szSubFromPath, L"%s\\%s", szFromPath, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                //创建目标文件夹
                CDirectory::Create(szSubDeskPath);

                //拷贝源文件夹的内容到目标文件夹
                if (!CDirectory::Copy(szSubFromPath, szSubDeskPath))
                {
                    bFinish = false;
                    break;
                }
            }
            else if (!CopyFileW(szSubFromPath, szSubDeskPath, false))
            {
                //删除文件
                bFinish = false;
                break;
            }
        }
        while(FindNextFileW(hFindFile, &findFileData));

        FindClose(hFindFile);

        return bFinish;
    }

    bool CDirectory::Move(const char* szFromPath, const char* szToPath)
    {
        if (!szFromPath || !szToPath)
        {
            return false;
        }

        if (NULL == szFromPath[0] || NULL == szToPath[0])
        {
            return false;
        }

        if (!CDirectory::IsDirectory(szFromPath))
        {
            return false;
        }

        int nFromPathLen = (int)strlen(szFromPath);
        int nToPathLen = (int)strlen(szToPath);

        if (0 == nFromPathLen || nFromPathLen >= MAX_PATH || 0 == nToPathLen || nToPathLen >= MAX_PATH)
        {
            return false;
        }

        if (nFromPathLen == nToPathLen)
        {
            if (0 == strncmp(szFromPath, szToPath, nFromPathLen))
            {
                return true;
            }
        }

        if (!CDirectory::Create(szToPath, true))
        {
            return NULL;
        }

        WIN32_FIND_DATAA findFileData;
        CHAR szFindPath[MAX_PATH] = {0};
        sprintf_s(szFindPath, "%s\\*", szFromPath);

        HANDLE hFindFile = FindFirstFileA(szFindPath, &findFileData);

        if (INVALID_HANDLE_VALUE == hFindFile)
        {
            return false;
        }

        bool bFinish = true;

        do
        {
            if (strcmp(findFileData.cFileName, ".") == 0
                || strcmp(findFileData.cFileName, "..") == 0)
            {
                continue;
            }

            CHAR szSubDeskPath[MAX_PATH] = {0};  
            sprintf_s(szSubDeskPath, "%s\\%s", szToPath, findFileData.cFileName);

            CHAR szSubFromPath[MAX_PATH] = {0};
            sprintf_s(szSubFromPath, "%s\\%s", szFromPath, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                //创建目标文件夹
                CDirectory::Create(szSubDeskPath);

                //拷贝源文件夹的内容到目标文件夹
                if (!CDirectory::Move(szSubFromPath, szSubDeskPath))
                {
                    bFinish = false;
                    break;
                }
            }
            else
            {
                if (!CopyFileA(szSubFromPath, szSubDeskPath, false))
                {
                    //删除文件
                    bFinish = false;
                    break;
                }
                else
                {
                    DeleteFileA(szSubFromPath);
                }
            }
        }
        while(FindNextFileA(hFindFile, &findFileData));

        FindClose(hFindFile);

        RemoveDirectoryA(szFromPath);

        return bFinish;
    }

    bool CDirectory::Move(const WCHAR* szFromPath, const WCHAR* szToPath)
    {
        if (!szFromPath || !szToPath)
        {
            return false;
        }

        if (NULL == szFromPath[0] || NULL == szToPath[0])
        {
            return false;
        }

        if (!CDirectory::IsDirectory(szFromPath))
        {
            return false;
        }

        int nFromPathLen = (int)wcslen(szFromPath);
        int nToPathLen = (int)wcslen(szToPath);

        if (0 == nFromPathLen || nFromPathLen >= MAX_PATH || 0 == nToPathLen || nToPathLen >= MAX_PATH)
        {
            return false;
        }

        if (nFromPathLen == nToPathLen)
        {
            if (0 == wcsncmp(szFromPath, szToPath, nFromPathLen))
            {
                return true;
            }
        }

        if (!CDirectory::Create(szToPath, true))
        {
            return NULL;
        }

        WIN32_FIND_DATAW findFileData;
        WCHAR szFindPath[MAX_PATH] = {0};
        swprintf_s(szFindPath, L"%s\\*", szFromPath);

        HANDLE hFindFile = FindFirstFileW(szFindPath, &findFileData);

        if (INVALID_HANDLE_VALUE == hFindFile)
        {
            return false;
        }

        bool bFinish = true;

        do
        {
            if (wcscmp(findFileData.cFileName, L".") == 0
                || wcscmp(findFileData.cFileName, L"..") == 0)
            {
                continue;
            }

            WCHAR szSubDeskPath[MAX_PATH] = {0};  
            swprintf_s(szSubDeskPath, L"%s\\%s", szToPath, findFileData.cFileName);

            WCHAR szSubFromPath[MAX_PATH] = {0};
            swprintf_s(szSubFromPath, L"%s\\%s", szFromPath, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                //创建目标文件夹
                CDirectory::Create(szSubDeskPath);

                //拷贝源文件夹的内容到目标文件夹
                if (!CDirectory::Move(szSubFromPath, szSubDeskPath))
                {
                    bFinish = false;
                    break;
                }
            }
            else
            {
                if (!CopyFileW(szSubFromPath, szSubDeskPath, false))
                {
                    //删除文件
                    bFinish = false;
                    break;
                }
                else
                {
                    DeleteFileW(szSubFromPath);
                }
            }
        }
        while(FindNextFileW(hFindFile, &findFileData));

        FindClose(hFindFile);

        RemoveDirectoryW(szFromPath);

        return bFinish;
    }

    bool CDirectory::Delete(const char* szPathName, bool bDeleteAllNode)
    {
        if (RemoveDirectoryA(szPathName))
        {
            return true;
        }

        else if (!bDeleteAllNode)
        {
            return false;
        }

        WIN32_FIND_DATAA findFileData = {0};
        CHAR szFindPath[MAX_PATH] = {0};
        sprintf_s(szFindPath, "%s\\*", szPathName);

        HANDLE hFindFile = FindFirstFileA(szFindPath, &findFileData);

        if (INVALID_HANDLE_VALUE == hFindFile)
        {
            return false;
        }

        bool bFinish = true;

        do
        {
            if (strcmp(findFileData.cFileName, ".") == 0
                || strcmp(findFileData.cFileName, "..") == 0)
            {
                continue;
            }

            CHAR szSubDeskPath[MAX_PATH] = {0};  
            sprintf_s(szSubDeskPath, "%s\\%s", szPathName, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!CDirectory::Delete(szSubDeskPath, true))
                {
                    bFinish = false;
                    break;
                }
            }
            else if (!DeleteFileA(szSubDeskPath))
            {
                //删除文件
                bFinish = false;
                break;
            }
        }
        while(FindNextFileA(hFindFile, &findFileData));

        FindClose(hFindFile);

        if (RemoveDirectoryA(szPathName))
        {
            return true;
        }

        return bFinish;
    }

    bool CDirectory::Delete(const WCHAR* szPathName, bool bDeleteAllNode)
    {
        if (RemoveDirectoryW(szPathName))
        {
            return true;
        }
        else if (!bDeleteAllNode)
        {
            return false;
        }

        WIN32_FIND_DATAW findFileData = {0};
        WCHAR szFindPath[MAX_PATH] = {0};
        swprintf_s(szFindPath, L"%s\\*", szPathName);

        HANDLE hFindFile = FindFirstFileW(szFindPath, &findFileData);

        if (INVALID_HANDLE_VALUE == hFindFile)
        {
            return false;
        }

        bool bFinish = true;

        do
        {
            if (wcscmp(findFileData.cFileName, L".") == 0
                || wcscmp(findFileData.cFileName, L"..") == 0)
            {
                continue;
            }

            WCHAR szSubDeskPath[MAX_PATH] = {0};  
            swprintf_s(szSubDeskPath, L"%s\\%s", szPathName, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!CDirectory::Delete(szSubDeskPath, true))
                {
                    bFinish = false;
                    break;
                }
            }
            else if (!DeleteFileW(szSubDeskPath))
            {
                //删除文件
                bFinish = false;
                break;
            }
        }
        while(FindNextFileW(hFindFile, &findFileData));

        FindClose(hFindFile);

        if (RemoveDirectoryW(szPathName))
        {
            return true;
        }

        return bFinish;
    }
}