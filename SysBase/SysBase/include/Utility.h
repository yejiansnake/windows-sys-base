#pragma once

#include "SysBase.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CStringBuilder 字符串构造类(ANSII)

	class CStringBuilderA;
	class CStringBuilderW;

#ifndef _UNICODE
	typedef CStringBuilderA CStringBuilder;
#else
	typedef CStringBuilderW CStringBuilder;
#endif

    //////////////////////////////////////////////////////////////////////////
    //CStringBuilderA

    class CStringBuilderA : public SysBase::CObject
    {
    public:
        CStringBuilderA(void);
        CStringBuilderA(const char* szSource);
        CStringBuilderA(const CStringBuilderA& strBuilder);

        ~CStringBuilderA(void);

        void Append(const char* szSource);

        char* GetString(char* outStr);

        INT64 GetStrLength();

        void Clear();

    private:

        void Build(const char* szSource);
        void FreeStr();

        void* m_vsHead;
        void* m_vsEnd;

        INT64 m_nLength;
    };
    
    //////////////////////////////////////////////////////////////////////////
    //CStringBuilderW
    
	class CStringBuilderW : public SysBase::CObject
	{
	public:
		CStringBuilderW(void);
		CStringBuilderW(const WCHAR* szSource);
		CStringBuilderW(const CStringBuilderW& strBuilder);

		~CStringBuilderW(void);

		void Append(const WCHAR* szSource);

		WCHAR* GetString(WCHAR* outStr);

		INT64 GetStrLength();

		void Clear();

	private:

		void Build(const WCHAR* szSource);
		void FreeStr();

		void* m_vsHead;
		void* m_vsEnd;

		INT64 m_nLength;
	};

    //////////////////////////////////////////////////////////////////////////
    //Convert 类型转换命名空间

    class Convert
    {
    public:

        //功能:定位于当前文件指针
        //参数:
        //      UINT32 nLow IN 低位的DWORD
        //      UINT32 nHigh IN 高位的DOWRD
        //返回值: 转换的结果
        static UINT64 ToUINT64(UINT32 nLow, UINT32 nHigh);


        //功能:定位于当前文件指针
        //参数:
        //      UINT64 nInt64 IN 需要转换的数
        //      DWORD* pDwHigh OUT 高位的DOWRD,为NULL值时不返回高位
        //返回值: 低位的DWORD
        static DWORD ToDWORD(UINT64 nInt64, DWORD* pDwHigh);

        static void LocalToUTF8(string strTextLocal, string& strTextUtf8);
    };

    //////////////////////////////////////////////////////////////////////////
    //CAppInfo 应用程序信息

    class CAppInfo
    {
    public:

		CAppInfo();

		virtual ~CAppInfo();

        //功能:获取应用程序名(包含完整路径)
        //返回值: 应用程序名(包含完整路径)
        PCHAR GetAppFilePath();

        //功能:获取应用程序名(不包含完整路径)
        //返回值: 应用程序名
        PCHAR GetAppFileName();

        //功能:获取应用程序所在目录
        //返回值: 应用程序所在目录
        PCHAR GetAppDirectory();

	private:

		char m_szFilePath[MAX_PATH];

		char* m_pFileName;

		char m_szAppDir[MAX_PATH];
    };

    class CAppInfoW
    {
    public:

        CAppInfoW();

        virtual ~CAppInfoW();

        //功能:获取应用程序名(包含完整路径)
        //返回值: 应用程序名(包含完整路径)
        PWCHAR GetAppFilePath();

        //功能:获取应用程序名(不包含完整路径)
        //返回值: 应用程序名
        PWCHAR GetAppFileName();

        //功能:获取应用程序所在目录
        //返回值: 应用程序所在目录
        PWCHAR GetAppDirectory();

    private:

        WCHAR m_szFilePathW[MAX_PATH];

        PWCHAR m_pFileNameW;

        WCHAR m_szAppDirW[MAX_PATH];
    };

    //////////////////////////////////////////////////////////////////////////
    //唯一表示生成

    class CGUID
    {
    public:

        CGUID();

        virtual ~CGUID();

        GUID Get();

        GUID ReCreate();

        string ToString();

        static string CreateToString();

    protected:
    private:

        GUID m_guid;
    };

    //////////////////////////////////////////////////////////////////////////
    //算术类
    
    class CMath
    {
    public:

        //获取指定长度的随机数
        static bool GetRandom(unsigned char* buf ,int iCount);

    private:

        CMath();

        virtual ~CMath();
    };     
    
}
