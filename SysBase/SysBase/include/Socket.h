#pragma once

#include "SocketCommon.h"
#include "ThreadSync.h"

namespace SysBase
{
	//////////////////////////////////////////////////////////////////////////
	//Socket地址信息类
	
	class CSocketAddr
	{
	public:
	
#ifndef SOCKET_IPV6

		//功能:转换网络字节序IPV4地址信息为字符串
		//参数:
		//      char* szIP OUT 输出的缓冲区
		//		WORD wSize IN 缓冲区大小(必须大于等于 SOCKET_IP_MAX_SIZE = 50)
		//		UINT32 nIP IN IP地址
		//返回值:0 错误; >0 实际有效的数据大小（不包括\0）

		static WORD ToStringIP(char* szIP, WORD wSize, UINT32 nIP);
		
		//功能:转换IPV4地址字符串为网络字节序数值
		//参数:
		//		char* szIP IN 地址字符串
		//返回值: 网络字节序数值

		static DWORD ToDwordIP(const char* szIP);
#endif

		//功能:转换地址信息为pTSockAddr结构 
		//参数:
		//      INET_SOCKET_ADDRESS_STRUCT* pSockAddr IN 地址信息,不可为NULL
		//      TSockAddr* pTSockAddr OUT 地址信息结构指针
		static bool ToTSockAddr(
			const INET_SOCKET_ADDRESS_STRUCT* pSockAddr, 
			TSockAddr* pTSockAddr);
	
		//功能:转换地址信息为pTSockAddr结构 
		//参数:
		//      TSockAddr* pTSockAddr IN 地址信息结构指针
		//      INET_SOCKET_ADDRESS_STRUCT* pSockAddr OUT 地址信息,不可为NULL
		static bool ToInetSockAddr(
			const TSockAddr* pTSockAddr,
			INET_SOCKET_ADDRESS_STRUCT* pSockAddr);
	};

	//////////////////////////////////////////////////////////////////////////
	//CSOCKET 套接字类

	class CSocket : public CThreadMutex
	{
	public:

		virtual ~CSocket();

		//功能:closesocket（一般不会立即关闭而经历TIME_WAIT的过程）后是否重用该socket
		//参数:
		//      bool bIsReUse IN 是或否

		bool SetReuseAddr(bool bIsReUse);

		//功能:是否重用该socket
		//返回值: 重用或否

		bool IsReuseAddr();

		//功能: 是否要已经处于连接状态的soket在调用closesocket后强制关闭，不经历TIME_WAIT的过程
		//参数:
		//      bool bIsReUse IN 是或否

		bool SetDontLinger(bool bDontLinger);

		//功能: 已经处于连接状态的soket在调用closesocket后是否强制关闭，不经历TIME_WAIT的过程
		//返回值: 是否不等待

		bool IsDontLinger();

		//功能: 设置SOCKET发送缓冲区大小，可以使得系统一次可以发送更多的信息[系统默认的状态发送一次为8688字节(约为8K)]
		//      发送数据的，希望时不经历由系统缓冲区到socket缓冲区的拷贝而影响程序的性能，设置为 0 
		//参数:
		//      int nBufferSize IN 缓冲区大小

		bool SetSendBufferSize(int nBufferSize);

		//功能: 获取SOCKET发送缓冲区大小
		//返回值: 发送缓冲区大小

		int GetSendBufferSize();

		//功能: 设置SOCKET接收缓冲区大小，可以使得系统一次可以接收更多的信息[系统默认的状态接收一次为8688字节(约为8K)]
		//      接收数据的，希望时不经历由系统缓冲区到socket缓冲区的拷贝而影响程序的性能，设置为 0 
		//参数:
		//      int nBufferSize IN 缓冲区大小

		bool SetRecvBufferSzie(int nBufferSize);

		//功能: 获取SOCKET接收缓冲区大小
		//返回值: 接收缓冲区大小

		int GetRecvBufferSize();

		//功能: 设置在发送UDP数据报的时候，该socket发送的数据是否具有广播特性
		//参数:
		//      bool bBroadcast IN 是或否

		bool SetBroadcast(bool bBroadcast);

		//功能: 获取在发送UDP数据报的时候，该socket发送的数据是否具有广播特性
		//返回值: 是否

		bool IsBroadcast();

