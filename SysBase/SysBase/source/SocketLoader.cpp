#include "../SysBase_Interface.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//CSocketLoader

	CSocketLoader CSocketLoader::s_CSocketLoader;

	BOOL CSocketLoader::bInitSocket = false;

	CSocketLoader::CSocketLoader()
	{
	}

	CSocketLoader::~CSocketLoader()
	{
		WSACleanup();

		CSocketLoader::bInitSocket = false;
	}

	bool CSocketLoader::InitSocket()
	{
		if (CSocketLoader::bInitSocket)
		{
			return true;
		}

		WSADATA wsaData;

		if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
		{
			return false;
		}

		CSocketLoader::bInitSocket = true;

		return true;
	}
}