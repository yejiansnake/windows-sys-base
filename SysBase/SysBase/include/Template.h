#pragma once

#include "SysBase.h"
#include "ThreadSync.h"

namespace SysBase
{
    ////////////////////////////////////////////////////////////////////////////
    ////CQueue 队列 (先进先出)

    template<class T>
    class CQueue : public CObject
    {
    public:

        CQueue()
        {
            m_bInit = false;

            m_nFrontIndex = 0;

            m_nBackIndex = 0;

            m_Items = NULL;
        }

        bool Init(UINT32 nCount, bool bSemaphore = false, bool bThreadSafe = false)
        {
            if (m_bInit)
            {
                return true;
            }

            if (nCount <= 0)
            {
                return false;
            }

            m_nCount = nCount;

            m_nIdleCount = m_nCount;

            m_bSemaphore = bSemaphore;

			m_bThreadSafe = bThreadSafe;

            m_Items = new char[sizeof(T) * nCount];

			m_bInit = true;

            return true;
        }

        //功能:析构函数
        virtual ~CQueue()
        {
			if (!m_bInit)
			{
				return;
			}

            this->Clear();

            if (m_Items)
            {
                delete [] m_Items;

                m_Items = NULL;
            }
        }

        //功能:添加到队列
        //参数:
        //      T aItem IN 对象
        bool Push(const T& aItem)
        {
			if (!m_bInit)
			{
				return false;
			}

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Lock();
			}

            if (m_nIdleCount == 0)
            {
				if (m_bThreadSafe)
				{
					m_CThreadMutex.Unlock();
				}

                return false;
            }

            m_nIdleCount--;

            ::new(m_Items + sizeof(T) * m_nBackIndex) T(aItem);

            m_nBackIndex++;

            if (m_nBackIndex >= m_nCount)
            {
                m_nBackIndex = 0;
            }

            if (m_bSemaphore)
            {
                m_CSemaphore.Add();
            }

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Unlock();
			}

