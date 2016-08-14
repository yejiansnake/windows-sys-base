#include "../SysBase_Interface.h"

namespace SysBase
{

	//////////////////////////////////////////////////////////////////////////
	//StringBuilderA

	struct VarStringA
	{
        VarStringA()
        {
            szStr = NULL;
            Next = NULL;
            nLength = 0;
        }

        ~VarStringA()
        {
            _DELETE(szStr);
        }

		char* szStr;
		DWORD nLength;
		VarStringA* Next;
	};

	CStringBuilderA::CStringBuilderA(void)
	{
		m_vsHead = NULL;
		m_vsEnd = NULL;
		m_nLength = 0;
	}

	CStringBuilderA::CStringBuilderA(const char* szSource)
	{
		this->Build(szSource);
	}	
	CStringBuilderA::CStringBuilderA(const CStringBuilderA& strBuilder)
	{
		m_vsHead = NULL;
		m_vsEnd = NULL;
		m_nLength = 0;

		VarStringA* pTemp = NULL;
		VarStringA* sourceVsHead = (VarStringA*)strBuilder.m_vsHead;

		//对表头进行赋值
		if (sourceVsHead != NULL)
		{
			m_vsHead = new(std::nothrow) VarStringA;
			
			if (!m_vsHead)
			{
				return;
			}

			VarStringA* vsHead = (VarStringA*)m_vsHead;

			vsHead->Next = NULL;

            vsHead->nLength = (DWORD)strlen(sourceVsHead->szStr);

			vsHead->szStr = new(std::nothrow) char[(size_t)(vsHead->nLength + 1)];
			
			if (!vsHead->szStr)
			{
				return;
			}

            strcpy(vsHead->szStr, sourceVsHead->szStr);

			m_nLength += sourceVsHead->nLength; 

			m_vsEnd = m_vsHead;
		}
		else
		{
			return;
		}
		
		VarStringA* vsEnd = (VarStringA*)m_vsEnd;

		for (VarStringA* pVS = sourceVsHead->Next; pVS != NULL; pVS = pVS->Next)
		{
			pTemp = new(std::nothrow) VarStringA;
			
			if (!pTemp)
			{
				return;
			}

			pTemp->Next = NULL;
			
			pTemp->nLength = (DWORD)strlen(pVS->szStr);

			pTemp->szStr = new(std::nothrow) char[(size_t)(pTemp->nLength + 1)];
			
			if (!pTemp)
			{
				return;
			}

			strcpy(pTemp->szStr, pVS->szStr);
			
			m_nLength += pTemp->nLength; 

			vsEnd->Next = pTemp;
			
			vsEnd = vsEnd->Next;
			
			pTemp = NULL;
		}
	}

	void CStringBuilderA::Append(const char* szSource)
	{
		
		//检测-----------------------------------
		if (szSource == NULL || strlen(szSource) == 0)
		{
			return;
		}
	 
		//实现-----------------------------------

		VarStringA* pTemp = new(std::nothrow) VarStringA;
		
		if (!pTemp)
		{
			return;
		}

		pTemp->Next = NULL;

		pTemp->nLength = (DWORD)strlen(szSource);

		pTemp->szStr = new(std::nothrow) char[(size_t)(pTemp->nLength + 1)];
		
		if (!pTemp)
		{
			return;
		}

		strcpy(pTemp->szStr, szSource);
		
		m_nLength += pTemp->nLength;

		if (m_vsHead == NULL)
		{
			m_vsHead = pTemp;

			m_vsEnd = pTemp;
		}
		else
		{
			VarStringA* vsEnd = (VarStringA*)m_vsEnd;

			vsEnd->Next = pTemp;

			m_vsEnd = vsEnd->Next;
		}

		pTemp = NULL;
	}

	CStringBuilderA::~CStringBuilderA(void)
	{
		this->FreeStr();
	}

