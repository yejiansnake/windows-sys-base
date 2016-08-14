#pragma once

#include "SysBase.h"


#include <Winsock2.h>
#include <mswsock.h>
#include <Ws2tcpip.h>
#pragma comment( lib, "Ws2_32.lib" )

#include <map>
using namespace std;

//////////////////////////////////////////////////////////////////////////
//备注：使用有关于SOCKET库之前必须先调用CWinSocketLoader::InitSocket()
//      来初始化Socket库
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//宏

#define SOCKET_IP_MAX_SIZE 50               //IP地址字符串缓冲区最大大小
#define SOCKET_IP_MAX_LENGTH 49             //IP地址字符串最大长度
#define SOCKET_UDP_SIZE 1024             //默认的UDP的收发数据大小
#define SOCKET_UDP_SEND_BUFFER_COUNT 3000   //默认的UDP发送缓冲区个数

//SOCKET 地址相关------------------------------------->
#define INET_SOCKET_ADDRESS_STRUCT_IPV4 SOCKADDR_IN                 //IPV4 地址结构

#define INET_SOCKET_ADDRESS_STRUCT_IPV6 SOCKADDR_IN6                //IPV6 地址结构

#define INET_SOCKET_ADDRESS_STRUCT_LENGTH_IPV4 sizeof(SOCKADDR_IN)  //IPV4 地址结构大小

#define INET_SOCKET_ADDRESS_STRUCT_LENGTH_IPV6 sizeof(SOCKADDR_IN6) //IPV6 地址结构大小

#define INET_LOCALHOST_IPV4 "127.0.0.1"		//IPV4 的本地地址形式

#define INET_LOCALHOST_IPV6 "::1"			//IPV6 的本地地址形式

#define INET_ADDR_ANY_IPV4 "0.0.0.0"		//IPV4 的任意地址形式

#define INET_ADDR_ANY_IPV6 "::0"			//IPV6 的本地地址形式

//-------------------------------------<

//协议族 IPV4为PE_INET ; IPV6为PF_INET6
#ifdef SOCKET_IPV6

#define INET_PF_FAMILY PF_INET6

#define INET_SOCKET_ADDRESS_STRUCT INET_SOCKET_ADDRESS_STRUCT_IPV6  //地址结构[配置]

#define INET_SOCKET_ADDRESS_STRUCT_LENGTH INET_SOCKET_ADDRESS_STRUCT_LENGTH_IPV6    //地址结构大小[配置]

#define INET_LOCALHOST_IP INET_LOCALHOST_IPV6   //本地地址形式[配置]

#define INET_ADDR_ANY_IP INET_ADDR_ANY_IPV6		//任意地址形式[配置]

#else   //这里为 IPV4 协议

#define INET_PF_FAMILY PF_INET   

#define INET_SOCKET_ADDRESS_STRUCT INET_SOCKET_ADDRESS_STRUCT_IPV4  //地址结构[配置]

#define INET_SOCKET_ADDRESS_STRUCT_LENGTH INET_SOCKET_ADDRESS_STRUCT_LENGTH_IPV4    //地址结构大小[配置]

#define INET_LOCALHOST_IP INET_LOCALHOST_IPV4   //本地地址形式[配置]

#define INET_ADDR_ANY_IP INET_ADDR_ANY_IPV4		//任意地址形式[配置]

#endif

//////////////////////////////////////////////////////////////////////////
//定义结构及宏 

//WIN32 心跳包默认结构
typedef struct sysbase_tcp_keepalive
{ 
	u_long  onoff;
	u_long  keepalivetime; 
	u_long  keepaliveinterval; 
}SYSBASE_TCP_KEEPALIVE,* PSYSBASE_TCP_KEEPALIVE; 

#define SIO_KEEPALIVE_VALS   _WSAIOW(IOC_VENDOR,4) 

//////////////////////////////////////////////////////////////////////////

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//TSockAddr SOCKET地址数据结构

	struct TSockAddr
	{
		char szIP[SOCKET_IP_MAX_SIZE];
		USHORT uPort;
	};
}
