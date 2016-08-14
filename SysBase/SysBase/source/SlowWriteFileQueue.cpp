#include "../SysBase_Interface.h"

#define SYSBASE_SRFQ_BLOCK_VALID_USE 0
#define SYSBASE_SRFQ_BLOCK_VALID_CONFIRM 1

namespace SysBase
{

	//////////////////////////////////////////////////////////////////////////
	//CSlowWriteFileQueueImp

	class CSlowWriteFileQueueImp
	{
	public:

		//////////////////////////////////////////////////////////////////////////

		enum ERROR_CODE
		{
			ERROR_CODE_NONE = 0,
			ERROR_CODE_NO_INIT,         //未初始化
			ERROR_CODE_PARAM,           //参数错误
			ERROR_CODE_SYSTEM,          //内部资源创建失败或程序内逻辑失败
			ERROR_CODE_FILE,            //目标文件与当前初始化信息
			ERROR_CODE_FULL,            //没有空闲的数据空间
			ERROR_CODE_EMTPY,           //队列中没有任何有效数据
			ERROR_CODE_INDEX_OUT,       //索引值超出范围
			ERROR_CODE_INDEX_INVALID,   //索引值对应的数据块已经释放
		};

		//////////////////////////////////////////////////////////////////////////

		CSlowWriteFileQueueImp();

		virtual ~CSlowWriteFileQueueImp();

		CSlowWriteFileQueue::ERROR_CODE Init(const char* lpFileDir, const char* lpWirteFileName, const char* lpExtName, const char* lpName, UINT32 nBlockSize, UINT32 nMaxFileSize, UINT32 nFileLiveTime);

		CSlowWriteFileQueue::ERROR_CODE Add(const void* pData, UINT32 nDataSize);

	protected:

		//////////////////////////////////////////////////////////////////////////
		//写入部分

		bool CreateNewWriteFile();

		void CloseWriteFile();

		void CheckWriteFileLiveTime();

		void GetNewWriteFilePath(char* lpNewFilePath, const char* lpFileDir, const char* lpWirteFileName, const char* lpExtName);

		void RenameOldWriteFile(char* lpFilePath, const char* lpFileDir, const char* lpWirteFileName, const char* lpExtName);

		static DWORD WINAPI ThreadProc(LPVOID lpParam);

	private:

#pragma pack(1)

		//文件头
		struct TFileHead
		{
			//UINT32 nConfirmBlockCount;  //确认的数据块个数（如果 nConfirmBlockCount == nBlockCount 则可以删除该文件）
			//UINT32 nBlockCount;         //文件存在的有效数据块个数
			char szName[64];
			UINT32 nBlockSize;          //最大可以写入每个块的最大空间
		};

		//数据块头
		struct TBlockHead
		{
			BYTE btValid;
			UINT32 nDataSize;
		};

#pragma pack()

		//////////////////////////////////////////////////////////////////////////
		//共用部分

		bool m_bInit;

		bool m_bThreadStop;

		char m_szFileDir[MAX_PATH];

		char m_szKeyName[64];

		DWORD m_nBlockSize;            //块大小

		HANDLE m_hThread;

		//////////////////////////////////////////////////////////////////////////
		//写文件部分

		char m_szWriteFileName[MAX_PATH];

		char m_szExtName[11];

		CRITICAL_SECTION m_Write_CriticalSection;

		HANDLE m_hWriteFile;

		DWORD m_nMaxFileSize;  //文件总大小超过该值后则更换文件

		DWORD m_nCurFileSize;  //当前文件大小

		DWORD m_nFileLiveTime; //一段时间内超过该时间文件都没有被写入任何数据则更换文件

		DWORD m_nLastWriteTickCount;   //最后一次写入时间

		TFileHead m_Write_TFileHead;  //文件头        
	};

    CSlowWriteFileQueueImp::CSlowWriteFileQueueImp()
    {
        m_bInit = false;

        m_hThread = NULL;
    }

    CSlowWriteFileQueueImp::~CSlowWriteFileQueueImp()
    {
        m_bInit = false;

        m_bThreadStop = true;

        if (m_hThread)
        {
            if (WAIT_TIMEOUT == WaitForSingleObject(m_hThread, 3000))
            {
                TerminateThread(m_hThread, 0);
            }

            CloseHandle(m_hThread);

            m_hThread = NULL;
        }

        LeaveCriticalSection(&m_Write_CriticalSection);

        if (INVALID_HANDLE_VALUE != m_hWriteFile)
        {
            CloseHandle(m_hWriteFile);

            m_hWriteFile = INVALID_HANDLE_VALUE;
        }
    }

