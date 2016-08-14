#include "../SysBase_Interface.h"

namespace SysBase
{
    CSingleMgr::CSingleMgr()
    {
		m_pCObjectList = new(std::nothrow) list<CObject*>();

        if (!m_pCObjectList)
        {
            throw "Memory Failed !";
        }
    }

    CSingleMgr::~CSingleMgr()
    {
		list<CObject*>::iterator entity = m_pCObjectList->begin();

        list<CObject*>::iterator endEntity = m_pCObjectList->end();

        for ( ; entity != endEntity; ++entity)
        {
            delete* entity;
           * entity = NULL;
        }
            
        delete m_pCObjectList;
    }

    void CSingleMgr::Add(CObject* pCObject)
    {
        s_CSingleManagerInstance.m_pCObjectList->push_front(pCObject);
    }

    CSingleMgr CSingleMgr::s_CSingleManagerInstance;
}