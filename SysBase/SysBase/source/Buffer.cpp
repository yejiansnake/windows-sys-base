#include "../SysBase_Interface.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CBuffer »º³åÇø¹ÜÀí

    CBuffer::CBuffer(UINT32 nSize)
    {
        _NEWA_EX(m_pBuffer, char, nSize);

        m_nSize = nSize;

        ZeroMemory(m_pBuffer, m_nSize);

		m_nDataSize = 0;
    }

    CBuffer::~CBuffer()
    {
        _DELETEA(m_pBuffer)
    }

    UINT32 CBuffer::GetSize()
    {
        return m_nSize;
    }

    char* CBuffer::GetData(UINT32 nOffset)
    {
		if (nOffset > m_nSize)
		{
			return NULL;
		}

        return (m_pBuffer + nOffset);
    }

	UINT32 CBuffer::GetDataSize()
	{
		return m_nDataSize;
	}

	CBuffer::ERROR_CODE CBuffer::SetDataSize(UINT32 nDataSize)
	{
		if (m_nDataSize > m_nSize)
		{
			return ERROR_CODE_PARAM;
		}

		m_nDataSize = nDataSize;

		return ERROR_CODE_SUCCESS;
	}

	void CBuffer::ClearData()
	{
		ZeroMemory(m_pBuffer, m_nSize);
	}
}