    CSlowWriteFileQueue::ERROR_CODE CSlowWriteFileQueueImp::Init(
        const char* lpFileDir, 
        const char* lpWirteFileName, 
        const char* lpExtName, 
        const char* lpName,
        UINT32 nBlockSize, 
        UINT32 nMaxFileSize, 
        UINT32 nFileLiveTime)
    {
        if (m_bInit)
        {
            return CSlowWriteFileQueue::ERROR_CODE_NONE;
        }

        if (!lpFileDir
            || !lpWirteFileName
            || !lpName
            || 0 == nBlockSize
            || 0 == nMaxFileSize
            || 0 == nFileLiveTime)
        {
            return CSlowWriteFileQueue::ERROR_CODE_PARAM;
        }

        //////////////////////////////////////////////////////////////////////////

        ZeroMemory(m_szFileDir, MAX_PATH);

        ZeroMemory(m_szWriteFileName, MAX_PATH);

        ZeroMemory(m_szExtName, 11);

        ZeroMemory(m_szKeyName, 64);

        //////////////////////////////////////////////////////////////////////////
        //Base

        //KeyName
        int nKeyLen = (int)strlen(lpName);

        if (nKeyLen > 64)
        {
            memcpy(m_szKeyName, lpName, 64);

        }
        else
        {
            memcpy(m_szKeyName, lpName, nKeyLen);
        }

        //FileDir
        int nFileDirLen = (int)strlen(lpFileDir);

        strncpy(m_szFileDir, lpFileDir, nFileDirLen > MAX_PATH ? MAX_PATH : nFileDirLen);

        m_szFileDir[MAX_PATH - 1] = NULL;

        SHCreateDirectoryExA(NULL, m_szFileDir, NULL);

        //m_nBlockSize
        m_nBlockSize = nBlockSize + sizeof(TBlockHead);

        //////////////////////////////////////////////////////////////////////////
        //Write

        //FileName
        int nWirteFileNameLen = (int)strlen(lpWirteFileName);

        strncpy(m_szWriteFileName, lpWirteFileName, nWirteFileNameLen > MAX_PATH ? MAX_PATH : nWirteFileNameLen);

        m_szWriteFileName[MAX_PATH - 1] = NULL;

        //ExtName
        int nExtNameLen = (int)strlen(lpExtName);

        strncpy(m_szExtName, lpExtName, nExtNameLen > 11 ? 11 : nExtNameLen);

        m_szExtName[11 - 1] = NULL;

        InitializeCriticalSection(&m_Write_CriticalSection);

        m_hWriteFile = INVALID_HANDLE_VALUE;

        m_nMaxFileSize = nMaxFileSize + sizeof(TFileHead);

        m_nCurFileSize = 0;
        
        m_nFileLiveTime = nFileLiveTime;

        m_nLastWriteTickCount = ULONG_MAX;

        ZeroMemory(&m_Write_TFileHead, sizeof(TFileHead));

        m_hThread = CreateThread(NULL, 0, CSlowWriteFileQueueImp::ThreadProc, this, 0, NULL);

        if (!m_hThread)
        {
            return CSlowWriteFileQueue::ERROR_CODE_SYSTEM;
        }

        m_bInit = true;

        return CSlowWriteFileQueue::ERROR_CODE_NONE;
    }