		//功能: 设置在client连接服务器过程中，如果处于非阻塞模式下的socket在connect()的过程中
		//      是否延时,直到accpet()被呼叫(本函数设置只有在非阻塞的过程中有显著的
		//      作用，在阻塞的函数调用中作用不大)
		//参数:
		//      bool bBroadcast IN 是或否

		bool SetConditionalAccept(bool bConditionalAccept);

		//功能: 获取处于非阻塞模式下的socket在connect()的过程中是否延时
		//返回值: 是否

		bool IsConditionalAccept();

		//功能: 设置等待多少秒让没发完的数据发送出去后再关闭socket[设置在发送数据的过程中(send()没有完成，
		//      还有数据没发送)而调用了closesocket(),数据是肯定丢失了]
		//参数:
		//      u_short dwSecond IN 等待多少秒

		bool SetWaitTime(u_short dwSecond);

		//功能: 获取等待多少秒
		//返回值: 等待的秒数

		WORD GetWaitTime();

		//功能: 打开连接保存检测
		//参数:
		//      UINT32 nIntervalTime IN 两次检测间隔的时间[单位为毫秒]
		//      UINT32 nTime IN 首次开始检测的时间[单位为毫秒]

		bool OpenKeepAlive(
			UINT32 nIntervalTime, 
			UINT32 nTime);

		//功能: 关闭连接保持检测

		bool CloseKeepAlive();

		//功能: 停止通信
		//参数:
		//  int nHow IN 枚举类型：SD_BOTH：关闭接收和发送、SD_RECEIVE：关闭接收、SD_SEND：关闭发送
		void Stop(int nHow = SD_BOTH);

		//功能: 是否已经停止
		//返回值:TURE 停止 false 未停止
		bool IsStop();

		//功能:关闭
		virtual void Close();

		//功能;是否已经关闭
		//返回值:TURE 关闭 false 未关闭
		bool IsClose();

		//获取 Socket 的 HANDLE 
		//返回值: Socket 的 HANDLE 
		SOCKET GetSocketHandle();

		//功能:获取远程地址
		//参数:
		//      INET_SOCKET_ADDRESS_STRUCT* pSockAddr OUT 地址信息,不可为NULL
		//返回值: 转换是否成功
		bool GetRemoveAddress(INET_SOCKET_ADDRESS_STRUCT* pSocketAddress);

		//功能:获取远程地址
		//参数:
		//      TSockAddr* pSockAddr OUT 地址信息,不可为NULL
		//返回值: 转换是否成功
		bool GetRemoveAddress(TSockAddr* pSocketAddress);

		//功能:获取本地地址
		//参数:
		//      INET_SOCKET_ADDRESS_STRUCT* pSockAddr OUT 地址信息,不可为NULL
		//返回值: 转换是否成功
		bool GetLoaclAddress(INET_SOCKET_ADDRESS_STRUCT* pSocketAddress);

		//功能:获取本地地址
		//参数:
		//      TSockAddr* pSockAddr OUT 地址信息,不可为NULL
		//返回值: 转换是否成功
		bool GetLoaclAddress(TSockAddr* pSocketAddress);

	protected:

		//构造函数

		CSocket();

		//功能:等待是否有数据可接收
		//参数:
		//		 UINT32 nWaitTime IN 等待操作的时间
		//返回值:有数据可接收或无
		bool WaitRecv(UINT32 nWaitTime);
		
		//功能:是否能发送数据
		//参数:
		//		 UINT32 nWaitTime IN 等待操作的时间
		//返回值:是或否
		bool WaitSend(UINT32 nWaitTime);
		
		//功能:设置是否为阻塞
		//参数:
		//      bool bBlock IN TURE 为阻塞模式 false 为非阻塞模式
		bool SetBlock(bool bBlock);

		//参数参考MSDN setsockopt
		bool SetSocketOption(
			int level, 
			int optname, 
			const char* optval, 
			int optlen);

		//参数参考MSDN getsockopt
		bool GetSocketOption(
			int level, 
			int optname, 
			char* optval, 
			int* optlen);

		//////////////////////////////////////////////////////////////////////////
		//数据成员

		bool m_bIsClose;    //是否关闭

		SOCKET m_Socket;    //主要的数据成员：SOCKET
	};
}