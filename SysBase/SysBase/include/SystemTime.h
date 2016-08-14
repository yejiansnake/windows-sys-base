#pragma once

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CSystemTime

    class CSystemTime
    {
    public:
		struct WindowsPatchResult
		{
			INT64 Time;
			INT64 Ret;
		};
        static UINT64 GetTickCount();

        //获取开机时间
        static INT64 GetStartTime();

        //获取上次开机时间
        static INT64 GetLastShutdownTime();

		//获取系统安装时间
		static INT64 GetInstallTime();

		//获取上次补丁的安装时间
		static bool GetPatchInstallTime(WindowsPatchResult& result);

		//获取上次补丁的检测时间
		static bool GetPatchDectectTime(WindowsPatchResult& result);

		//获取上次补丁的下载时间
		static bool GetPatchDownloadTime(WindowsPatchResult& result);

		//获取指定长度的随机数
		static bool GetRandom(unsigned char* buf ,int iCount);
    private:

        CSystemTime();

        virtual ~CSystemTime();
    };
}
