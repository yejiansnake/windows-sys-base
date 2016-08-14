#include "../SysBase_Interface.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CBufferQueueImp

	class CBufferQueueImp
	{
	public:

		CBufferQueueImp();

		virtual ~CBufferQueueImp();

		CBufferQueue::ERROR_CODE Init(UINT32 nSize, UINT32 nCount, bool bSemaphore = false);

        void Release();

		CBufferQueue::ERROR_CODE GetBufferWrite(CBufferWrite& CBufferWrite);

		CBufferQueue::ERROR_CODE Push(CBufferWrite& aCBufferWrite);

		CBufferQueue::ERROR_CODE Push(const void* pData, UINT32 nDataSize);

		CBufferQueue::ERROR_CODE Pop(void* pBuffer, UINT32 nBufferSize, UINT32& nDataSize, UINT32 nWaitTime);

		CBufferQueue::ERROR_CODE Pop(CBufferRead& aCBufferRead, UINT32 nWaitTim);

		UINT32 GetCount();

		UINT32 GetIdleCount();

		UINT32 GetItemCount();

        void Clear();

        bool Empty();

		//////////////////////////////////////////////////////////////////////////

		void Release(CBufferBase* pCBufferBase);

	private:

		struct TBufferBlock 
		{
			char* pBuffer;
			UINT32 nDataSize;
		};

		bool m_bInit;

		CThreadMutex m_CThreadMutex;

		CSemaphore m_CSemaphore;

		bool m_bSemaphore;

		UINT32 m_nSize;

		UINT32 m_nCount;

		char* m_pBuffer;

		CQueue<TBufferBlock> m_IdleQueue;

		CQueue<TBufferBlock> m_ItemQueue;
	};

	//////////////////////////////////////////////////////////////////////////
	//CBufferRead

	CBufferRead::CBufferRead()
	{
		m_pBuffer = NULL;

		m_nSize = 0;

		m_nDataSize = 0;

		m_pCBufferQueueImp = NULL;
	}

	CBufferRead::~CBufferRead()
	{
		this->Release();
	}

	const char* CBufferRead::GetData(UINT32 nOffset)
	{
		if (!m_pCBufferQueueImp)
		{
			return NULL;
		}

		if (nOffset > m_nDataSize)
		{
			return NULL;
		}

		return (m_pBuffer + nOffset);
	}

	UINT32 CBufferRead::GetDataSize()
	{
		if (!m_pCBufferQueueImp)
		{
			return 0;
		}

		return m_nDataSize;
	}

	UINT32 CBufferRead::GetSize()
	{
		if (!m_pCBufferQueueImp)
		{
			return 0;
		}

		return m_nSize;
	}

	void CBufferRead::Release()
	{
		if (!m_pCBufferQueueImp)
		{
			return;
		}

		m_pCBufferQueueImp->Release(this);

		m_pCBufferQueueImp = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//CBufferWrite

	CBufferWrite::CBufferWrite()
	{
		m_pBuffer = NULL;

		m_nSize = 0;

		m_nDataSize = 0;

		m_pCBufferQueueImp = NULL;
	}

	CBufferWrite::~CBufferWrite()
	{
		this->Release();
	}

	//数据压入源队列（调用后该对象不再可用）
	void CBufferWrite::Push()
	{
		if (!m_pCBufferQueueImp)
		{
			return;
		}

		m_pCBufferQueueImp->Push(*this);

		m_pCBufferQueueImp = NULL;
	}

	//释放，源队列回收空间，但并不加入队列（调用后该对象不再可用）
	void CBufferWrite::Release()
	{
		if (!m_pCBufferQueueImp)
		{
			return;
		}

		m_pCBufferQueueImp->Release(this);

		m_pCBufferQueueImp = NULL;
	}

	CBufferWrite::ERROR_CODE CBufferWrite::AddData(const void* pData, UINT32 nDataSize)
	{
		if (!m_pCBufferQueueImp)
		{
			return ERROR_CODE_INVALID;
		}

		if (!pData || 0 == nDataSize)
		{
			return ERROR_CODE_PARAM;
		}

		if (m_nDataSize + nDataSize > m_nSize)
		{
			return ERROR_CODE_OUT_RANGE;
		}

		memcpy(m_pBuffer + m_nDataSize, pData, nDataSize);

		m_nDataSize += nDataSize;

		return ERROR_CODE_SUCCESS;
	}

	CBufferWrite::ERROR_CODE CBufferWrite::SetData(const void* pData, UINT32 nDataSize)
	{
		if (!m_pCBufferQueueImp)
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

	char* CBufferWrite::GetData(UINT32 nOffset)
	{
		if (!m_pCBufferQueueImp)
		{
			return NULL;
		}

		if (nOffset > m_nSize)
		{
			return NULL;
		}

		return (m_pBuffer + nOffset);
	}

	UINT32 CBufferWrite::GetDataSize()
	{
		if (!m_pCBufferQueueImp)
		{
			return 0;
		}

		return m_nDataSize;
	}

	CBufferWrite::ERROR_CODE CBufferWrite::SetDataSize(UINT32 nDataSize)
	{
		if (!m_pCBufferQueueImp)
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

	UINT32 CBufferWrite::GetSize()
	{
		if (!m_pCBufferQueueImp)
		{
			return 0;
		}

		return m_nSize;
	}

	void CBufferWrite::Clear()
	{
		if (!m_pCBufferQueueImp)
		{
			return;
		}

		m_nDataSize = 0;

        ZeroMemory(m_pBuffer, m_nSize);
	}

	//////////////////////////////////////////////////////////////////////////
	//CBufferQueueImp 实现

	CBufferQueueImp::CBufferQueueImp()
	{
		m_bInit = false;

		m_nSize = 0;

		m_nCount = 0;

		m_pBuffer = NULL;
	}

	CBufferQueueImp::~CBufferQueueImp()
	{
		this->Release();
	}

	CBufferQueue::ERROR_CODE CBufferQueueImp::Init(UINT32 nSize, UINT32 nCount, bool bSemaphore)
	{
		if (m_bInit)
		{
			return CBufferQueue::ERROR_CODE_SUCCESS;
		}

		if (0 == nSize || 0 == nCount)
		{
			return CBufferQueue::ERROR_CODE_PARAM;
		}

		m_bSemaphore = bSemaphore;

		m_nSize = nSize;

		m_nCount = nCount;

		m_bSemaphore = bSemaphore;

		if (!m_IdleQueue.Init(nCount))
        {
            return CBufferQueue::ERROR_CODE_SYSTEM;
        }

		if (!m_ItemQueue.Init(nCount))
        {
            return CBufferQueue::ERROR_CODE_SYSTEM;
        }

		m_pBuffer = new(std::nothrow) char[nSize * nCount];

		if (!m_pBuffer)
		{
			return CBufferQueue::ERROR_CODE_SYSTEM;
		}

        ZeroMemory(m_pBuffer, nSize * nCount);

		UINT32 nIndex = 0;

		for (nIndex = 0; nIndex < nCount; ++nIndex)
		{
			TBufferBlock aBlock = {0};
			aBlock.nDataSize = 0;
			aBlock.pBuffer = m_pBuffer + nSize * nIndex;

			m_IdleQueue.Push(aBlock);
		}

		m_bInit = true;

		return CBufferQueue::ERROR_CODE_SUCCESS;
	}

    void CBufferQueueImp::Release()
    {
        if (!m_bInit)
        {
            return;
        }

        m_bInit = false;

        if (m_pBuffer)
        {
            delete[] m_pBuffer;

            m_pBuffer = NULL;
        }

        m_IdleQueue.Clear();

        m_ItemQueue.Clear();

        if (m_bSemaphore)
        {
            while (m_CSemaphore.Wait(0))
            {
                //这里什么都不错
            }
        }
    }

	CBufferQueue::ERROR_CODE CBufferQueueImp::GetBufferWrite(CBufferWrite& aCBufferWrite)
	{
		if (!m_bInit)
		{
			return CBufferQueue::ERROR_CODE_NO_INIT;
		}

		if (aCBufferWrite.m_pBuffer
			|| aCBufferWrite.m_pCBufferQueueImp
			|| 0 != aCBufferWrite.m_nDataSize)
		{
			return CBufferQueue::ERROR_CODE_PARAM;
		}

		CREATE_SMART_MUTEX(m_CThreadMutex);

		TBufferBlock aBlock = {0};

		if (!m_IdleQueue.Pop(aBlock))
		{
			return CBufferQueue::ERROR_CODE_FULL;
		}

		aCBufferWrite.m_pBuffer = aBlock.pBuffer;
		aCBufferWrite.m_nDataSize = 0;
		aCBufferWrite.m_nSize = m_nSize;
		aCBufferWrite.m_pCBufferQueueImp = this;

		return CBufferQueue::ERROR_CODE_SUCCESS;
	}

	CBufferQueue::ERROR_CODE CBufferQueueImp::Push(CBufferWrite& aCBufferWrite)
	{
		if (!m_bInit)
		{
			return CBufferQueue::ERROR_CODE_NO_INIT;
		}

		if (!aCBufferWrite.m_pBuffer
			|| !aCBufferWrite.m_pCBufferQueueImp
			|| aCBufferWrite.m_pCBufferQueueImp != this
			|| 0 == aCBufferWrite.m_nDataSize)
		{
			return CBufferQueue::ERROR_CODE_PARAM;
		}

		CREATE_SMART_MUTEX(m_CThreadMutex);

		TBufferBlock aBlock = {0};

		aBlock.nDataSize = aCBufferWrite.m_nDataSize;
		aBlock.pBuffer = aCBufferWrite.m_pBuffer;

		m_ItemQueue.Push(aBlock);

        aCBufferWrite.m_pCBufferQueueImp = NULL;

		if (m_bSemaphore)
		{
			m_CSemaphore.Add();
		}

		return CBufferQueue::ERROR_CODE_SUCCESS;
	}
 
	CBufferQueue::ERROR_CODE CBufferQueueImp::Push(const void* pData, UINT32 nDataSize)
	{
		if (!m_bInit)
		{
			return CBufferQueue::ERROR_CODE_NO_INIT;
		}

		if (!pData || 0 == nDataSize)
		{
			return CBufferQueue::ERROR_CODE_PARAM;
		}

		if (nDataSize > m_nSize)
		{
			return CBufferQueue::ERROR_CODE_MORE_DATA_SIZE;
		}

		CREATE_SMART_MUTEX(m_CThreadMutex);

		if (m_IdleQueue.Empty())
		{
			return CBufferQueue::ERROR_CODE_FULL;
		}

		TBufferBlock aBlock = {0};
		
		if (!m_IdleQueue.Push(aBlock))
		{
			return CBufferQueue::ERROR_CODE_FULL;
		}

		memcpy(aBlock.pBuffer, pData, nDataSize);

		aBlock.nDataSize = nDataSize;

		m_ItemQueue.Push(aBlock);

		if (m_bSemaphore)
		{
			m_CSemaphore.Add();
		}

		return CBufferQueue::ERROR_CODE_SUCCESS;
	}

	CBufferQueue::ERROR_CODE CBufferQueueImp::Pop(void* pBuffer, UINT32 nBufferSize, UINT32& nDataSize, UINT32 nWaitTime)
	{
		if (!m_bInit)
		{
			return CBufferQueue::ERROR_CODE_NO_INIT;
		}

		if (!pBuffer || 0 == nBufferSize)
		{
			return CBufferQueue::ERROR_CODE_PARAM;
		}

		if (m_bSemaphore)
		{
			if (!m_CSemaphore.Wait(nWaitTime))
			{
				return CBufferQueue::ERROR_CODE_EMPTY;
			}
		}

		CREATE_SMART_MUTEX(m_CThreadMutex);

		TBufferBlock aBlock = {0};
		
		if (!m_ItemQueue.Pop(aBlock))
		{
			return CBufferQueue::ERROR_CODE_EMPTY;
		}

		if (aBlock.nDataSize > nBufferSize)
		{
			memcpy(pBuffer, aBlock.pBuffer, nBufferSize);
		}
		else
		{
			memcpy(pBuffer, aBlock.pBuffer, aBlock.nDataSize);
		}

		nDataSize = aBlock.nDataSize;

		m_IdleQueue.Push(aBlock);

		return CBufferQueue::ERROR_CODE_SUCCESS;
	}

	CBufferQueue::ERROR_CODE CBufferQueueImp::Pop(CBufferRead& aCBufferRead, UINT32 nWaitTime)
	{
		if (!m_bInit)
		{
			return CBufferQueue::ERROR_CODE_NO_INIT;
		}

		if (m_bSemaphore)
		{
			if (!m_CSemaphore.Wait(nWaitTime))
			{
				return CBufferQueue::ERROR_CODE_EMPTY;
			}
		}

		CREATE_SMART_MUTEX(m_CThreadMutex);

		TBufferBlock aBlock = {0};
		
		if (!m_ItemQueue.Pop(aBlock))
		{
			return CBufferQueue::ERROR_CODE_EMPTY;
		}

		aCBufferRead.m_nDataSize = aBlock.nDataSize;

		aCBufferRead.m_pBuffer = aBlock.pBuffer;

		aCBufferRead.m_nSize = m_nSize;

		aCBufferRead.m_pCBufferQueueImp = this;

		return CBufferQueue::ERROR_CODE_SUCCESS;
	}

	UINT32 CBufferQueueImp::GetCount()
	{
		if (!m_bInit)
		{
			return 0;
		}

		return m_nCount;
	}

	UINT32 CBufferQueueImp::GetIdleCount()
	{
		if (!m_bInit)
		{
			return 0;
		}

		CREATE_SMART_MUTEX(m_CThreadMutex);

		return (UINT32)m_IdleQueue.GetItemCount();
	}

	UINT32 CBufferQueueImp::GetItemCount()
	{
		if (!m_bInit)
		{
			return 0;
		}

		CREATE_SMART_MUTEX(m_CThreadMutex);

		return (UINT32)m_ItemQueue.GetItemCount();
	}

	void CBufferQueueImp::Release(CBufferBase* pCBufferBase)
	{
		if (!pCBufferBase)
		{
			return;
		}

		CREATE_SMART_MUTEX(m_CThreadMutex);

		TBufferBlock aBlock = {0};
		aBlock.nDataSize = 0;
		aBlock.pBuffer = pCBufferBase->m_pBuffer;

		m_IdleQueue.Push(aBlock);

        pCBufferBase->m_pCBufferQueueImp = NULL;
	}

    void CBufferQueueImp::Clear()
    {
        if (!m_bInit)
        {
            return;
        }

        CREATE_SMART_MUTEX(m_CThreadMutex);

        TBufferBlock aTBufferBlock = {0};

        while (m_ItemQueue.Pop(aTBufferBlock, 0))
        {
            m_IdleQueue.Push(aTBufferBlock);
        }

        m_ItemQueue.Clear();

        if (m_bSemaphore)
        {
            while (m_CSemaphore.Wait(0))
            {
                //这里什么都不错
            }
        }
    }

    bool CBufferQueueImp::Empty()
    {
        if (!m_bInit)
        {
            return true;
        }

        CREATE_SMART_MUTEX(m_CThreadMutex);
            
        return m_ItemQueue.Empty();
    }

	//////////////////////////////////////////////////////////////////////////
	//CBufferQueue

	CBufferQueue::CBufferQueue()
	{
		m_pImp = new(std::nothrow) CBufferQueueImp();
	}

	CBufferQueue::~CBufferQueue()
	{
		if (m_pImp)
		{
			CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

			delete pImp;

			m_pImp = NULL;
		}
	}

	CBufferQueue::ERROR_CODE CBufferQueue::Init(UINT32 nSize, UINT32 nCount, bool bSemaphore)
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

		return pImp->Init(nSize, nCount, bSemaphore);
	}

    void CBufferQueue::Release()
    {
        if (!m_pImp)
        {
            return;
        }

        CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

        return pImp->Release();
    }

	CBufferQueue::ERROR_CODE CBufferQueue::GetBufferWrite(CBufferWrite& CBufferWrite)
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

		return pImp->GetBufferWrite(CBufferWrite);
	}

	CBufferQueue::ERROR_CODE CBufferQueue::Push(CBufferWrite& aCBufferWrite)
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

		return pImp->Push(aCBufferWrite);
	}

	CBufferQueue::ERROR_CODE CBufferQueue::Push(const void* pData, UINT32 nDataSize)
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

		return pImp->Push(pData, nDataSize);
	}

	CBufferQueue::ERROR_CODE CBufferQueue::Pop(void* pBuffer, UINT32 nBufferSize, UINT32& nDataSize, UINT32 nWaitTime)
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

		return pImp->Pop(pBuffer, nBufferSize, nDataSize, nWaitTime);
	}

	CBufferQueue::ERROR_CODE CBufferQueue::Pop(CBufferRead& aCBufferRead, UINT32 nWaitTim)
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

		return pImp->Pop(aCBufferRead, nWaitTim);
	}

	UINT32 CBufferQueue::GetCount()
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

		return pImp->GetCount();
	}

	UINT32 CBufferQueue::GetIdleCount()
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

		return pImp->GetIdleCount();
	}

	UINT32 CBufferQueue::GetItemCount()
	{
		if (!m_pImp)
		{
			return ERROR_CODE_SYSTEM;
		}

		CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

		return pImp->GetItemCount();
	}

    void CBufferQueue::Clear()
    {
        if (!m_pImp)
        {
            return;
        }

        CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

        pImp->Clear();
    }

    bool CBufferQueue::Empty()
    {
        if (!m_pImp)
        {
            return true;
        }

        CBufferQueueImp* pImp = (CBufferQueueImp*)m_pImp;

        return pImp->Empty();
    }
}