	char*  CStringBuilderA::GetString(char* szDesk)
	{
        if (!szDesk)
        {
            return NULL;
        }

		char* szPDesk = szDesk;
		char* szTempSource = NULL;

		for (VarStringA* pVS = (VarStringA*)m_vsHead; pVS != NULL; pVS = pVS->Next)
		{
			szTempSource = pVS->szStr;

			while (*szTempSource != '\0')
			{
				*szPDesk =* szTempSource;
				
				++szPDesk;
				++szTempSource;
			}
		}

		*(szPDesk) = '\0';

		return szDesk;
	}

	INT64 CStringBuilderA::GetStrLength()
	{
		return m_nLength;
	}	
	void CStringBuilderA::Clear()
	{
		this->FreeStr();
	}

	void CStringBuilderA::FreeStr()
	{

		m_nLength = 0;

		VarStringA* vsHead = (VarStringA*)m_vsHead;

		for (VarStringA* pVS = vsHead; vsHead != NULL; pVS = vsHead)
		{
			vsHead = vsHead->Next;
	
			//释放结构体
            _DELETE(pVS);
		}

		m_vsHead = NULL;

		m_vsEnd = NULL;
	}

	void CStringBuilderA::Build(const char* szSource)
	{
		m_nLength = 0;

		m_vsHead = new(std::nothrow) VarStringA;
		
		if (!m_vsHead)
		{
			return;
		}

		VarStringA* vsHead = (VarStringA*)m_vsHead;

		vsHead->nLength = (DWORD)strlen(szSource);
		
		vsHead->szStr = new(std::nothrow) char[(size_t)(vsHead->nLength + 1)];
		
		if (!m_vsHead)
		{
			return;
		}
		
		strcpy(vsHead->szStr, szSource);

		m_nLength += vsHead->nLength;

		vsHead->Next = NULL;

		m_vsEnd = m_vsHead;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//CStringBuilderW
	
	struct VarStringW
	{
		VarStringW()
		{
			szStr = NULL;
			Next = NULL;
			nLength = 0;
		}

		~VarStringW()
		{
			_DELETE(szStr);
		}

		WCHAR* szStr;
		DWORD nLength;
		VarStringW* Next;
	};

	CStringBuilderW::CStringBuilderW(void)
	{
		m_vsHead = NULL;
		m_vsEnd = NULL;
		m_nLength = 0;
	}

	CStringBuilderW::CStringBuilderW(const WCHAR* szSource)
	{
		this->Build(szSource);
	}	
	CStringBuilderW::CStringBuilderW(const CStringBuilderW& strBuilder)
	{
		m_vsHead = NULL;
		m_vsEnd = NULL;
		m_nLength = 0;

		VarStringW* pTemp = NULL;
		VarStringW* sourceVsHead = (VarStringW*)strBuilder.m_vsHead;

		//对表头进行赋值
		if (sourceVsHead != NULL)
		{
			m_vsHead = new(std::nothrow) VarStringW;

			if (!m_vsHead)
			{
				return;
			}

			VarStringW* vsHead = (VarStringW*)m_vsHead;

			vsHead->Next = NULL;

			vsHead->nLength = (DWORD)wcslen(sourceVsHead->szStr);

			vsHead->szStr = new(std::nothrow) WCHAR[(size_t)(vsHead->nLength + 1)];

			if (!vsHead->szStr)
			{
				return;
			}

			wcscpy(vsHead->szStr, sourceVsHead->szStr);

			m_nLength += sourceVsHead->nLength; 

			m_vsEnd = m_vsHead;
		}
		else
		{
			return;
		}

		VarStringW* vsEnd = (VarStringW*)m_vsEnd;

		for (VarStringW* pVS = sourceVsHead->Next; pVS != NULL; pVS = pVS->Next)
		{
			pTemp = new(std::nothrow) VarStringW;

			if (!pTemp)
			{
				return;
			}

			pTemp->Next = NULL;

			pTemp->nLength = (DWORD)wcslen(pVS->szStr);

			pTemp->szStr = new(std::nothrow) WCHAR[(size_t)(pTemp->nLength + 1)];

			if (!pTemp)
			{
				return;
			}

			wcscpy(pTemp->szStr, pVS->szStr);

			m_nLength += pTemp->nLength; 

			vsEnd->Next = pTemp;

			vsEnd = vsEnd->Next;

			pTemp = NULL;
		}
	}

	void CStringBuilderW::Append(const WCHAR* szSource)
	{

		//检测-----------------------------------
		if (szSource == NULL || wcslen(szSource) == 0)
		{
			return;
		}

		//实现-----------------------------------

		VarStringW* pTemp = new(std::nothrow) VarStringW;

		if (!pTemp)
		{
			return;
		}

		pTemp->Next = NULL;

		pTemp->nLength = (DWORD)wcslen(szSource);

		pTemp->szStr = new(std::nothrow) WCHAR[(size_t)(pTemp->nLength + 1)];

		if (!pTemp)
		{
			return;
		}

		wcscpy(pTemp->szStr, szSource);

		m_nLength += pTemp->nLength;

		if (m_vsHead == NULL)
		{
			m_vsHead = pTemp;

			m_vsEnd = pTemp;
		}
		else
		{
			VarStringW* vsEnd = (VarStringW*)m_vsEnd;

			vsEnd->Next = pTemp;

			m_vsEnd = vsEnd->Next;
		}

		pTemp = NULL;
	}

	CStringBuilderW::~CStringBuilderW(void)
	{
		this->FreeStr();
	}

	WCHAR*  CStringBuilderW::GetString(WCHAR* szDesk)
	{
		if (!szDesk)
		{
			return NULL;
		}

		WCHAR* szPDesk = szDesk;
		WCHAR* szTempSource = NULL;

		for (VarStringW* pVS = (VarStringW*)m_vsHead; pVS != NULL; pVS = pVS->Next)
		{
			szTempSource = pVS->szStr;

			while (*szTempSource != '\0')
			{
				*szPDesk =* szTempSource;

				++szPDesk;
				++szTempSource;
			}
		}

		*(szPDesk) = '\0';

		return szDesk;
	}

	INT64 CStringBuilderW::GetStrLength()
	{
		return m_nLength;
	}	
	void CStringBuilderW::Clear()
	{
		this->FreeStr();
	}

	void CStringBuilderW::FreeStr()
	{

		m_nLength = 0;

		VarStringA* vsHead = (VarStringA*)m_vsHead;

		for (VarStringA* pVS = vsHead; vsHead != NULL; pVS = vsHead)
		{
			vsHead = vsHead->Next;

			//释放结构体
			_DELETE(pVS);
		}

		m_vsHead = NULL;

		m_vsEnd = NULL;
	}

	void CStringBuilderW::Build(const WCHAR* szSource)
	{
		m_nLength = 0;

		m_vsHead = new(std::nothrow) VarStringW;

		if (!m_vsHead)
		{
			return;
		}

		VarStringW* vsHead = (VarStringW*)m_vsHead;

		vsHead->nLength = (DWORD)wcslen(szSource);

		vsHead->szStr = new(std::nothrow) WCHAR[(size_t)(vsHead->nLength + 1)];

		if (!m_vsHead)
		{
			return;
		}

		wcscpy(vsHead->szStr, szSource);

		m_nLength += vsHead->nLength;

		vsHead->Next = NULL;

		m_vsEnd = m_vsHead;
	}

    //////////////////////////////////////////////////////////////////////////
    //Convert

    UINT64 Convert::ToUINT64(UINT32 nLow, UINT32 nHigh)
    {
        UINT64 nSize = nHigh;

        nSize = nSize << 32;

        nSize+= nLow;

        return nSize;
    }

    DWORD Convert::ToDWORD(UINT64 nInt64, DWORD* pDwHigh)
    {
        if (pDwHigh)
        {
           * pDwHigh = (DWORD)(nInt64 >> 32);
        }

        return (DWORD)(nInt64 & 0xFFFFFFFF);
    }

    void Convert::LocalToUTF8(string strTextLocal, string& strTextUtf8)
    {
        strTextUtf8 = "";

        if (0 == strTextLocal.length())
        {
            return;
        }

        vector<WCHAR> strUnicode;

        int nSize = MultiByteToWideChar(CP_ACP, 0, strTextLocal.c_str(), -1, NULL, 0);

        strUnicode.resize(nSize + 1);

        MultiByteToWideChar(CP_ACP, 0, strTextLocal.c_str(), -1, strUnicode.data(), (int)strUnicode.size());

        strUnicode[strUnicode.size() - 1] = NULL;

        vector<CHAR> strUTF8;

        nSize = WideCharToMultiByte(CP_UTF8, 0, strUnicode.data(), -1, NULL, 0, NULL, NULL);

        strUTF8.resize(nSize + 1);

        WideCharToMultiByte(CP_UTF8, 0, strUnicode.data(), -1, strUTF8.data(), (int)strUTF8.size(), NULL, NULL);

        strUTF8[strUTF8.size() - 1] = NULL;

        strTextUtf8 = strUTF8.data();
    }

    //////////////////////////////////////////////////////////////////////////
    //CAppInfo

	CAppInfo::CAppInfo()
	{
		ZeroMemory(m_szFilePath, MAX_PATH);

		ZeroMemory(m_szAppDir, MAX_PATH);

		GetModuleFileNameA(NULL, m_szFilePath, MAX_PATH);

		m_pFileName = strrchr(m_szFilePath, '\\') + 1;

		strcpy(m_szAppDir, m_szFilePath);

		PCHAR lastPtr = strrchr(m_szAppDir, '\\');

		*lastPtr = NULL;
	}

	CAppInfo::~CAppInfo()
	{
		m_pFileName = NULL;
	}

    PCHAR CAppInfo::GetAppFilePath()
    {
        return m_szFilePath;
    }

    PCHAR CAppInfo::GetAppFileName()
    {
        return m_pFileName;
    }

    PCHAR CAppInfo::GetAppDirectory()
    {
        return m_szAppDir;
    }

    //////////////////////////////////////////////////////////////////////////
    //CAppInfoW

    CAppInfoW::CAppInfoW()
    {
        ZeroMemory(m_szFilePathW, MAX_PATH * sizeof(WCHAR));

        ZeroMemory(m_szAppDirW, MAX_PATH * sizeof(WCHAR));

        GetModuleFileNameW(NULL, m_szFilePathW, MAX_PATH * sizeof(WCHAR));

        m_pFileNameW = wcsrchr(m_szFilePathW, L'\\') + 1;

        wcscpy(m_szAppDirW, m_szFilePathW);

        PWCHAR lastPtr = wcsrchr(m_szAppDirW, L'\\');
    
        *lastPtr = NULL;
    }

    CAppInfoW::~CAppInfoW()
    {
        m_pFileNameW = NULL;
    }

    PWCHAR CAppInfoW::GetAppFilePath()
    {
        return m_szFilePathW;
    }

    PWCHAR CAppInfoW::GetAppFileName()
    {
        return m_pFileNameW;
    }

    PWCHAR CAppInfoW::GetAppDirectory()
    {
        return m_szAppDirW;
    }

    //////////////////////////////////////////////////////////////////////////
    //CGUID

    CGUID::CGUID()
    {
        CoCreateGuid(&m_guid);
    }

    CGUID::~CGUID()
    {

    }

    GUID CGUID::Get()
    {
        return m_guid;
    }

    GUID CGUID::ReCreate()
    {
        CoCreateGuid(&m_guid);

        return m_guid;
    }

    string CGUID::ToString()
    {
        static char buf[128] = {0};

        sprintf( buf,
            "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            m_guid.Data1, m_guid.Data2, m_guid.Data3,
            m_guid.Data4[0], m_guid.Data4[1],
            m_guid.Data4[2], m_guid.Data4[3],
            m_guid.Data4[4], m_guid.Data4[5],
            m_guid.Data4[6], m_guid.Data4[7]);

        return buf;
    }

    string CGUID::CreateToString()
    {
        CGUID aGuid;

        return aGuid.ToString();
    }

    //////////////////////////////////////////////////////////////////////////

    CMath::CMath()
    {

    }

    CMath::~CMath()
    {

    }

    bool CMath::GetRandom(unsigned char* buf ,int iCount)
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