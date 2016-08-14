//////////////////////////////////////////////////////////////////////////
//该文件下的类用来管理需要初始化才可以使用的函数，如加载DLL导出函数，
//SOCKET的init函数等
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SysBase.h"

using namespace std;

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //单件模板
    template <typename T>
    class CSingletonT
    {
    public:
  
        virtual ~CSingletonT(){}

    protected:

        CSingletonT(){}

    public:

        static T& Instance()
        {
            static T instance;
            return instance;
        } 
    };

    //单件管理类
    class CSingleMgr
    {
    public:
        
        //释放托管对象
        ~CSingleMgr();

        //增加托管的对象 //参数为继承 IInitLoader、IDLLLoader 这两个接口的类对象
        static void Add(CObject* pCObject);

    protected:

        CSingleMgr();

    private:

        list<CObject*>* m_pCObjectList;

        static CSingleMgr s_CSingleManagerInstance;
    };

    //初始化函数库的类，如SOCKET使用前需要先调用函数加载相应的版本函数库
    class IInitLoader : public CObject
    {
    public:
        
        //释放函数
        virtual ~IInitLoader(){};

        //初始化需要的过程
        virtual BOOL Init() = 0;

    protected:

    private:
    };

    //动态连接库加载程序
    class IDLLLoader : public CObject
    {
    public:

        //在这里释放加载的DLL
        virtual ~IDLLLoader(){};

        //加载DLL
        virtual BOOL Load(LPCWSTR lpFileName) = 0;

    private:

        HMODULE m_hModule;
    };

     
}
