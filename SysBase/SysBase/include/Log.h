#pragma once

#include "SysBase.h"

#define SYSBASE_LOG_FILE_MAX_SIZE 1024 * 1024 * 20      //20M

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CLog:输出到文件的日志信息类

    class CLog : public CObject
    {
    public:

		//////////////////////////////////////////////////////////////////////////
		//错误码

		enum ERROR_CODE
		{
			ERROR_CODE_SUCCESS = 0,
			ERROR_CODE_PARAM,
			ERROR_CODE_SYSTEM,
			ERROR_CODE_HAS_INIT,
		};

		//////////////////////////////////////////////////////////////////////////
		//日志信息等级

		enum LOG_LEVEL
		{
			LOG_LEVEL_NONE = 0,       //不输出信息
			LOG_LEVEL_ERROR = 1,      //错误信息
			LOG_LEVEL_WARNING = 2,    //警告信息
			LOG_LEVEL_INFO = 3,       //正常信息
			LOG_LEVEL_DEBUG = 4,      //调试信息
			LOG_LEVEL_SYSTEM = 5      //系统底层信息
		};

		//////////////////////////////////////////////////////////////////////////

        CLog();

		virtual ~CLog();

        //功能:初始化(使用该类前必须先调用)
		//参数:
        //      const char* pFilePath IN 日志文件保存路径
        //      TRACE_LEVEL eLevel IN 日志保存等级
        //      UINT32 nMaxFileSize IN 日志文件最大大小
        //      bool bOutputDefault IN 是否默认输出日期，时间，线程编号
        //      bool bAutoLine IN 是否默认输出内容与日志实际内容输出时自动换行
        //返回值：ERROR_CODE_SUCCESS 为成功，其他参考 ERROR_CODE
        ERROR_CODE Init(
			const char* pFilePath, 
			LOG_LEVEL eLevel = LOG_LEVEL_DEBUG, 
			UINT32 nMaxFileSize = SYSBASE_LOG_FILE_MAX_SIZE, 
			bool bOutputDefault = true,
            bool bAutoLine = false);

		//功能:写入日志信息
		//参数:
		//      TRACE_LEVEL eLevel IN 日志等级
		//       const char* szFormat IN 格式化字符串
		void Write(LOG_LEVEL eLevel, const char* szFormat, ...);

		//功能:设置日志等级
		//参数:
		//      TRACE_LEVEL eLevel IN 日志等级
		void SetLogLevel(LOG_LEVEL eLevel);

    private:

		void* m_pImp;
    };

    //////////////////////////////////////////////////////////////////////////
    //日志函数

    bool LOG_INIT(
        int nLogKey,
        const char* pFilePath, 
        CLog::LOG_LEVEL eLevel = CLog::LOG_LEVEL_DEBUG, 
        UINT32 nMaxFileSize = SYSBASE_LOG_FILE_MAX_SIZE, 
        bool bOutputDefault = true,
        bool bAutoLine = false);

    void LOG_WRITE(int nLogKey, CLog::LOG_LEVEL eLevel, const char* szFormat, ...);

    void LOG_SET_LEVEL(int nLogKey, CLog::LOG_LEVEL eLevel);

    //////////////////////////////////////////////////////////////////////////
}
