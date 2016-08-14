#include "../SysBase_Interface.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CBufferPoolImp

	class CBufferPoolImp
	{
	public:

		CBufferPoolImp();

		virtual ~CBufferPoolImp();

		CBufferPool::ERROR_CODE Init(UINT32 nSize, UINT32 nCount);

		CPBuffer* Allocate();

		void Free(CPBuffer* pCPBuffer);

		UINT32 GetSize();

		UINT32 GetIdleCount();

		UINT32 GetCount();

        void Release(CPBuffer* pCPBuffer);

	private:

		struct TBufferBlock 
		{
			char* pBuffer;
		};

		bool m_bInit;

		UINT32 m_nSize;

		UINT32 m_nCount;

		char* m_pBuffer;

		CQueue<TBufferBlock> m_IdleQueue;
	};

	//////////////////////////////////////////////////////////////////////////
	//CPBuffer

    CPBuffer::CPBuffer()
    {
        m_pBuffer = NULL;

        m_nSize = 0;

        m_nDataSize = 0;

        m_pCBufferPoolImp = NULL;
    }

    CPBuffer::~CPBuffer()
    {
        this->Release();
    }

    CPBuffer::ERROR_CODE CPBuffer::AddData(const void* pData, UINT32 nDataSize)
    {
        if (!m_pCBufferPoolImp)
        {
            return ERROR_CODE_INVALID;
        }

        if (!pData || 0 == nDataSize)
        {
            return ERROR_CODE_PARAM;
        }

        if (nDataSize + m_nDataSize > m_nSize)
        {
            return ERROR_CODE_OUT_RANGE;
        }

        memcpy(m_pBuffer + m_nDataSize, pData, nDataSize);

        m_nDataSize += nDataSize;

        return ERROR_CODE_SUCCESS;
    }

    CPBuffer::ERROR_CODE CPBuffer::SetData(const void* pData, UINT32 nDataSize)
    {
        if (!m_pCBufferPoolImp)
        {
            return ERROR_CODE_INVALID;
        }

        if (!pData || 0 == nDataSize)
        {
            return ERROR_CODE_PARAM;
        }

        if (nDataSize > m_nSize)
        {
            return ERROR_CODE_OUT_RANGE;
        }

        memcpy(m_pBuffer, pData, nDataSize);

        m_nDataSize = nDataSize;

        return ERROR_CODE_SUCCESS;
    }

    char* CPBuffer::GetData(UINT32 nOffset)
    {
        if (!m_pCBufferPoolImp)
        {
            return NULL;
        }

        if (nOffset > m_nSize)
        {
            return NULL;
        }

        return (m_pBuffer + nOffset);
    }

    UINT32 CPBuffer::GetDataSize()
    {
        if (!m_pCBufferPoolImp)
        {
            return 0;
        }

        return m_nDataSize;
    }

    CPBuffer::ERROR_CODE CPBuffer::SetDataSize(UINT32 nDataSize)
    {
        if (!m_pCBufferPoolImp)
        {
            return ERROR_CODE_INVALID;
        }

        if (nDataSize > m_nSize)
        {
            return ERROR_CODE_OUT_RANGE;
        }

        m_nDataSize = nDataSize;

        return ERROR_CODE_SUCCESS;
    }

    UINT32 CPBuffer::GetSize()
    {
        if (!m_pCBufferPoolImp)
        {
            return 0;
        }

        return m_nSize;
    }

    void CPBuffer::Clear()
    {
        if (!m_pCBufferPoolImp)
        {
            return;
        }

        m_nDataSize = 0;

        ZeroMemory(m_pBuffer, m_nSize);
    }

    void CPBuffer::Release()
    {
        if (!m_pCBufferPoolImp)
        {
            return;
        }

        m_pCBufferPoolImp->Release(this);

        m_pCBufferPoolImp = NULL;
    }

	//////////////////////////////////////////////////////////////////////////
	//CBufferPoolImp й╣ож

    CBufferPoolImp::CBufferPoolImp()
    {
        m_bInit = false;

        m_nSize = 0;

        m_nCount = 0;

        m_pBuffer = NULL;
    }

    CBufferPoolImp::~CBufferPoolImp()
    {
        if (m_pBuffer)
        {
            delete [] m_pBuffer;

            m_pBuffer = NULL;
        }

        m_IdleQueue.Clear();
    }

    CBufferPool::ERROR_CODE CBufferPoolImp::Init(UINT32 nSize, UINT32 nCount)
    {
        if (m_bInit)
        {
            return CBufferPool::ERROR_CODE_SUCCESS;
        }

        if (0 == nSize || 0 == nCount)
        {
            return CBufferPool::ERROR_CODE_PARAM;
        }

        m_nSize = nSize;

        m_nCount = nCount;

        if (!m_IdleQueue.Init(nCount, false, true))
        {
            return CBufferPool::ERROR_CODE_SYSTEM;
        }

        m_pBuffer = new(std::nothrow) char[nSize * nCount];

        if (!m_pBuffer)
        {
            return CBufferPool::ERROR_CODE_SYSTEM;
        }

        UINT32 nIndex = 0;

        for (nIndex = 0; nIndex < nCount; ++nIndex)
        {
            TBufferBlock aBlock = {0};
            aBlock.pBuffer = m_pBuffer + nSize * nIndex;

            m_IdleQueue.Push(aBlock);
        }

        m_bInit = true;

        return CBufferPool::ERROR_CODE_SUCCESS;
    }

    CPBuffer* CBufferPoolImp::Allocate()
    {
		if (!m_bInit)
		{
			return NULL;
		}

        TBufferBlock aBlock = {0};

        if (!m_IdleQueue.Pop(aBlock))
        {
            return NULL;
        }

        CPBuffer* pCPBuffer = new(std::nothrow) CPBuffer();

        if (!pCPBuffer)
        {
            m_IdleQueue.Push(aBlock);

            return NULL;
        }

        pCPBuffer->m_nDataSize = 0;

        pCPBuffer->m_nSize = m_nSize;

        pCPBuffer->m_pBuffer = aBlock.pBuffer;

        pCPBuffer->m_pCBufferPoolImp = this;

        return pCPBuffer;
    }

    void CBufferPoolImp::Free(CPBuffer* pCPBuffer)
    {
		if (!m_bInit)
		{
			return;
		}

        if (!pCPBuffer)
        {
            return;
        }

        if (pCPBuffer->m_pCBufferPoolImp != this)
        {
            return;
        }

        TBufferBlock aBlock = {0};

        aBlock.pBuffer = pCPBuffer->m_pBuffer;

        m_IdleQueue.Push(aBlock);

        pCPBuffer->m_pCBufferPoolImp = NULL;
    }

    UINT32 CBufferPoolImp::GetSize()
    {
		if (!m_bInit)
		{
			return 0;
		}

        return m_nSize;
    }

    UINT32 CBufferPoolImp::GetIdleCount()
    {
		if (!m_bInit)
		{
			return 0;
		}

        return m_IdleQueue.GetItemCount();
    }

    UINT32 CBufferPoolImp::GetCount()
    {
		if (!m_bInit)
		{
			return 0;
		}

        return m_nCount;
    }

    void CBufferPoolImp::Release(CPBuffer* pCPBuffer)
    {
		if (!m_bInit)
		{
			return;
		}

        if (!pCPBuffer)
        {
            return;
        }
        
        if (pCPBuffer->m_pCBufferPoolImp != this)
        {
            return;
        }

        TBufferBlock aBlock = {0};

        aBlock.pBuffer = pCPBuffer->m_pBuffer;

        m_IdleQueue.Push(aBlock);

        pCPBuffer->m_pCBufferPoolImp = NULL;
    }

	//////////////////////////////////////////////////////////////////////////
	//CBufferPool

	CBufferPool::CBufferPool()
	{
		m_pImp = new CBufferPoolImp();
	}

	CBufferPool::~CBufferPool()
	{
		if (m_pImp)
		{
			CBufferPoolImp* pImp = (CBufferPoolImp*)m_pImp;

			delete pImp;

			m_pImp = NULL;
		}
	}

	CBufferPool::ERROR_CODE CBufferPool::Init(UINT32 nSize, UINT32 nCount)
	{
		if (!m_pImp)
		{
			return CBufferPool::ERROR_CODE_SYSTEM;
		}

		CBufferPoolImp* pImp = (CBufferPoolImp*)m_pImp;

		return pImp->Init(nSize, nCount);
	}

	CPBuffer* CBufferPool::Allocate()
	{
		if (!m_pImp)
		{
			return NULL;
		}

		CBufferPoolImp* pImp = (CBufferPoolImp*)m_pImp;

		return pImp->Allocate();
	}

	void CBufferPool::Free(CPBuffer* pCPBuffer)
	{
		if (!m_pImp)
		{
			return;
		}

		CBufferPoolImp* pImp = (CBufferPoolImp*)m_pImp;

		return pImp->Free(pCPBuffer);
	}

	UINT32 CBufferPool::GetSize()
	{
		if (!m_pImp)
		{
			return 0;
		}

		CBufferPoolImp* pImp = (CBufferPoolImp*)m_pImp;

		return pImp->GetSize();
	}

	UINT32 CBufferPool::GetIdleCount()
	{
		if (!m_pImp)
		{
			return 0;
		}

		CBufferPoolImp* pImp = (CBufferPoolImp*)m_pImp;

		return pImp->GetIdleCount();
	}

	UINT32 CBufferPool::GetCount()
	{
		if (!m_pImp)
		{
			return 0;
		}

		CBufferPoolImp* pImp = (CBufferPoolImp*)m_pImp;

		return pImp->GetCount();
	}
}