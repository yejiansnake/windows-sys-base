#pragma once

#include "SysBase.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//声明

	class CBufferQueueImp;

	//////////////////////////////////////////////////////////////////////////
	//CBufferBase

	class CBufferBase : public SysBase::CObject
	{
	public:

		CBufferBase(){};

		virtual ~CBufferBase(){};

		friend class CBufferQueueImp;

	protected:

		char* m_pBuffer;

		UINT32 m_nSize;

		UINT32 m_nDataSize;

		CBufferQueueImp* m_pCBufferQueueImp;
	};

	//////////////////////////////////////////////////////////////////////////
	//CBufferRead

	class CBufferRead : public CBufferBase
	{
	public:

		CBufferRead();

		virtual ~CBufferRead();

		const char* GetData(UINT32 nOffset = 0);

		UINT32 GetDataSize();

		UINT32 GetSize();

		void Release();
	};

	//////////////////////////////////////////////////////////////////////////
	//CBufferWrite

	class CBufferWrite : public CBufferBase
	{
	public:

		//////////////////////////////////////////////////////////////////////////

		enum ERROR_CODE
		{
			ERROR_CODE_SUCCESS = 0,			//成功
			ERROR_CODE_PARAM,				//参数错误
			ERROR_CODE_OUT_RANGE,			//写入数据缓冲区最大超出范围
			ERROR_CODE_INVALID,				//对象无效
		};

		//////////////////////////////////////////////////////////////////////////

		CBufferWrite();

		virtual ~CBufferWrite();

		//数据压入源队列（调用后该对象不再可用）
		void Push();

		//释放，源队列回收空间，但并不加入队列（调用后该对象不再可用）
		void Release();

		ERROR_CODE AddData(const void* pData, UINT32 nDataSize);

		ERROR_CODE SetData(const void* pData, UINT32 nDataSize);

		char* GetData(UINT32 nOffset = 0);

		UINT32 GetDataSize();

		ERROR_CODE SetDataSize(UINT32 nDataSize);

		UINT32 GetSize();

		void Clear();
	};

	//////////////////////////////////////////////////////////////////////////

	class CBufferQueue : public SysBase::CObject
	{
	public:
		
		//////////////////////////////////////////////////////////////////////////

		enum ERROR_CODE
		{
			ERROR_CODE_SUCCESS = 0,			//成功
			ERROR_CODE_PARAM,				//参数错误
			ERROR_CODE_FULL,				//队列满
			ERROR_CODE_SYSTEM,				//系统级别问题（一般为内存错误或系统对象初始化失败）
			ERROR_CODE_NO_INIT,				//未初始化
			ERROR_CODE_EMPTY,				//队列空，没有新数据
			ERROR_CODE_MORE_DATA_SIZE,		//输出或压入的数据比目标缓冲区大
		};

		//////////////////////////////////////////////////////////////////////////

		CBufferQueue();

		virtual ~CBufferQueue();

		ERROR_CODE Init(UINT32 nSize, UINT32 nCount, bool bSemaphore = false);

        void Release();

		ERROR_CODE GetBufferWrite(CBufferWrite& aCBufferWrite);

		ERROR_CODE Push(CBufferWrite& aCBufferWrite);

		ERROR_CODE Push(const void* pData, UINT32 nDataSize);

		ERROR_CODE Pop(void* pBuffer, UINT32 nBufferSize, UINT32& nDataSize, UINT32 nWaitTime = 1000);

		ERROR_CODE Pop(CBufferRead& aCBufferRead, UINT32 nWaitTime = 1000);

		UINT32 GetCount();

		UINT32 GetIdleCount();

		UINT32 GetItemCount();

        void Clear();

        bool Empty();

	private:

		void* m_pImp;
	};
}
