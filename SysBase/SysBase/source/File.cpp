#include "../SysBase_Interface.h"
#include <sys/stat.h>

namespace SysBase
{
    static INT64 SysBase_GetFileSize(HANDLE hFile)
    {
        LARGE_INTEGER nFileSize = {0};
        if (!::GetFileSizeEx(hFile, &nFileSize))
        {
            return 0;
        }
        
        return nFileSize.QuadPart;
    }

    //////////////////////////////////////////////////////////////////////////

    CFile::CFile()
    {
        m_pFile = NULL;

        m_bThreadSafe = false;
    }

    CFile::~CFile()
    {
        this->Close();
    }

    void CFile::Close()
    {
        if (m_pFile)
        {
            fclose(m_pFile);

            m_pFile = NULL;
        }

        m_bThreadSafe = false;
    }

    bool CFile::Create(const char* szFileName, const char* szMode, bool bThreadSafe)
    {
        if (m_pFile)
        {
            this->Close();
        }

        if (!szFileName || !szMode)
        {
            return false;
        }

        m_pFile = _fsopen(szFileName, szMode, _SH_DENYWR);

        if (!m_pFile)
        {
            return false;
        }

        m_bThreadSafe = bThreadSafe;

        return true;
    }

    bool CFile::Create(const WCHAR* wzFileName, const WCHAR* wzMode, bool bThreadSafe)
    {
        if (m_pFile)
        {
            this->Close();
        }

        if (!wzFileName || !wzMode)
        {
            return false;
        }

        m_pFile = _wfsopen(wzFileName, wzMode, _SH_DENYWR);

        if (!m_pFile)
        {
            return false;
        }

        m_bThreadSafe = bThreadSafe;

        return true;
    }

    bool CFile::IsOpen()
    {
        if (!m_pFile)
        {
            return false;
        }

        return true;
    }

    UINT32 CFile::Read(void* pBuffer, UINT32 nMaxReadBytesCount)
    {
        if (!m_pFile)
        {
            return 0;
        }

        if (m_bThreadSafe)
        {
            m_lock.Lock();
        }

        UINT32 nResValue = (UINT32)fread(pBuffer, 1, nMaxReadBytesCount, m_pFile);

        if (m_bThreadSafe)
        {
            m_lock.Unlock();  
        }

        return nResValue;
    }

    UINT32 CFile::Write(const void* pBuffer, UINT32 nMaxWriteBytesCount)
    {
        if (!m_pFile)
        {
            return 0;
        }

        if (m_bThreadSafe)
        {
            m_lock.Lock();
        }

        UINT32 nResValue = (UINT32)fwrite(pBuffer, 1, nMaxWriteBytesCount, m_pFile);

        if (m_bThreadSafe)
        {
            m_lock.Unlock();  
        }

        return nResValue;
    }

    UINT32 CFile::Read(void* pBuffer, UINT32 nMaxReadBytesCount, INT64 nSeekOffset, EFileSeek eSeekOpt)
    {
        if (!m_pFile)
        {
            return 0;
        }

        if (m_bThreadSafe)
        {
            m_lock.Lock();
        }

        int nOrigin = 0;

        switch (eSeekOpt)
        {
        case SysBase::CFile::FILE_SEEK_CUR:
            {
                nOrigin = SEEK_CUR;
            }
            break;
        case SysBase::CFile::FILE_SEEK_BEGIN:
            {
                nOrigin = SEEK_SET;
            }
            break;
        case SysBase::CFile::FILE_SEEK_END:
            {
                nOrigin = SEEK_END;
            }
            break;
        default:

            if (m_bThreadSafe)
            {
                m_lock.Unlock();  
            }

            return false;
        }

        if (0 != _fseeki64(m_pFile, nSeekOffset, nOrigin))
        {
            if (m_bThreadSafe)
            {
                m_lock.Unlock();  
            }

            return false;
        }

        UINT32 nResValue = (UINT32)fread(pBuffer, 1, nMaxReadBytesCount, m_pFile);

        if (m_bThreadSafe)
        {
            m_lock.Unlock();  
        }

        return nResValue;
    }

