#pragma once

#include "SocketCommon.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//WINDOWS SOCKET 初始化类，负责自动调用 WSAStartup 和 WSACleanup

	class CSocketLoader
	{
	public:

		~CSocketLoader();

		//////////////////////////////////////////////////////////////////////////
		//静态函数

		//初始化SOCKET类
		//异常
		static bool InitSocket();

	protected:

		CSocketLoader();

	private:

		static CSocketLoader s_CSocketLoader;

		static BOOL bInitSocket;
	};
}
