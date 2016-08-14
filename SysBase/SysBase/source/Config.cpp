#include "../SysBase_Interface.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CIniConfig

    CIniConfig::CIniConfig()
    {
        ZeroMemory(m_szFilePath, MAX_PATH);
    }

    CIniConfig::~CIniConfig()
    {
    }

    bool CIniConfig::Load(const char* pFilePath, bool bExist)
    {
        if (!pFilePath)
        {
            return false;
        }

        if (bExist)
        {
            if (_access(pFilePath, 0) != 0)
            {
                return false;   
            }
        }

        UINT32 nStrLen = (UINT32)strlen(pFilePath);

        ZeroMemory(m_szFilePath, MAX_PATH);

        if (nStrLen > (MAX_PATH - 1))
        {
            return false;
        }

        strncpy(m_szFilePath, pFilePath, nStrLen);

        return true;
    }

    UINT32 CIniConfig::GetString(const char* pSection, const char* pKey, char* pValue, UINT32 nValueSize, const char* pDefaultValue)
    {
        return (UINT32)GetPrivateProfileStringA(pSection, pKey, pDefaultValue, pValue, nValueSize, m_szFilePath);
    }

    bool CIniConfig::SetString(const char* pSection, const char* pKey, const char* pValue)
    {
        if (!WritePrivateProfileStringA(pSection, pKey, pValue, m_szFilePath))
		{
			return false;
		}

		return true;
    }
    
    UINT32 CIniConfig::GetInt(const char* pSection, const char* pKey, INT32 nDefault)
    {
        return GetPrivateProfileIntA(pSection, pKey, nDefault, m_szFilePath);
    }

    bool CIniConfig::SetInt(const char* pSection, const char* pKey, INT32 nValue)
    {
        char szTemp[128] = {0};

        sprintf_s(szTemp, 127, "%d", nValue);

        return this->SetString(pSection, pKey, szTemp);
    }

    const char* CIniConfig::GetFilePath()
    {
        return m_szFilePath;
    }
}