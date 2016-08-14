#pragma once

#include <map>

#include "SysBase.h"

using namespace std;

//注册表字符最大长度
#define REG_SZ_MAX_LENGTH 256

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CIniConfig 用于在profile类似INI文件格式的文件中读取或保存配置信息

    class CIniConfig 
    {
    public:
        
        CIniConfig();

        ~CIniConfig();

        //功能:构造函数
        //参数:
        //      const char* pFilePath IN 文件名
        //      bool bExist IN 是否判断文件存在
        //返回值：true成功；false 失败；当 bExist == false 时，总会返回true
        bool Load(const char* pFilePath, bool bExist = false);

        //功能:获取配置值
        //参数:
        //      const char* pSection IN 配置模块分类名
        //      const char* pKey IN 配置键
        //      char* pValue OUT 配置值(字符串的大小最少应该为 sizeof(TCHAR)*  1024)
        //      UINT32 nValueSize IN lpValue 缓冲区大小
        //      char* pDefaultValue IN 如果无配置信息，lpValue 则用默认值返回
        //返回值: 获取的字符串长度（长度不包括最后的NULL）
        UINT32 GetString(const char* pSection, const char* pKey, char* pValue, 
			UINT32 nValueSize, const char* pDefaultValue = "");

        //功能:设置配置值
        //参数:
        //      const char* pSection IN 配置模块分类名
        //      const char* pKey IN 配置健
        //      const char* pValue IN 配置值
        //返回值: 设置是否成功
        bool SetString(const char* pSection, const char* pKey, const char* pValue);
        
        //功能:获取整型配置值
        //参数:
        //      const char* pSection IN 配置模块分类名
        //      const char* pKey IN 配置健
        //      INT32 nDefault IN 默认值
        //返回值: 整型配置值
        UINT32 GetInt(const char* pSection, const char* pKey, INT32 nDefault);

        //功能:设置整型配置值
        //参数:
        //      const char* pSection IN 配置模块分类名
        //      const char* pKey IN 配置健
        //      INT32 nValue IN 默认值
        //返回值: 整型配置值
        bool SetInt(const char* pSection, const char* pKey, INT32 nValue);

        const char* GetFilePath(); 

    private:

        char m_szFilePath[MAX_PATH];
    };
}
