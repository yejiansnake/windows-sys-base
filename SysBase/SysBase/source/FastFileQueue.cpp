#include "../SysBase_Interface.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CFastFileQueueImp

	class CFastFileQueueImp
	{
	public:

		CFastFileQueueImp();

		virtual ~CFastFileQueueImp();

		CFastFileQueue::ERROR_CODE Init(const char* lpFilePath, const char* lpName, UINT32 nBlockSize, UINT32 nBlockCount);

		CFastFileQueue::ERROR_CODE Add(const void* pData, UINT32 nDataSize);

		CFastFileQueue::ERROR_CODE GetData(TFastFileBlock& aFastFileBlock, UINT32 nWaitTime);

		CFastFileQueue::ERROR_CODE FreeData(TFastFileBlock& aFastFileBlock);

		UINT32 GetQueueCount();

	private:

#pragma pack(1)

		//文件头
		struct TFileHead
		{
			char szName[64];
			UINT32 nBlockSize;
			UINT32 nBlockCount;
		};

		//数据块头
		struct TBlockHead
		{
			BYTE btValid;
			UINT32 nDataSize;
		};

#pragma pack()

		typedef map<UINT32, BYTE> WaitFreeDataIndexMap;

		WaitFreeDataIndexMap m_WaitFreeDataIndexMap;

		typedef list<UINT32> BlockList;

		PCHAR m_pBlockArray;            //数据块队列

		BlockList m_FreeBlockList;      //空闲数据块列表

		BlockList m_BlockList;          //有写入内容的数据块列表

		TFileHead* m_pTFileHead;        //文件头

		UINT32 m_nBlockSize;

		HANDLE m_hSemaphore;

		CRITICAL_SECTION m_CriticalSection;

		HANDLE m_hFile;

		HANDLE m_hFileMap;

		LPVOID m_lpBuffer;

		bool m_bInit;
	};

    CFastFileQueueImp::CFastFileQueueImp()
    {
        m_pBlockArray = NULL;

        m_pTFileHead = NULL;

        m_hSemaphore = NULL;

        m_hFile = INVALID_HANDLE_VALUE;

        m_hFileMap = NULL;

        m_lpBuffer = NULL;

        m_bInit = false;

        m_nBlockSize = 0;
    }

    CFastFileQueueImp::~CFastFileQueueImp()
    {
        m_bInit = false;

        DeleteCriticalSection(&m_CriticalSection);

        if (m_hSemaphore)
        {
            CloseHandle(m_hSemaphore);
        }

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

        m_pBlockArray = NULL;

        m_pTFileHead = NULL;

        m_nBlockSize = 0; 
    }

    CFastFileQueue::ERROR_CODE CFastFileQueueImp::Init(const char* lpFilePath, const char* lpName, UINT32 nBlockSize, UINT32 nBlockCount)
    {
        if (m_bInit)
        {
            return CFastFileQueue::ERROR_CODE_NONE;
        }

        if (!lpFilePath || !lpName || 0 == nBlockSize || 0 == nBlockCount)
        {
            return CFastFileQueue::ERROR_CODE_PARAM;
        }

        int nLen = (int)strlen(lpFilePath);

        if ((MAX_PATH - 1) < nLen)
        {
            return CFastFileQueue::ERROR_CODE_PARAM;
        }

        //////////////////////////////////////////////////////////////////////////

        m_hSemaphore = CreateSemaphoreA(NULL, 0, LONG_MAX, NULL);

        if (!m_hSemaphore)
        {
            return CFastFileQueue::ERROR_CODE_SYSTEM;
        }

        InitializeCriticalSection(&m_CriticalSection);

        //////////////////////////////////////////////////////////////////////////

        char szKeyName[64] = {0};
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
            char szDirPath[MAX_PATH] = {0};

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
            return CFastFileQueue::ERROR_CODE_FILE;
        }

        m_nBlockSize = nBlockSize + sizeof(TBlockHead);

        UINT32 nFileSize = sizeof(TFileHead) + m_nBlockSize * nBlockCount;

        m_hFileMap = CreateFileMappingA(m_hFile, NULL, PAGE_READWRITE, 0, nFileSize, NULL);

        if (!m_hFileMap)
        {
            return CFastFileQueue::ERROR_CODE_FILE;
        }

        m_lpBuffer = (PCHAR)MapViewOfFile(m_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        if (!m_lpBuffer)
        {
            return CFastFileQueue::ERROR_CODE_FILE;
        }

        //////////////////////////////////////////////////////////////////////////

        m_pTFileHead = (TFileHead*)m_lpBuffer;

        char szNewKeyName[64] = {0};

        if (0 == memcmp(m_pTFileHead->szName, szNewKeyName, 64))
        {
            //新创建的文件
            m_pTFileHead->nBlockCount = nBlockCount;
            m_pTFileHead->nBlockSize = nBlockSize;
            memcpy(m_pTFileHead->szName, szKeyName, 64);
        }
        else
        {
            if (0 == memcmp(m_pTFileHead->szName, szKeyName, 64))
            {
                //已经存在的文件
                m_pTFileHead->nBlockCount = nBlockCount;
                m_pTFileHead->nBlockSize = nBlockSize;
            }
            else
            {
                //非一致的文件
                return CFastFileQueue::ERROR_CODE_FILE;
            }
        }

        //////////////////////////////////////////////////////////////////////////

        FlushViewOfFile(m_lpBuffer, sizeof(TFileHead));

        m_pBlockArray = (PCHAR)m_lpBuffer + sizeof(TFileHead);

        UINT32 nIndex = 0;

        for (nIndex = 0; nIndex < nBlockCount; nIndex++)
        {
            TBlockHead* pTBlockHead = (TBlockHead*)(m_pBlockArray + m_nBlockSize * nIndex);

            if (pTBlockHead->btValid)
            {
                m_BlockList.push_back(nIndex);

                ReleaseSemaphore(m_hSemaphore, 1, NULL);
            }
            else
            {
                //pTBlockHead->btValid = 0;
                //pTBlockHead->nDataSize = 0;

                //FlushViewOfFile(pTBlockHead, sizeof(TBlockHead));

                m_FreeBlockList.push_back(nIndex);
            }
        }
        
        m_bInit = true;

        return CFastFileQueue::ERROR_CODE_NONE;
    }

    CFastFileQueue::ERROR_CODE CFastFileQueueImp::Add(const void* pData, UINT32 nDataSize)
    {
        if (!m_bInit)
        {
            return CFastFileQueue::ERROR_CODE_NO_INIT;
        }

        if (!pData || nDataSize <= 0)
        {
            return CFastFileQueue::ERROR_CODE_PARAM;
        }

        if (nDataSize > m_nBlockSize - sizeof(TBlockHead))
        {
            return CFastFileQueue::ERROR_CODE_PARAM;
        }

        EnterCriticalSection(&m_CriticalSection);

        if (m_FreeBlockList.size() == 0)
        {
            LeaveCriticalSection(&m_CriticalSection);

            return CFastFileQueue::ERROR_CODE_FULL;
        }

        UINT32 nBlockIndex = m_FreeBlockList.front();

        m_FreeBlockList.pop_front();

        PCHAR pBlockData = m_pBlockArray + m_nBlockSize * nBlockIndex;

        TBlockHead* pTBlockHead = (TBlockHead*)pBlockData;
        pTBlockHead->btValid = 1;
        pTBlockHead->nDataSize = nDataSize;

        memcpy(pBlockData + sizeof(TBlockHead), pData, nDataSize);

        FlushViewOfFile(pBlockData, sizeof(TBlockHead) + nDataSize);
    
        m_BlockList.push_back(nBlockIndex);

        LeaveCriticalSection(&m_CriticalSection);

        ReleaseSemaphore(m_hSemaphore, 1, NULL);

        return CFastFileQueue::ERROR_CODE_NONE;
    }

    CFastFileQueue::ERROR_CODE CFastFileQueueImp::GetData(TFastFileBlock& aFastFileBlock, UINT32 nWaitTime)
    {
        if (!m_bInit)
        {
            return CFastFileQueue::ERROR_CODE_NO_INIT;
        }

        bool bHasData = false;

        switch (WaitForSingleObject(m_hSemaphore, nWaitTime))
        {
        case WAIT_TIMEOUT:
            {
                return CFastFileQueue::ERROR_CODE_EMTPY;
            }
            break;
        case WAIT_OBJECT_0:
            {
                bHasData = true;
            }
            break;
        default:
            {
                return CFastFileQueue::ERROR_CODE_SYSTEM;
            }
        }

        if (!bHasData)
        {
            return CFastFileQueue::ERROR_CODE_EMTPY;
        }

        EnterCriticalSection(&m_CriticalSection);

        if (m_BlockList.size() == 0)
        {
            LeaveCriticalSection(&m_CriticalSection);

            return CFastFileQueue::ERROR_CODE_EMTPY;
        }

        UINT32 nBlockIndex = m_BlockList.front();

        m_BlockList.pop_front();

        PCHAR pBlockData = m_pBlockArray + m_nBlockSize * nBlockIndex;

        TBlockHead* pTBlockHead = (TBlockHead*)pBlockData;

        if (pTBlockHead->btValid
            && pTBlockHead->nDataSize > 0)
        {
            aFastFileBlock.pData = pBlockData + sizeof(TBlockHead);

            aFastFileBlock.nDataSize = pTBlockHead->nDataSize;

            aFastFileBlock.nIndex = nBlockIndex;

            m_WaitFreeDataIndexMap.insert(WaitFreeDataIndexMap::value_type(nBlockIndex, 0));
        }
        else
        {
            pTBlockHead->btValid = 0;
            pTBlockHead->nDataSize = 0;

            FlushViewOfFile(pBlockData, sizeof(TBlockHead));

            m_FreeBlockList.push_back(nBlockIndex);

            m_WaitFreeDataIndexMap.erase(nBlockIndex);

            LeaveCriticalSection(&m_CriticalSection);

            return CFastFileQueue::ERROR_CODE_SYSTEM;
        }

        LeaveCriticalSection(&m_CriticalSection);
        
        return CFastFileQueue::ERROR_CODE_NONE;
    }

    CFastFileQueue::ERROR_CODE CFastFileQueueImp::FreeData(TFastFileBlock& aFastFileBlock)
    {
        if (!m_bInit)
        {
            return CFastFileQueue::ERROR_CODE_NO_INIT;
        }

        if (aFastFileBlock.nIndex >= m_pTFileHead->nBlockCount)
        {
            return CFastFileQueue::ERROR_CODE_INDEX_OUT;
        }

        EnterCriticalSection(&m_CriticalSection);

        if (m_WaitFreeDataIndexMap.end() == m_WaitFreeDataIndexMap.find(aFastFileBlock.nIndex))
        {
            LeaveCriticalSection(&m_CriticalSection);

            return CFastFileQueue::ERROR_CODE_INDEX_INVALID;
        }

        PCHAR pBlockData = m_pBlockArray + m_nBlockSize * aFastFileBlock.nIndex;

        TBlockHead* pTBlockHead = (TBlockHead*)pBlockData;

        if (pTBlockHead->btValid)
        {
            pTBlockHead->btValid = 0;
            pTBlockHead->nDataSize = 0;

            FlushViewOfFile(pBlockData, sizeof(TBlockHead));
        }

        m_FreeBlockList.push_back(aFastFileBlock.nIndex);

        m_WaitFreeDataIndexMap.erase(aFastFileBlock.nIndex);

        LeaveCriticalSection(&m_CriticalSection);

        return CFastFileQueue::ERROR_CODE_NONE;
    }

    UINT32 CFastFileQueueImp::GetQueueCount()
    {
        if (!m_bInit)
        {
            return 0;
        }
        
        UINT32 nQueueCount = 0;

        EnterCriticalSection(&m_CriticalSection);

        nQueueCount = (DWORD)m_BlockList.size();

        LeaveCriticalSection(&m_CriticalSection);

        return nQueueCount;
    }

	//////////////////////////////////////////////////////////////////////////
	//

	CFastFileQueue::CFastFileQueue()
	{
		m_pImp = new(std::nothrow) CFastFileQueueImp();
	}

	CFastFileQueue::~CFastFileQueue()
	{
		if (!m_pImp)
		{
			return;
		}

		CFastFileQueueImp* pCFastFileQueueImp = (CFastFileQueueImp*)m_pImp;

		delete pCFastFileQueueImp;

		m_pImp = NULL;
	}

	CFastFileQueue::ERROR_CODE CFastFileQueue::Init(const char* lpFilePath, const char* lpName, UINT32 nBlockSize, UINT32 nBlockCount)
	{
		if (!m_pImp)
		{
			return CFastFileQueue::ERROR_CODE_SYSTEM;
		}

		CFastFileQueueImp* pCFastFileQueueImp = (CFastFileQueueImp*)m_pImp;

		return pCFastFileQueueImp->Init(lpFilePath, lpName, nBlockSize, nBlockCount);
	}

	CFastFileQueue::ERROR_CODE CFastFileQueue::Add(const void* pData, UINT32 nDataSize)
	{
		if (!m_pImp)
		{
			return CFastFileQueue::ERROR_CODE_SYSTEM;
		}

		CFastFileQueueImp* pCFastFileQueueImp = (CFastFileQueueImp*)m_pImp;

		return pCFastFileQueueImp->Add(pData, nDataSize);
	}

	CFastFileQueue::ERROR_CODE CFastFileQueue::GetData(TFastFileBlock& aFastFileBlock, UINT32 nWaitTime)
	{
		if (!m_pImp)
		{
			return CFastFileQueue::ERROR_CODE_SYSTEM;
		}

		CFastFileQueueImp* pCFastFileQueueImp = (CFastFileQueueImp*)m_pImp;

		return pCFastFileQueueImp->GetData(aFastFileBlock, nWaitTime);
	}

	CFastFileQueue::ERROR_CODE CFastFileQueue::FreeData(TFastFileBlock& aFastFileBlock)
	{
		if (!m_pImp)
		{
			return CFastFileQueue::ERROR_CODE_SYSTEM;
		}

		CFastFileQueueImp* pCFastFileQueueImp = (CFastFileQueueImp*)m_pImp;

		return pCFastFileQueueImp->FreeData(aFastFileBlock);
	}

	UINT32 CFastFileQueue::GetQueueCount()
	{
		if (!m_pImp)
		{
			return 0;
		}

		CFastFileQueueImp* pCFastFileQueueImp = (CFastFileQueueImp*)m_pImp;

		return pCFastFileQueueImp->GetQueueCount();
	}
}

