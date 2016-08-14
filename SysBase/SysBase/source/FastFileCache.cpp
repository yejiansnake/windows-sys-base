#include "../SysBase_Interface.h"

#define FAST_FILE_CACHE_NO_END 0
#define FAST_FILE_CACHE_IS_END 1

namespace SysBase
{
    CFastFileCache::CFastFileCache()
    {
        m_pTFileHead = NULL;

        m_hFile = INVALID_HANDLE_VALUE;

        m_hFileMap = NULL;

        m_lpBuffer = NULL;

        m_bInit = FALSE;

        m_dwMaxWriteSize = 0;
    }

    CFastFileCache::~CFastFileCache()
    {
        m_bInit = FALSE;

        DeleteCriticalSection(&m_CriticalSection);

        if (m_lpBuffer)
        {
            UnmapViewOfFile(m_lpBuffer);

            m_lpBuffer = NULL;
        }

        if (m_hFileMap)
        {
            CloseHandle(m_hFileMap);
            m_hFileMap = NULL;
        }

        if (INVALID_HANDLE_VALUE != m_hFile)
        {
            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
        }

        m_pTFileHead = NULL;

        m_dwMaxWriteSize = 0; 
    }

    CFastFileCache::ERROR_CODE CFastFileCache::Init(const char* lpFilePath, const char* lpName, UINT32 nMaxWriteSize, UINT32 nMaxFileSize)
    {
        if (m_bInit)
        {
            return ERROR_CODE_NONE;
        }

        if (!lpFilePath || !lpName || 0 == nMaxFileSize || 0 == nMaxWriteSize)
        {
            return ERROR_CODE_PARAM;
        }

        int nLen = (int)strlen(lpFilePath);

        if ((MAX_PATH - 1) < nLen)
        {
            return ERROR_CODE_PARAM;
        }

        //////////////////////////////////////////////////////////////////////////

        InitializeCriticalSection(&m_CriticalSection);

        //////////////////////////////////////////////////////////////////////////

        CHAR szKeyName[64] = {0};
        int nKeyLen = (int)strlen(lpName);

        if (nKeyLen > 64)
        {
            memcpy(szKeyName, lpName, 64);
        }
        else
        {
            memcpy(szKeyName, lpName, nKeyLen);
        }

        //////////////////////////////////////////////////////////////////////////

        //判断是否有文件夹目录
        const char* pPtr = strchr(lpFilePath, '\\');

        if (pPtr)
        {
            CHAR szDirPath[MAX_PATH] = {0};

            sprintf(szDirPath, lpFilePath);

            szDirPath[MAX_PATH - 1] = NULL;

            PCHAR pEndPtr = strrchr(szDirPath, '\\');

            if (pEndPtr)
            {
                *pEndPtr = NULL;

                SHCreateDirectoryExA(NULL, szDirPath, NULL);
            }
        }

        //////////////////////////////////////////////////////////////////////////

        m_hFile = CreateFileA(lpFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == m_hFile)
        {
            return ERROR_CODE_FILE;
        }

        m_dwMaxWriteSize = nMaxWriteSize;

        DWORD dwFileSize = sizeof(TFileHead) + nMaxFileSize;

        m_hFileMap = CreateFileMappingA(m_hFile, NULL, PAGE_READWRITE, 0, dwFileSize, NULL);

        if (!m_hFileMap)
        {
            return ERROR_CODE_FILE;
        }

        m_lpBuffer = (PCHAR)MapViewOfFile(m_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        if (!m_lpBuffer)
        {
            return ERROR_CODE_FILE;
        }

        //////////////////////////////////////////////////////////////////////////

        m_pTFileHead = (TFileHead*)m_lpBuffer;

        CHAR szNewKeyName[64] = {0};

        if (0 == memcmp(m_pTFileHead->szName, szNewKeyName, 64))
        {
            //新创建的文件
            m_pTFileHead->btEnd = FAST_FILE_CACHE_NO_END;
            m_pTFileHead->nHasWriteSize = 0;
            m_pTFileHead->nSize = dwFileSize;
            m_pTFileHead->nWriteCount = 0;
            memcpy(m_pTFileHead->szName, szKeyName, 64);
        }
        else
        {
            if (0 == memcmp(m_pTFileHead->szName, szKeyName, 64))
            {
                //已经存在的文件
                if (m_pTFileHead->nSize > dwFileSize)
                {
                    //非一致的文件
                    return ERROR_CODE_FILE;
                }

                m_pTFileHead->nSize = dwFileSize;
            }
            else
            {
                //非一致的文件
                return ERROR_CODE_FILE;
            }
        }

        //////////////////////////////////////////////////////////////////////////

        FlushViewOfFile(m_lpBuffer, sizeof(TFileHead));

        PCHAR m_pBlock = (PCHAR)m_lpBuffer + sizeof(TFileHead);

        DWORD dwIndex = 0;

        DWORD dwHasCheckSize = 0;

        for (dwIndex = 0; dwIndex < m_pTFileHead->nWriteCount; ++dwIndex)
        {
            TBlockHead* pBlockHead = (TBlockHead*)(m_pBlock + dwHasCheckSize);

            if (pBlockHead->nDataSize > 0 && pBlockHead->nIndex == dwIndex)
            {
                TFastFileCacheBlock& aBlock = m_BlockMap[dwIndex];

                aBlock.nIndex = dwIndex;
                aBlock.nDataSize = pBlockHead->nDataSize;
                aBlock.pData = m_pBlock + dwHasCheckSize + sizeof(TBlockHead);

                dwHasCheckSize += sizeof(TBlockHead) + aBlock.nDataSize;
            }
            else
            {
                //文件内容错误，读取会出现问题
                ERROR_CODE_FILE;
            }
        }
        
        m_bInit = TRUE;

        return ERROR_CODE_NONE;
    }

    CFastFileCache::ERROR_CODE CFastFileCache::Add(const void* pData, UINT32 nDataSize, DWORD* pdwIndexOut)
    {
        if (!m_bInit)
        {
            return ERROR_CODE_NO_INIT;
        }

        if (!pData || nDataSize <= 0)
        {
            return ERROR_CODE_PARAM;
        }

        if (nDataSize > m_dwMaxWriteSize)
        {
            return ERROR_CODE_PARAM;
        }

        EnterCriticalSection(&m_CriticalSection);

        //判断是否无法再写入，如果无法再写入，则把该文件设置为结束标记
        if (m_pTFileHead->nHasWriteSize + nDataSize + sizeof(TBlockHead) + sizeof(TFileHead) > m_pTFileHead->nSize)
        {
            m_pTFileHead->btEnd = FAST_FILE_CACHE_IS_END;

            FlushViewOfFile(m_lpBuffer, sizeof(TFileHead));

            LeaveCriticalSection(&m_CriticalSection);

            return ERROR_CODE_FULL;
        }

        PCHAR pBlockData = (PCHAR)m_lpBuffer + sizeof(TFileHead) + m_pTFileHead->nHasWriteSize;

        DWORD dwCount = (DWORD)m_BlockMap.size();

        TBlockHead* pTBlockHead = (TBlockHead*)pBlockData;
        pTBlockHead->nIndex = dwCount;
        pTBlockHead->nDataSize = nDataSize;

        memcpy(pBlockData + sizeof(TBlockHead), pData, nDataSize);

        FlushViewOfFile(pBlockData, sizeof(TBlockHead) + nDataSize);

        TFastFileCacheBlock& aBlock = m_BlockMap[dwCount];

        aBlock.nDataSize = nDataSize;
        aBlock.nIndex = dwCount;
        aBlock.pData = pBlockData;

        if (pdwIndexOut)
        {
            *pdwIndexOut = dwCount;
        }
        
        m_pTFileHead->nHasWriteSize += sizeof(TBlockHead) + nDataSize;
        m_pTFileHead->nWriteCount++;

        FlushViewOfFile(m_lpBuffer, sizeof(TFileHead));

        LeaveCriticalSection(&m_CriticalSection);

        return ERROR_CODE_NONE;
    }

    const TFastFileCacheBlock* CFastFileCache::GetBlock(UINT32 nIndex)
    {
        if (!m_bInit)
        {
            return NULL;
        }

        TFastFileCacheBlock* pBlock = NULL;

        EnterCriticalSection(&m_CriticalSection);

        BlockMap::iterator entity = m_BlockMap.find(nIndex);

        if (entity != m_BlockMap.end())
        {
            pBlock = &entity->second;
        }

        LeaveCriticalSection(&m_CriticalSection);

        return pBlock;
    }

    DWORD CFastFileCache::GetCurBlockCount(BOOL* pbIsEnd)
    {
        if (!m_bInit)
        {
            return 0;
        }

        DWORD dwCount = 0;

        EnterCriticalSection(&m_CriticalSection);

        dwCount = (DWORD)m_BlockMap.size();

        if (pbIsEnd)
        {
            *pbIsEnd = (BOOL)m_pTFileHead->btEnd;
        }

        LeaveCriticalSection(&m_CriticalSection);

        return dwCount;
    }

    void CFastFileCache::Clear()
    {
        if (!m_bInit)
        {
            return;
        }

        EnterCriticalSection(&m_CriticalSection);

        m_BlockMap.clear();

        m_pTFileHead->nHasWriteSize = 0;

        m_pTFileHead->nWriteCount = 0;

        m_pTFileHead->btEnd = FAST_FILE_CACHE_NO_END;

        LeaveCriticalSection(&m_CriticalSection);
    }
}