    CSlowWriteFileQueue::ERROR_CODE CSlowWriteFileQueueImp::Add(const void* pData, UINT32 nDataSize)
    {
        if (!m_bInit)
        {
            return CSlowWriteFileQueue::ERROR_CODE_NO_INIT;
        }

        if (!pData || 0 == nDataSize)
        {
            return CSlowWriteFileQueue::ERROR_CODE_PARAM;
        }

        if (nDataSize > m_nBlockSize - sizeof(TBlockHead))
        {
            return CSlowWriteFileQueue::ERROR_CODE_PARAM;
        }

        EnterCriticalSection(&m_Write_CriticalSection);

        if (INVALID_HANDLE_VALUE == m_hWriteFile)
        {
            if (!this->CreateNewWriteFile())
            {
                LeaveCriticalSection(&m_Write_CriticalSection);

                return CSlowWriteFileQueue::ERROR_CODE_FILE;
            }
        }
        else
        {
            //写入的文件大小超过最大大小
            if (m_nCurFileSize > m_nMaxFileSize)
            {
                if (!this->CreateNewWriteFile())
                {
                    LeaveCriticalSection(&m_Write_CriticalSection);

                    return CSlowWriteFileQueue::ERROR_CODE_FILE;
                }
            }
        }

        TBlockHead aTBlockHead = {0};
        aTBlockHead.btValid = SYSBASE_SRFQ_BLOCK_VALID_USE;
        aTBlockHead.nDataSize = nDataSize;

        DWORD dwWriteBytes = 0;

        if (!WriteFile(m_hWriteFile, &aTBlockHead, sizeof(TBlockHead), &dwWriteBytes, NULL))
        {
            LeaveCriticalSection(&m_Write_CriticalSection);

            return CSlowWriteFileQueue::ERROR_CODE_FILE;
        }

        if (!WriteFile(m_hWriteFile, pData, nDataSize, &dwWriteBytes, NULL))
        {
            LeaveCriticalSection(&m_Write_CriticalSection);

            return CSlowWriteFileQueue::ERROR_CODE_FILE;
        }

        //恢复文件写入指针位置
        //SetFilePointer(m_hWriteFile, 0, NULL, FILE_END);

        m_nCurFileSize += sizeof(TBlockHead) + nDataSize;

        m_nLastWriteTickCount = GetTickCount();
        
        LeaveCriticalSection(&m_Write_CriticalSection);

        return CSlowWriteFileQueue::ERROR_CODE_NONE;
    }


    //////////////////////////////////////////////////////////////////////////
    //写部分

