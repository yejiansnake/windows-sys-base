#pragma once

#include "SysBase.h"

namespace SysBase
{
    class CDirectory
    {
    public:

        //功能:创建目录
        //参数:
        //      const char* szPathName IN 需要创建的目录路径
        //      bool bRecurrence IN 当没有根目录的时候是否递归创建
        //返回值: 是否成功
        static bool Create(const char* szPathName, bool bRecurrence = false);

        //功能:创建目录
        //参数:
        //      const WCHAR* wzPathName IN 需要创建的目录路径
        //      bool bRecurrence IN 当没有根目录的时候是否递归创建
        //返回值: 是否成功
        static bool Create(const WCHAR* wzPathName, bool bRecurrence = false);

        //功能:设置当前目录
        //参数:
        //      const char* szPathName IN 目标目录
        //返回值: 是否为目录
        static bool IsDirectory(const char* szPathName);

        //功能:设置当前目录
        //参数:
        //      const WCHAR* wzPathName IN 目标目录
        //返回值: 是否为目录
        static bool IsDirectory(const WCHAR* wzPathName);

        //功能:删除一个存在的目录 : 最好能加一个回调函数
        //参数:
        //      const char* szPathName IN 需要删除的目录路径
        //      bool bDeleteAllNode IN 是否删除该目录下的所有子目录和子文件，如果为false，删除的目录需要为空目录才可以删除
        //返回值: 是否成功
        static bool Delete(const char* szPathName, bool bDeleteAllNode = false);

        //功能:删除一个存在的目录 : 最好能加一个回调函数
        //参数:
        //      const WCHAR* szPathName IN 需要删除的目录路径
        //      bool bDeleteAllNode IN 是否删除该目录下的所有子目录和子文件，如果为false，删除的目录需要为空目录才可以删除
        //返回值: 是否成功
        static bool Delete(const WCHAR* szPathName, bool bDeleteAllNode = false);

        //功能:拷贝一个目录到另一个目录 : 最好能加一个回调函数
        //参数:
        //      const char* szFromPath IN 源目录
        //      const char* szToPath IN 目标目录
        //返回值: 是否成功
        static bool Copy(const char* szFromPath, const char* szToPath);

        //功能:拷贝一个目录到另一个目录 : 最好能加一个回调函数
        //参数:
        //      const WCHAR* szFromPath IN 源目录
        //      const WCHAR* szToPath IN 目标目录
        //返回值: 是否成功
        static bool Copy(const WCHAR* szFromPath, const WCHAR* szToPath);

        static bool Move(const char* szFromPath, const char* szToPath);

        static bool Move(const WCHAR* szFromPath, const WCHAR* szToPath);

        static bool GetOpenDirName(string& strPath, const char* szTitle = NULL);

        static bool GetOpenDirName(wstring& strPath, const WCHAR* wzTitle = NULL);

    private:

        CDirectory();

        virtual ~CDirectory();
    };
}
