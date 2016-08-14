#include "../SysBase_Interface.h"

#define SYSBASE_LOG_STRING_MAX_SIZE 3072 
#define SYSBASE_LOG_STRING_MAX_LENGTH 3071
#define SYSBASE_LOG_EXT_MAX_SIZE 20
#define SYSBASE_LOG_EXT_MAX_LENGTH 19

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CLogImp:输出到文件的日志信息类

	class CLogImp
	{
	public:

		//////////////////////////////////////////////////////////////////////////

		CLogImp();

		virtual ~CLogImp();

		CLog::ERROR_CODE Init(
			const char* pFilePath, 
			CLog::LOG_LEVEL eLevel = CLog::LOG_LEVEL_DEBUG, 
			UINT32 nMaxFileSize = SYSBASE_LOG_FILE_MAX_SIZE, 
			bool bOutputDefault = true,
            bool bAutoLine = false);

		void Write(CLog::LOG_LEVEL eLevel, const char* szInfo, UINT32 nLen);

		void SetLogLevel(CLog::LOG_LEVEL eLevel);

		CLog::LOG_LEVEL CLogImp::GetLogLevel();

		bool GetOutputDefault();

        bool GetAuthLine();

        //////////////////////////////////////////////////////////////////////////

        static UINT32 GetLogText(CLogImp* pCLogImp, CLog::LOG_LEVEL eLevel, char* szBuffer, UINT32 nMaxBufferSize, const char* szFormat, va_list args);

	private:

        bool OpenFile();

		void BackupToNewFile();

		virtual void WriteAction(const char* szInfo, UINT32 nLen);

		CThreadMutex m_CThreadMutex;

		HANDLE m_hFile;

		char m_szFileName[MAX_PATH];                    //文件名(不包含文件扩展名)

		char m_szExt[SYSBASE_LOG_EXT_MAX_SIZE];         //扩展名

		char m_szFilePath[MAX_PATH];                    //文件名

		DWORD m_nMaxFileSize;                           //单个日志文件最大大小

		DWORD m_nCurFileSize;					        //当前日志文件大小

		WORD m_wDay;                                    //当前日期

		CLog::LOG_LEVEL m_eLevel;

		bool m_bOutputDefault;

        bool m_bAutoLine;

		BOOL m_bInit;                                   //是否已经初始化
	};

    //////////////////////////////////////////////////////////////////////////
    //CLog:输出到文件的日志信息类

    CLogImp::CLogImp()
    {
        m_bInit = FALSE;

		m_hFile = INVALID_HANDLE_VALUE;

        m_nCurFileSize = 0;

        m_nMaxFileSize = 0;

        m_wDay = 0;

        ZeroMemory(m_szFileName, MAX_PATH);

        ZeroMemory(m_szExt, MAX_PATH);

        ZeroMemory(m_szFilePath, MAX_PATH);

		m_bOutputDefault = true;

        m_bAutoLine = false;
    }

    CLogImp::~CLogImp()
    {
        if (INVALID_HANDLE_VALUE != m_hFile)
		{
			CloseHandle(m_hFile);

			m_hFile = INVALID_HANDLE_VALUE;
		}
    }
    
    CLog::ERROR_CODE CLogImp::Init(
        const char* pFilePath, 
        CLog::LOG_LEVEL eLevel, 
        UINT32 nMaxFileSize,
		bool bOutputDefault,
        bool bAutoLine)
    {
        if (m_bInit)
        {
            return CLog::ERROR_CODE_HAS_INIT;
        }

        char szFilePath[MAX_PATH] = {0};

        char* ptr = NULL;

        if (!pFilePath || strlen(pFilePath) >= MAX_PATH)
        {
            return CLog::ERROR_CODE_PARAM;
        }

        if (0 == nMaxFileSize)
        {
            return CLog::ERROR_CODE_PARAM;
        }

        if ((SHORT)eLevel > 4 && (SHORT)eLevel < 0)
        {
            return CLog::ERROR_CODE_PARAM;
        }

        m_eLevel = eLevel;

        m_nMaxFileSize = nMaxFileSize;

        strncpy(szFilePath, pFilePath, MAX_PATH - 1);

        ptr = strrchr(szFilePath, '\\');

        if (!ptr)
        {
            return CLog::ERROR_CODE_PARAM;
        }

        strncpy(m_szFilePath, szFilePath, MAX_PATH - 1);

        ptr = strrchr(szFilePath, '.');

        if (ptr)
        {
           * ptr = NULL;
            strncpy(m_szExt, ptr + 1, SYSBASE_LOG_EXT_MAX_LENGTH);
        }

        strncpy(m_szFileName, szFilePath, MAX_PATH -1);

        SYSTEMTIME systemTime = {0};

        GetLocalTime(&systemTime);
        
        m_wDay = systemTime.wDay;
        
		m_bOutputDefault = bOutputDefault;

        m_bAutoLine = bAutoLine;

        m_bInit = TRUE;

		return CLog::ERROR_CODE_SUCCESS;
    }

	void CLogImp::Write(CLog::LOG_LEVEL eLevel, const char* szInfo, UINT32 nLen)
	{
		this->WriteAction(szInfo, nLen);
	}

	void CLogImp::SetLogLevel(CLog::LOG_LEVEL eLevel)
	{
		m_eLevel = eLevel;
	}

	CLog::LOG_LEVEL CLogImp::GetLogLevel()
	{
		return m_eLevel;
	}

	bool CLogImp::GetOutputDefault()
	{
		return m_bOutputDefault;
	}

    bool CLogImp::GetAuthLine()
    {
        return m_bAutoLine;
    }

    bool CLogImp::OpenFile()
    {
        if (INVALID_HANDLE_VALUE == m_hFile)
        {
            //1.1 判断目录是否存在
            char szDir[MAX_PATH] = {0};

            char* ptr = NULL;

            strncpy(szDir, m_szFilePath, MAX_PATH - 1);

            ptr = strrchr(szDir, '\\');

            if (!ptr)
            {
                return false;
            }
            else
            {
                * ptr = NULL;
            }

            SHCreateDirectoryExA(NULL, szDir, NULL);

            //1.2.加载文件
            while(1)
            {
                m_hFile = CreateFileA(m_szFilePath,
                    GENERIC_WRITE, 
                    FILE_SHARE_READ,
                    NULL, 
                    OPEN_ALWAYS, 
                    FILE_ATTRIBUTE_NORMAL, 
                    NULL);

                if (INVALID_HANDLE_VALUE == m_hFile)
                {
                    return false;
                }

                m_nCurFileSize = SetFilePointer(m_hFile, 0, NULL, FILE_END);

                //1.4 大于文件则重新生成一个文件
                if (m_nCurFileSize > (INT64)m_nMaxFileSize)
                {
                    this->BackupToNewFile();

                    return false;
                }
                else
                {
                    break;
                }
            }
        }

        return true;
    }

    void CLogImp::BackupToNewFile()
    {
        char szNewFilePath[MAX_PATH*  2] = {0};

        SYSTEMTIME systemTime = {0};

        GetLocalTime(&systemTime);

        sprintf(szNewFilePath, "%s_%d-%d-%d_%d-%d-%d.%s", 
            m_szFileName, 
            systemTime.wYear, 
            systemTime.wMonth, 
            systemTime.wDay, 
            systemTime.wHour, 
            systemTime.wMinute,
            systemTime.wSecond,
            m_szExt);

        if (INVALID_HANDLE_VALUE != m_hFile)
		{
			CloseHandle(m_hFile);

			m_hFile = INVALID_HANDLE_VALUE;
		}

        if (!MoveFileA(m_szFilePath, szNewFilePath))
        {
            DWORD dwError =GetLastError();

            DeleteFileA(szNewFilePath);

            MoveFileA(m_szFilePath, szNewFilePath);
        }

        m_wDay = systemTime.wDay;

        m_nCurFileSize = 0;
    }

    void CLogImp::WriteAction(const char* szInfo, UINT32 nLen)
    {
        if (!m_bInit)
        {
            return; 
        }

        if (!szInfo || 0 == nLen)
        {
            return;
        }
    
        CREATE_SMART_MUTEX(m_CThreadMutex);

        //////////////////////////////////////////////////////////////////////////
        //1.加载文件(无法创建与打开文件)

        if (!OpenFile())
        {
            return;
        }

        //////////////////////////////////////////////////////////////////////////
        //2.写入文件信息

		DWORD dwHasWriteBytes = 0;

		while (dwHasWriteBytes < nLen)
		{
			DWORD dwWriteBytes = 0;

			if (WriteFile(m_hFile, szInfo + dwHasWriteBytes, nLen - dwHasWriteBytes, &dwWriteBytes, NULL))
			{
				dwHasWriteBytes += dwWriteBytes;
			}
			else
			{
				break;
			}
		}

        m_nCurFileSize += nLen;

        //////////////////////////////////////////////////////////////////////////
        //3.判断写入后的文件大小

        SYSTEMTIME systemTime = {0};

        GetLocalTime(&systemTime);

        //写入后大于最大文件大小生成新文件
        if (m_nCurFileSize > m_nMaxFileSize || m_wDay != systemTime.wDay)
        {
            this->BackupToNewFile();
        }
    }

    UINT32 CLogImp::GetLogText(CLogImp* pCLogImp, CLog::LOG_LEVEL eLevel, char* szBuffer, UINT32 nMaxBufferSize, const char* szFormat, va_list args)
    {
        if (!pCLogImp)
        {
            return 0;
        }

        if (CLog::LOG_LEVEL_NONE == eLevel)
        {
            return 0;
        }

        if (!szFormat)
        {
            return 0;
        }

        if (nMaxBufferSize < 1024)
        {
            return 0;
        }

        if (eLevel > pCLogImp->GetLogLevel())
        {
            return 0;
        }

        UINT32 nLogBodyLen = 0;

        int nLogHeadLen = 0;

        if (pCLogImp->GetOutputDefault())
        {
            SYSTEMTIME systemTime = {0};

            GetLocalTime(&systemTime);

            nLogHeadLen = sprintf(szBuffer, "[%04hd-%02hd-%02hd %02hd:%02hd:%02hd:%03hd][%lu][%lu][%hu] ",
                systemTime.wYear, 
                systemTime.wMonth, 
                systemTime.wDay, 
                systemTime.wHour, 
                systemTime.wMinute,
                systemTime.wSecond,
                systemTime.wMilliseconds,
                GetTickCount(),
                GetCurrentThreadId(),
                (UINT16)eLevel);

            if (pCLogImp->GetAuthLine())
            {
                strcat(szBuffer + nLogHeadLen, "\r\n");

                nLogHeadLen += 2;
            }
        }

        nLogBodyLen = _vscprintf(szFormat, args);

        if ((nLogHeadLen + nLogBodyLen + 4) > nMaxBufferSize)
        {
            return 0;
        }

        vsprintf(szBuffer + nLogHeadLen, szFormat, args);

        int nLogEndLen = 0;

        if (pCLogImp->GetAuthLine())
        {
            strcat(szBuffer + nLogHeadLen + nLogBodyLen, "\r\n\r\n");

            nLogEndLen = 4;
        }
        else
        {
            strcat(szBuffer + nLogHeadLen + nLogBodyLen, "\r\n");

            nLogEndLen = 2;
        }

        return (nLogHeadLen + nLogBodyLen + nLogEndLen);
    }

	//////////////////////////////////////////////////////////////////////////

	CLog::CLog()
	{
		m_pImp = new(std::nothrow) CLogImp();
	}

	CLog::~CLog()
	{
		if (!m_pImp)
		{
			return;
		}

		CLogImp* pCLogImp = (CLogImp*)m_pImp;

		delete pCLogImp;

		m_pImp = NULL;
	}

	CLog::ERROR_CODE CLog::Init(
        const char* pFilePath, 
        LOG_LEVEL eLevel, 
        UINT32 nMaxFileSize, 
        bool bOutputDefault, 
        bool bAutoLine)
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CLogImp* pCLogImp = (CLogImp*)m_pImp;

		return pCLogImp->Init(pFilePath, eLevel, nMaxFileSize, bOutputDefault, bAutoLine);
	}

	void CLog::Write(LOG_LEVEL eLevel, const char* szFormat, ...)
	{
		if (!m_pImp)
		{
			return;
		}

		CLogImp* pCLogImp = (CLogImp*)m_pImp;

		if (CLog::LOG_LEVEL_NONE == eLevel)
		{
			return;
		}

		if (!szFormat)
		{
			return;
		}

		va_list args;

		char szLog[SYSBASE_LOG_STRING_MAX_SIZE] = {0};

        va_start(args, szFormat);

        UINT32 nTextLen = CLogImp::GetLogText(pCLogImp, eLevel, szLog, SYSBASE_LOG_STRING_MAX_LENGTH, szFormat, args);

        va_end(args);

		pCLogImp->Write(eLevel, szLog, nTextLen);
	}

	void CLog::SetLogLevel(LOG_LEVEL eLevel)
	{
		if (!m_pImp)
		{
			return;
		}

		CLogImp* pCLogImp = (CLogImp*)m_pImp;

		pCLogImp->SetLogLevel(eLevel);
	}

    //////////////////////////////////////////////////////////////////////////

    typedef map<int, CLogImp*> CLogMap;

    CLogMap s_CLogMap;
    CThreadMutex s_CThreadMutex;

    bool LOG_INIT(
        int nLogKey,
        const char* pFilePath, 
        CLog::LOG_LEVEL eLevel, 
        UINT32 nMaxFileSize, 
        bool bOutputDefault,
        bool bAutoLine)
    {
        
        CREATE_SMART_MUTEX(s_CThreadMutex);

        CLogMap::iterator entity = s_CLogMap.find(nLogKey);

        if (entity != s_CLogMap.end())
        {
            return true;
        }
        
        CLogImp* pCLogImp = new(std::nothrow) CLogImp();

        if (!pCLogImp)
        {
            return false;
        }

        if (CLog::ERROR_CODE_SUCCESS != pCLogImp->Init(pFilePath, eLevel, nMaxFileSize, bOutputDefault, bAutoLine))
        {
            delete pCLogImp;
            return false;
        }

        s_CLogMap.insert(CLogMap::value_type(nLogKey, pCLogImp));

        return true;
    }

    void LOG_WRITE(int nLogKey, CLog::LOG_LEVEL eLevel, const char* szFormat, ...)
    {
        CLogImp* pCLogImp = NULL;

        {
            CREATE_SMART_MUTEX(s_CThreadMutex);

            CLogMap::iterator entity = s_CLogMap.find(nLogKey);

            if (entity == s_CLogMap.end())
            {
                return;
            }

            pCLogImp = entity->second;
        }

        if (CLog::LOG_LEVEL_NONE == eLevel)
        {
            return;
        }

        if (!szFormat)
        {
            return;
        }

        if (eLevel > pCLogImp->GetLogLevel())
        {
            return;
        }

        va_list args;

        char szLog[SYSBASE_LOG_STRING_MAX_SIZE] = {0};

        va_start(args, szFormat);

        UINT32 nTextLen = CLogImp::GetLogText(pCLogImp, eLevel, szLog, SYSBASE_LOG_STRING_MAX_LENGTH, szFormat, args);

        va_end(args);

        pCLogImp->Write(eLevel, szLog, nTextLen);

    }

    void LOG_SET_LEVEL(int nLogKey, CLog::LOG_LEVEL eLevel)
    {
        CLogImp* pCLogImp = NULL;

        {
            CREATE_SMART_MUTEX(s_CThreadMutex);

            CLogMap::iterator entity = s_CLogMap.find(nLogKey);

            if (entity == s_CLogMap.end())
            {
                return;
            }

            pCLogImp = entity->second;
        }

        pCLogImp->SetLogLevel(eLevel);
    }

    //////////////////////////////////////////////////////////////////////////
}