    UINT32 CFile::Write(const void* pBuffer, UINT32 nMaxWriteBytesCount, INT64 nSeekOffset, EFileSeek eSeekOpt)
    {
        if (!m_pFile)
        {
            return 0;
        }

        if (m_bThreadSafe)
        {
            m_lock.Lock();
        }

        int nOrigin = 0;

        switch (eSeekOpt)
        {
        case SysBase::CFile::FILE_SEEK_CUR:
            {
                nOrigin = SEEK_CUR;
            }
            break;
        case SysBase::CFile::FILE_SEEK_BEGIN:
            {
                nOrigin = SEEK_SET;
            }
            break;
        case SysBase::CFile::FILE_SEEK_END:
            {
                nOrigin = SEEK_END;
            }
            break;
        default:

            if (m_bThreadSafe)
            {
                m_lock.Unlock();  
            }

            return false;
        }

        if (0 != _fseeki64(m_pFile, nSeekOffset, nOrigin))
        {
            if (m_bThreadSafe)
            {
                m_lock.Unlock();  
            }

            return false;
        }

        UINT32 nResValue = (UINT32)fwrite(pBuffer, 1, nMaxWriteBytesCount, m_pFile);

        if (m_bThreadSafe)
        {
            m_lock.Unlock();  
        }

        return nResValue;
    }

    bool CFile::Flush()
    {
        if (!m_pFile)
        {
            return false;
        }

        if (m_bThreadSafe)
        {
            m_lock.Lock();
        }

        bool bResValue = fflush(m_pFile) == 0 ? true : false;

        if (m_bThreadSafe)
        {
            m_lock.Unlock();  
        }

        return bResValue;
    }

    bool CFile::Seek(INT64 nOffset, EFileSeek eSeekOpt)
    {
        if (!m_pFile)
        {
            return false;
        }

        if (m_bThreadSafe)
        {
            m_lock.Lock();
        }

        int nOrigin = 0;

        switch (eSeekOpt)
        {
        case SysBase::CFile::FILE_SEEK_CUR:
            {
                nOrigin = SEEK_CUR;
            }
            break;
        case SysBase::CFile::FILE_SEEK_BEGIN:
            {
                nOrigin = SEEK_SET;
            }
            break;
        case SysBase::CFile::FILE_SEEK_END:
            {
                nOrigin = SEEK_END;
            }
            break;
        default:

            if (m_bThreadSafe)
            {
                m_lock.Unlock();  
            }

            return false;
        }

        if (0 != _fseeki64(m_pFile, nOffset, nOrigin))
        {
            if (m_bThreadSafe)
            {
                m_lock.Unlock();  
            }

            return false;
        }

        if (m_bThreadSafe)
        {
            m_lock.Unlock();  
        }

        return true;
    }

    INT64 CFile::GetSize()
    {
        if (!m_pFile)
        {
            return 0;
        }

        if (m_bThreadSafe)
        {
            m_lock.Lock();
        }

        INT64 nCurPos = _ftelli64(m_pFile);

        _fseeki64(m_pFile, 0, SEEK_END);
        INT64 nResValue = _ftelli64(m_pFile);
        _fseeki64(m_pFile, nCurPos, SEEK_SET);

        if (nResValue < 0)
        {
            nResValue = 0;
        }

        if (m_bThreadSafe)
        {
            m_lock.Unlock();  
        }

        return nResValue;
    }

    //////////////////////////////////////////////////////////////////////////

    bool CFile::IsFile(const char* szFilePath)
    {
        struct __stat64 fileStat = {0};

        if (_stat64(szFilePath, &fileStat))
        {
            return false;
        }
        
        if (fileStat.st_mode & _S_IFDIR)
        {
            return false;
        }

        return true;
    }

    bool CFile::IsFile(const WCHAR* wzFilePath)
    {
        struct __stat64 fileStat = {0};

        if (_wstat64(wzFilePath, &fileStat))
        {
            return false;
        }

        if (fileStat.st_mode & _S_IFDIR)
        {
            return false;
        }

        return true;
    }

    bool CFile::Copy(const char* szFromFilePath, const char* szToFilePath)
    {
        if (CopyFileA(szFromFilePath, szToFilePath, false))
        {
            return true;
        }

        return false;
    }

