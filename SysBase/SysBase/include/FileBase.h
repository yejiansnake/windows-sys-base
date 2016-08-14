#pragma once

#include "SysBase.h"

namespace SysBase
{
    class CFileBase
    {
    public:

        //功能:文件或文件夹是否存在
        //参数:
        //      const char* szPath IN 路径
        //返回值: 是否存在
        static bool IsExist(const char* szPath);

        //功能:文件或文件夹是否存在
        //参数:
        //      const char* szPath IN 路径
        //返回值: 是否存在
        static bool IsExist(const WCHAR* szPath);

        //功能:重命名文件或文件夹
        //参数:
        //      const char* szOldPath IN 旧目录名
        //      const char* szNewPath IN 新目录名
        //返回值: 是否成功
        static bool Rename(const char* szOldPath, const char* szNewPath);

        //功能:重命名文件或文件夹
        //参数:
        //      const char* szOldPath IN 旧目录名
        //      const char* szNewPath IN 新目录名
        //返回值: 是否成功
        static bool Rename(const WCHAR* szOldPath, const WCHAR* szNewPath);

    protected:
    private:
    };
}