#pragma once

#include "SysBase.h"
#include "Memory.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //引用计数基类

    class CBaseRef
    {
    public:
        CBaseRef()
        {
            m_nRefCount = 1;
        }
        virtual ~CBaseRef()
        {

        }

        void AddRef()
        {
            InterlockedIncrement(&m_nRefCount);
        }

        void Release()
        {
            if (0 == InterlockedDecrement(&m_nRefCount))
            {
                delete this;
            }
        }

    private:

        UINT32 m_nRefCount;
    };

    //////////////////////////////////////////////////////////////////////////
    //

    template <typename T>
    class CAutoRelease
    {
    public:

        CAutoRelease(T* ptr)
        {
            m_ConPtr = ptr;
        }

        virtual ~CAutoRelease()
        {
            if (m_ConPtr)
            {
                m_ConPtr->Release();
            }
        }

    protected:
    private:

        T* m_ConPtr;
    };

    #define CREATE_AUTO_RELEASE_PTR(type, ptr) CAutoRelease<type> CREATE_UNIQUE_VAR_NAME(_autoRelease)(ptr);

	//////////////////////////////////////////////////////////////////////////
	//CSharedPtr 共享智能指针

	template <typename T>
	class CSharedPtr : public SysBase::CObject
	{
	public:

        CSharedPtr()
        {
            m_pObj = NULL;
            m_plRefCount = NULL;
        }

		CSharedPtr(T* pObj)
		{
			if (!pObj)
			{
				throw "CSharedPtr 构造函数参数错误";
			}

			m_pObj = pObj;

			_NEW_EX(m_plRefCount, LONG);

			*m_plRefCount = 0;

			InterlockedIncrement(m_plRefCount);
		}

		CSharedPtr(CSharedPtr<T>& objPtr)
		{
            if (objPtr.m_plRefCount)
            {
                InterlockedIncrement(objPtr.m_plRefCount);

                m_plRefCount = objPtr.m_plRefCount;

                m_pObj = objPtr.m_pObj;
            }
            else
            {
                m_plRefCount = NULL;
                m_pObj = NULL;
            }
		}

		CSharedPtr<T>& operator =(CSharedPtr<T>& objPtr)
		{
			if (m_pObj)
			{
				if (0 == InterlockedDecrement(m_plRefCount))
				{
					_DELETE(m_plRefCount);

					_DELETE(m_pObj);
				}
			}

			InterlockedIncrement(objPtr.m_plRefCount);

			m_plRefCount = objPtr.m_plRefCount;

			m_pObj = objPtr.m_pObj;

			return *this;
		}

        bool IsEmpty()
        {
            return m_pObj == NULL ? true : false;
        }

		T* operator->()
		{
			return m_pObj;
		}

		virtual ~CSharedPtr()
		{
            if (m_plRefCount)
            {
                if (0 == InterlockedDecrement(m_plRefCount))
                {
                    _DELETE(m_plRefCount);

                    _DELETE(m_pObj);
                }
            }
		}

	private:

		LONG* m_plRefCount; 

		T* m_pObj;
	};

	//////////////////////////////////////////////////////////////////////////
	//CAutoArrayPtr 

	template <typename T> 
	class CAutoArrayPtr : public SysBase::CObject
	{
	public:

		CAutoArrayPtr(SIZE_T nArrayLength)
		{	
			m_ptr = new(std::nothrow) T[nArrayLength]();

			_NEW_VALIDATE(m_ptr);

			m_nLength = nArrayLength;
		}

		CAutoArrayPtr(CAutoArrayPtr& autoArrayPtr)
		{
			m_ptr = autoArrayPtr.m_ptr;

			m_nLength = autoArrayPtr.m_nLength; 

			autoArrayPtr.m_ptr = 0;
		}

		virtual ~CAutoArrayPtr()
		{
			if (m_ptr)
			{
				_DELETEA(m_ptr);
			}
		}

		T* operator->()
		{
			return m_ptr;
		}

		//获取分配的内存空间的首地址
		T* GetDataPtr()
		{
			return m_ptr;
		}

		//获取数组的长度
		SIZE_T GetDataLength()
		{
			return m_nLength;
		}

		//获取数据占用内存空间的大小
		SIZE_T GetDataSize()
		{
			return m_nLength * sizeof(T);
		}

	private:
		T* m_ptr;

		SIZE_T m_nLength;
	};
}