    bool CFile::Copy(const WCHAR* szFromFilePath, const WCHAR* szToFilePath)
    {
        if (CopyFileW(szFromFilePath, szToFilePath, false))
        {
            return true;
        }

        return false;
    }

    bool CFile::Move(const char* szFromFilePath, const char* szToFilePath)
    {
        if (MoveFileExA(szFromFilePath, szToFilePath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
        {
            return true;
        }

        return false;
    }

    bool CFile::Move(const WCHAR* szFromFilePath, const WCHAR* szToFilePath)
    {
        if (MoveFileExW(szFromFilePath, szToFilePath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
        {
            return true;
        }

        return false;
    }

    bool CFile::Delete(const char* szFilePath)
    {
        if (0 == _unlink(szFilePath))
        {
            return true;
        }

        return false;
    }

    bool CFile::Delete(const WCHAR* szFilePath)
    {
        if (0 == _wunlink(szFilePath))
        {
            return true;
        }

        return false;
    }

    INT64 CFile::GetFileSize(const char* szFilePath)
    {
        HANDLE hFile = CreateFileA(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == hFile)
        {
            return -1;
        }

        INT64 nFileSize = SysBase_GetFileSize(hFile);

        CloseHandle(hFile);

        return nFileSize;
    }

    INT64 CFile::GetFileSize(const WCHAR* wzFilePath)
    {
        HANDLE hFile = CreateFileW(wzFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == hFile)
        {
            return -1;
        }

        INT64 nFileSize = SysBase_GetFileSize(hFile);

        CloseHandle(hFile);

        return nFileSize;
    }

    bool CFile::GetFileInfosByDirPath(const char* szDirPath, FileInfoList& fileInfoList)
    {
        if (!szDirPath)
        {
            return false;
        }

        if (!SysBase::CDirectory::IsDirectory(szDirPath))
        {
            return false;
        }

        CHAR szFindPath[MAX_PATH + 10] = {0};

        sprintf(szFindPath, "%s\\*", szDirPath);

        WIN32_FIND_DATAA findFileData;

        HANDLE hFindFile = FindFirstFileA(szFindPath, &findFileData);

        if (INVALID_HANDLE_VALUE == hFindFile)
        {
            return false;
        }

        do
        {
            TFileInfo aFileInfo = {0};

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                aFileInfo.isDirectory = true;
            }
            else
            {
                aFileInfo.isDirectory = false;
            }

            LARGE_INTEGER largeInt = {0};
            largeInt.HighPart = findFileData.nFileSizeHigh;
            largeInt.LowPart = findFileData.nFileSizeLow;

            aFileInfo.nFileSize = largeInt.QuadPart;

            if (strcmp(findFileData.cFileName, ".") == 0
                || strcmp(findFileData.cFileName, "..") == 0)
            {
                continue;
            }

            strcpy(aFileInfo.szFileName, findFileData.cFileName);

            {
                SYSTEMTIME stUTC = {0};
                SYSTEMTIME stLocal = {0};
                FileTimeToSystemTime(&findFileData.ftCreationTime, &stUTC);
                SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

                CDateTime aDateTime(stLocal);

                aFileInfo.nCreationTime = aDateTime.GetTime();
            }

            {
                SYSTEMTIME stUTC = {0};
                SYSTEMTIME stLocal = {0};
                FileTimeToSystemTime(&findFileData.ftLastAccessTime, &stUTC);
                SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

                CDateTime aDateTime(stLocal);

                aFileInfo.nLastAccessTime = aDateTime.GetTime();
            }

            {
                SYSTEMTIME stUTC = {0};
                SYSTEMTIME stLocal = {0};
                FileTimeToSystemTime(&findFileData.ftLastWriteTime, &stUTC);
                SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

                CDateTime aDateTime(stLocal);

                aFileInfo.nLastWriteTime = aDateTime.GetTime();
            }

            fileInfoList.push_back(aFileInfo);
        }
        while(FindNextFileA(hFindFile, &findFileData));

        FindClose(hFindFile);

        return true;
    }

    bool CFile::GetFileInfosByDirPath(const WCHAR* wzDirPath, FileInfoListW& fileInfoList)
    {
        if (!wzDirPath)
        {
            return false;
        }

        if (!SysBase::CDirectory::IsDirectory(wzDirPath))
        {
            return false;
        }

        WCHAR wzFindPath[MAX_PATH + 10] = {0};

        wsprintfW(wzFindPath, L"%s\\*", wzDirPath);

        WIN32_FIND_DATAW findFileData;

        HANDLE hFindFile = FindFirstFileW(wzFindPath, &findFileData);

        if (INVALID_HANDLE_VALUE == hFindFile)
        {
            return false;
        }

        do
        {
            TFileInfoW aFileInfo = {0};

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                aFileInfo.isDirectory = true;
            }
            else
            {
                aFileInfo.isDirectory = false;
            }

            LARGE_INTEGER largeInt = {0};
            largeInt.HighPart = findFileData.nFileSizeHigh;
            largeInt.LowPart = findFileData.nFileSizeLow;

            aFileInfo.nFileSize = largeInt.QuadPart;

            if (wcscmp(findFileData.cFileName, L".") == 0
                || wcscmp(findFileData.cFileName, L"..") == 0)
            {
                continue;
            }

            wcscpy(aFileInfo.wzFileName, findFileData.cFileName);

            {
                SYSTEMTIME stUTC = {0};
                SYSTEMTIME stLocal = {0};
                FileTimeToSystemTime(&findFileData.ftCreationTime, &stUTC);
                SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

                CDateTime aDateTime(stLocal);

                aFileInfo.nCreationTime = aDateTime.GetTime();
            }

            {
                SYSTEMTIME stUTC = {0};
                SYSTEMTIME stLocal = {0};
                FileTimeToSystemTime(&findFileData.ftLastAccessTime, &stUTC);
                SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

                CDateTime aDateTime(stLocal);

                aFileInfo.nLastAccessTime = aDateTime.GetTime();
            }

            {
                SYSTEMTIME stUTC = {0};
                SYSTEMTIME stLocal = {0};
                FileTimeToSystemTime(&findFileData.ftLastWriteTime, &stUTC);
                SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

                CDateTime aDateTime(stLocal);

                aFileInfo.nLastWriteTime = aDateTime.GetTime();
            }

            fileInfoList.push_back(aFileInfo);
        }
        while(FindNextFileW(hFindFile, &findFileData));

        FindClose(hFindFile);

        return true;
    }

    bool GetSaveFileName(string& strFilePath, const PCHAR szFileName, const PCHAR szTitle)
    {
        WCHAR wzFileName[MAX_PATH] = {0};
        WCHAR wzTitle[128] = {0};

        if (szFileName)
        {
            MultiByteToWideChar(CP_ACP, 0, szFileName, -1, wzFileName, MAX_PATH - 1);
        }

        if (szTitle)
        {
            MultiByteToWideChar(CP_ACP, 0, szTitle, -1, wzTitle, 127);
        }

        wstring strFilePathW;

        if (!CFile::GetSaveFilePath(
            strFilePathW, 
            szFileName == NULL ? NULL : wzFileName, 
            szTitle == NULL ? NULL : wzTitle))
        {
            return false;
        }

        CHAR szPath[MAX_PATH] = {0};

        WideCharToMultiByte(CP_ACP, 0, strFilePathW.c_str(), -1, szPath, MAX_PATH - 1, NULL, NULL);

        strFilePath = szPath;

        return true;
    }

    bool CFile::GetSaveFilePath(wstring& strFilePath, const PWCHAR wzFileName, const PWCHAR wzTitle)
    {
        OPENFILENAMEW saveFileParam = {0};
        WCHAR wzFilePath[MAX_PATH] = {0};

        if (wzFileName)
        {
            int nLen = (int)wcslen(wzFileName);
            
            if (nLen > (MAX_PATH -1))
            {
                nLen = (MAX_PATH -1);
            }

            wcsncpy(wzFilePath, wzFileName, nLen);
        }

        saveFileParam.lStructSize = sizeof(OPENFILENAMEW);
        saveFileParam.hwndOwner = GetActiveWindow();
        saveFileParam.hInstance =  (HMODULE)GetCurrentProcess();
        saveFileParam.lpstrCustomFilter = NULL;
        saveFileParam.nMaxCustFilter = 0;
        saveFileParam.nFilterIndex = 0;

        saveFileParam.lpstrFile = wzFilePath; //输出的文件名
        saveFileParam.nMaxFile = MAX_PATH;

        if (wzTitle)  
        {
            //显示在对话框窗口上的标题
            saveFileParam.lpstrTitle = wzTitle;    
        }

        saveFileParam.Flags = OFN_EXPLORER;

        if (!::GetSaveFileNameW(&saveFileParam))
        {
            return false;
        }

        strFilePath = saveFileParam.lpstrFile;

        return true;
    }

    bool CFile::GetOpenFilePath(string& strPath, const char* szTitle)
    {
        WCHAR wzTitle[128] = {0};

        if (szTitle)
        {
            MultiByteToWideChar(CP_ACP, 0, szTitle, -1, wzTitle, 127);
        }

        wstring strDirPath;

        if (!GetOpenFilePath(strDirPath, szTitle == NULL ? NULL : wzTitle))
        {
            return false;
        }

        CHAR szPath[MAX_PATH] = {0};

        WideCharToMultiByte(CP_ACP, 0, strDirPath.c_str(), -1, szPath, MAX_PATH - 1, NULL, NULL);

        strPath = szPath;

        return true;
    }

    bool CFile::GetOpenFilePath(wstring& strPath, const WCHAR* wzTitle)
    {
        WCHAR wzFilePath[MAX_PATH] = {0};

        OPENFILENAMEW openFileParam = {0};
        openFileParam.lStructSize = sizeof(OPENFILENAMEW);
        openFileParam.hwndOwner = GetActiveWindow();
        openFileParam.hInstance =  (HMODULE)GetCurrentProcess();
        openFileParam.lpstrCustomFilter = NULL;
        openFileParam.nMaxCustFilter = 0;
        openFileParam.nFilterIndex = 0;

        openFileParam.lpstrFile = wzFilePath; //输出的文件名
        openFileParam.nMaxFile = MAX_PATH;

        openFileParam.lpstrInitialDir = NULL;   //初始化的路径
        openFileParam.lpstrFileTitle = NULL;    //选中框内显示的信息
        openFileParam.nMaxFileTitle = 0;

        if (wzTitle)  
        {
            //显示在对话框窗口上的标题
            openFileParam.lpstrTitle = wzTitle;    
        }

        openFileParam.Flags = OFN_EXPLORER;
        openFileParam.nFileOffset = 0;
        openFileParam.nFileExtension = 0;
        openFileParam.lpstrDefExt = NULL;

        if (!GetOpenFileNameW(&openFileParam))
        {
            return false;
        }

        strPath = wzFilePath;

        return true;
    }

    bool CFile::CreateEmptyFile(PCCHAR szFilePath, INT64 nSize)
    {
        HANDLE hFile = CreateFileA(szFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == hFile)
        {
            return false;
        }

        LARGE_INTEGER fileSzie = {0};

        INT64 nValue = 1;

        fileSzie.QuadPart = nSize;

        HANDLE hFileMap = CreateFileMappingA(
            hFile,
            NULL,
            PAGE_READWRITE,
            fileSzie.HighPart,
            fileSzie.LowPart,
            NULL);

        if (hFileMap != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFileMap);
        }

        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }

        return true;
    }

    bool CFile::CreateEmptyFile(PCWCHAR szFilePath, INT64 nSize)
    {
        HANDLE hFile = CreateFileW(szFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == hFile)
        {
            return false;
        }

        LARGE_INTEGER fileSzie = {0};

        INT64 nValue = 1;

        fileSzie.QuadPart = nSize;

        HANDLE hFileMap = CreateFileMappingW(
            hFile,
            NULL,
            PAGE_READWRITE,
            fileSzie.HighPart,
            fileSzie.LowPart,
            NULL);

        if (hFileMap != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFileMap);
        }

        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }

        return true;
    }
}