            return true;
        }

        //功能:获取对象
        //参数: 
        //		T& aItem OUT 队列中的对象数据
        //      UINT32 nWaitTime IN 等待时间(开启信号量时有效)
        //返回值:true 取到数据;false 取不到数据
        bool Pop(T& aItem, UINT32 nWaitTime = 0)
        {
			if (!m_bInit)
			{
				return false;
			}

            if (m_bSemaphore)
            {
                if (!m_CSemaphore.Wait(nWaitTime))
                {
                    return false;
                }
            }

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Lock();
			}

            if (m_nIdleCount == m_nCount)
            {
				if (m_bThreadSafe)
				{
					m_CThreadMutex.Unlock();
				}

                return false;
            }

            T* pItem = (T*)(m_Items + sizeof(T) * m_nFrontIndex);

            aItem = *pItem;

            pItem->~T();

            m_nIdleCount++;

            m_nFrontIndex++;

            if (m_nFrontIndex >= m_nCount)
            {
                m_nFrontIndex = 0;
            }

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Unlock();
			}

            return true;
        }

        //功能:获取队列数
        //返回值:队列数
        UINT32 GetItemCount()
        {
			if (!m_bInit)
			{
				return 0;
			}

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Lock();
			}

			UINT32 nValue = m_nCount - m_nIdleCount;

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Unlock();
			}

            return nValue;
        }

        UINT32 GetIdleCount()
        {
			if (!m_bInit)
			{
				return 0;
			}

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Lock();
			}

			UINT32 nValue = m_nIdleCount;

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Unlock();
			}

            return nValue;
        }

        UINT32 GetCount()
        {
			if (!m_bInit)
			{
				return 0;
			}

            return m_nCount;
        }

        //功能:队列是否空
        //返回值:是否空
        bool Empty()
        {
			if (!m_bInit)
			{
				return true;
			}

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Lock();
			}

            if (m_nIdleCount == m_nCount)
            {
				if (m_bThreadSafe)
				{
					m_CThreadMutex.Unlock();
				}

                return true;
            }

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Unlock();
			}

            return false;
        }

        //功能:清空队列
        void Clear()
        {
			if (!m_bInit)
			{
				return;
			}

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Lock();
			}

            if (m_bSemaphore)
            {
                while (m_CSemaphore.Wait(0))
                {
                    //不做任何事情
                }
            }

            while (m_nIdleCount != m_nCount)
            {
                T* pItem = (T*)(m_Items + sizeof(T) * m_nFrontIndex);

                pItem->~T();

                m_nFrontIndex++;

                if (m_nFrontIndex >= m_nCount)
                {
                    m_nFrontIndex = 0;
                }

                m_nIdleCount++;
            }

            m_nFrontIndex = 0;

            m_nBackIndex = 0;

			if (m_bThreadSafe)
			{
				m_CThreadMutex.Unlock();
			}
        }

    private:

        bool m_bInit;

        UINT32 m_nIdleCount;

        UINT32 m_nCount;

        UINT32 m_nFrontIndex;

        UINT32 m_nBackIndex;

        char* m_Items;

        bool m_bSemaphore;

		bool m_bThreadSafe;

        CSemaphore m_CSemaphore;

        CThreadMutex m_CThreadMutex;
    };

    //////////////////////////////////////////////////////////////////////////
    //支持作为 map<KEY,VALUE> 中 KEY 使用的模版类

    template<typename T>
    class CMapKey
    {
    public:

        CMapKey()
        {
            ZeroMemory(&m_Obj, sizeof(T));
        }

        virtual ~CMapKey()
        {

        }

        CMapKey(const T& obj)
        {
            memcpy(&m_Obj, &obj, sizeof(T));
        }

        CMapKey(const CMapKey& mapKey)
        {
            memcpy(&m_Obj, &mapKey.m_Obj, sizeof(T));
        }

        //////////////////////////////////////////////////////////////////////////

        CMapKey& operator = (const T& obj)
        {
            memcpy(&m_Obj, &obj, sizeof(T));

            return *this;
        }

        bool operator> (const T& obj) const
        {
            return memcmp(&m_Obj, &obj, sizeof(T)) > 0 ? true : false;
        }

        bool operator< (const T& obj) const
        {
            return memcmp(&m_Obj, &obj, sizeof(T)) < 0 ? true : false;
        }

        bool operator== (const T& obj) const
        {
            return memcmp(&m_Obj, &obj, sizeof(T)) == 0 ? true : false;
        }

        //////////////////////////////////////////////////////////////////////////

        CMapKey& operator = (const CMapKey& mapKey)
        {
            memcpy(&m_Obj, &mapKey.m_Obj, sizeof(T));

            return *this;
        }

        bool operator> (const CMapKey& mapKey) const
        {
            return memcmp(&m_Obj, &mapKey.m_Obj, sizeof(T)) > 0 ? true : false;
        }

        bool operator< (const CMapKey& mapKey) const
        {
            return memcmp(&m_Obj, &mapKey.m_Obj, sizeof(T)) < 0 ? true : false;
        }

        bool operator== (const CMapKey& mapKey) const
        {
            return memcmp(&m_Obj, &mapKey.m_Obj, sizeof(T)) == 0 ? true : false;
        }

        T* operator-> ()
        {
            return &m_Obj;
        }

    private:

        T m_Obj;
    };

    //////////////////////////////////////////////////////////////////////////
    //对象池

    template<typename T>
    class CPool
    {
    public:

        CPool()
        {

        }

        CPool(UINT32 nObjcetCount)
        {
            if (!this->Init(nObjcetCount))
            {
                throw "CPool init failed";
            }
        }

        virtual ~CPool()
        {
            this->Release();
        }

        bool Init(UINT32 nObjectCount)
        {
            if (0 == nObjectCount)
            {
                return false;
            }

            CREATE_SMART_MUTEX(m_lock);

            UINT32 nIndex = 0;

            for (; nIndex < nObjectCount; ++nIndex)
            {
                T* pObject = new(std::nothrow) T();

                if (!pObject)
                {
                    this->Release();

                    return false;
                }

                m_queue.push_back(pObject);
                m_pool.push_back(pObject);
            }

            return true;
        }

        void Release()
        {
            CREATE_SMART_MUTEX(m_lock);

            ObjectPool::iterator entity = m_pool.begin();
            ObjectPool::iterator endEntity = m_pool.end();

            for (; entity != endEntity; ++entity)
            {
                delete (*entity);
            }

            m_pool.clear();
            m_queue.clear();
        }

        T* Pop()
        {
            CREATE_SMART_MUTEX(m_lock);

            if (m_queue.size() < 0)
            {
                return NULL;
            }

            T* pObject = m_queue.front();
            m_queue.pop_front();

            return pObject;
        }

        void Push(T* pObject)
        {
            CREATE_SMART_MUTEX(m_lock);

            m_queue.push_back(pObject);
        }

    private:

        //////////////////////////////////////////////////////////////////////////

        typedef list<T*> ObjectQueue;
        typedef list<T*> ObjectPool;

        ObjectQueue m_queue;
        ObjectPool m_pool;

        CThreadMutex m_lock;
    };
}