    bool CSlowWriteFileQueueImp::CreateNewWriteFile()
    {
        this->CloseWriteFile();

        char szFilePath[MAX_PATH + MAX_PATH] = {0};

        this->GetNewWriteFilePath(szFilePath, m_szFileDir, m_szWriteFileName, m_szExtName);

        //////////////////////////////////////////////////////////////////////////
        //遇到相同名称的先重命名

        if (0 == _access(szFilePath, 0))
        {
            this->RenameOldWriteFile(szFilePath, m_szFileDir, m_szWriteFileName, m_szExtName);
        }

        //////////////////////////////////////////////////////////////////////////
        //打开写文件

        m_hWriteFile = CreateFileA(szFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == m_hWriteFile)
        {
            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //写入头信息

        ZeroMemory(&m_Write_TFileHead, sizeof(TFileHead));

        //m_Write_TFileHead.nBlockCount = 0;
        //m_Write_TFileHead.nConfirmBlockCount = 0;
        m_Write_TFileHead.nBlockSize = m_nBlockSize;


        memcpy(m_Write_TFileHead.szName, m_szKeyName, 64);

        DWORD dwWriteBytes = 0;

        if (!WriteFile(m_hWriteFile, &m_Write_TFileHead, sizeof(TFileHead), &dwWriteBytes, NULL))
        {
            return false;
        }

        m_nCurFileSize = sizeof(TFileHead);

        return true;
    }

    void CSlowWriteFileQueueImp::CloseWriteFile()
    {
        if (INVALID_HANDLE_VALUE != m_hWriteFile)
        {
            CloseHandle(m_hWriteFile);

            m_hWriteFile = INVALID_HANDLE_VALUE;

            m_nCurFileSize = 0;

            m_nLastWriteTickCount = ULONG_MAX;
        }
    }

    void CSlowWriteFileQueueImp::CheckWriteFileLiveTime()
    {
        //超过文件生存期

        UINT32 nCurTickCount = GetTickCount();

        if (m_nLastWriteTickCount > nCurTickCount)
        {
            return;
        }

        if (nCurTickCount - m_nLastWriteTickCount > m_nFileLiveTime)
        {
            EnterCriticalSection(&m_Write_CriticalSection);

            this->CloseWriteFile();
            
            LeaveCriticalSection(&m_Write_CriticalSection);
        }
    }

    void CSlowWriteFileQueueImp::GetNewWriteFilePath(char* lpNewFilePath, const char* lpFileDir, const char* lpWirteFileName, const char* lpExtName)
    {
        SYSTEMTIME systemTime = {0};
        GetLocalTime(&systemTime);

        if (lpExtName[0] != NULL)
        {
            sprintf(lpNewFilePath, "%s\\%s_%d%d%d%d%d%d%d.%s", 
                lpFileDir, lpWirteFileName, 
                systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
                systemTime.wHour, systemTime.wMinute, systemTime.wSecond, 
                GetTickCount(),
                lpExtName);
        }
        else
        {
            sprintf(lpNewFilePath, "%s\\%s_%d%d%d%d%d%d%d", 
                lpFileDir, lpWirteFileName, 
                systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
                systemTime.wHour, systemTime.wMinute, systemTime.wSecond, 
                GetTickCount());
        }
    }

    void CSlowWriteFileQueueImp::RenameOldWriteFile(char* lpFilePath, const char* lpFileDir, const char* lpWirteFileName, const char* lpExtName)
    {
        char szNewFilePath[MAX_PATH + MAX_PATH] = {0};

        SYSTEMTIME systemTime = {0};
        GetLocalTime(&systemTime);

        if (lpExtName[0] != NULL)
        {
            sprintf(szNewFilePath, "%s\\%s_O_%d%d%d%d%d%d%d.%s", 
                lpFileDir, lpWirteFileName, 
                systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
                systemTime.wHour, systemTime.wMinute, systemTime.wSecond, 
                GetTickCount(),
                lpExtName);
        }
        else
        {
            sprintf(szNewFilePath, "%s\\%s_O_%d%d%d%d%d%d%d", 
                lpFileDir, lpWirteFileName, 
                systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
                systemTime.wHour, systemTime.wMinute, systemTime.wSecond, 
                GetTickCount());
        }

        rename(lpFilePath, szNewFilePath);   
    }

    DWORD WINAPI CSlowWriteFileQueueImp::ThreadProc(LPVOID lpParam)
    {
        CSlowWriteFileQueueImp* pCSlowWriteFileQueueImp = (CSlowWriteFileQueueImp*)lpParam;

        pCSlowWriteFileQueueImp->m_bThreadStop = false;

        UINT32 nLastWriteTickCount = GetTickCount();
        UINT32 nCurTickCount = 0;

        while (!pCSlowWriteFileQueueImp->m_bThreadStop)
        {
            nCurTickCount = GetTickCount();

            if ((nCurTickCount - nLastWriteTickCount) > 1000)
            {
                pCSlowWriteFileQueueImp->CheckWriteFileLiveTime();

                nLastWriteTickCount = GetTickCount();
            }
            else
            {
                Sleep(1000);
            }
        }

        return 0;
    }

	//////////////////////////////////////////////////////////////////////////
	//CSlowWriteFileQueue

	CSlowWriteFileQueue::CSlowWriteFileQueue()
	{
		m_pImp = new(std::nothrow) CSlowWriteFileQueueImp();
	}

	CSlowWriteFileQueue::~CSlowWriteFileQueue()
	{
		if (!m_pImp)
		{
			return;
		}

		CSlowWriteFileQueueImp* pCSlowWriteFileQueueImp = (CSlowWriteFileQueueImp*)m_pImp;

		delete pCSlowWriteFileQueueImp;

		m_pImp = NULL;
	}

	CSlowWriteFileQueue::ERROR_CODE CSlowWriteFileQueue::Init(const char* lpFileDir, const char* lpWirteFileName, 
		const char* lpExtName, const char* lpName, UINT32 nBlockSize, UINT32 nMaxFileSize, UINT32 nFileLiveTime)
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CSlowWriteFileQueueImp* pCSlowWriteFileQueueImp = (CSlowWriteFileQueueImp*)m_pImp;

		return Init(lpFileDir, lpWirteFileName, lpExtName, lpName, nBlockSize, nMaxFileSize, nFileLiveTime);
	}

	CSlowWriteFileQueue::ERROR_CODE CSlowWriteFileQueue::Add(const void* pData, UINT32 nDataSize)
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CSlowWriteFileQueueImp* pCSlowWriteFileQueueImp = (CSlowWriteFileQueueImp*)m_pImp;

		return pCSlowWriteFileQueueImp->Add(pData, nDataSize);
	}
}