#pragma once

#include "SysBase.h"

namespace SysBase
{
    //以太网适配器信息
    struct THWEthernetAdapterInfo
    {
        THWEthernetAdapterInfo()
        {
            nIndex = 0;
            bDhcpEnabled = false;
            bHaveWins = false;
            nLeaseObtained = 0;
            nLeaseExpires = 0;
            bAutoconfigEnabled = false;
            bAutoconfigActive = false;
            bConnected = false;
        }

        string Name;                                //适配器名称（GUID）
        string Description;                         //适配器描述（名称）
        string macAddr;                             //MAC地址
        UINT32 nIndex;                              //适配器索引
        bool bDhcpEnabled;                          //是否启用DHCP

        //KEY:IP地址 VALUE:子网掩码
        map<string, string> ipAddrList;             //IP地址

        list<string> gateWayList;                   //网关地址
        list<string> dhcpSrvList;                   //DHCP服务地址
        bool bHaveWins;                              //是否使用WINS
        list<string> primaryWinsSrvList;            //主要WINS服务地址
        list<string> secondaryWinsSrvList;          //次要WINS服务地址
        INT64 nLeaseObtained;                      //获得租约时间(单位：秒[UTC])
        INT64 nLeaseExpires;                       //租约过期时间(单位：秒)[在 nLeaseObtained 时间基础上的过期秒数]

        bool bAutoconfigEnabled;                    //自动配置是否启用
        bool bAutoconfigActive;                     //自动配置是否活动
        list<string> dnsAddrList;                   //DNS地址列表

        bool bConnected;                            //当前网卡状态（是否已经连接网络）
    };

    //以太网适配器性能信息
    struct TEthernetAdaptersPerformance
    {
        UINT32 nIndex;                  //编号
        UINT32 nMtu;                    //Maximum Transmission Unit (MTU)
        UINT32 nSpeed;                  //网卡每秒最大传输速度（单位：bits [ 1 bit = 1/8 byte ] ）
        UINT32 nAdminStatus;            //适配器状态：1:启用; 0:禁用
        UINT32 nLastChange;             //最后更改时间
        UINT32 nInOctets;               //收到的数据字节数
        UINT32 nInUcastPkts;            //接收的单播数据包数量
        UINT32 nInNUcastPkts;           //接收的非单播数据包数量（广播和多播数据包都包括在内）
        UINT32 nInDiscards;             //传入的数据包被丢弃，即使他们并没有错误
        UINT32 nInErrors;               //传入的数据包被丢弃，因为错误的数量
        UINT32 nInUnknownProtos;        //传入的数据包被丢弃，因为该协议是未知
        UINT32 nOutOctets;              //发送的数据字节数
        UINT32 nOutUcastPkts;           //发送的单播数据包数量
        UINT32 nOutNUcastPkts;          //发送的非单播数据包数量（广播和多播数据包都包括在内）
        UINT32 nOutDiscards;            //发送的数据包被丢弃，即使他们并没有错误
        UINT32 nOutErrors;              //发送的数据包被丢弃，因为错误的数量
        UINT32 nOutQueueLen;            //输出队列长度
    };

    class CHWEthernetAdapter
    {
    public:

        static bool GetEthernetAdaptersInfoList(list<THWEthernetAdapterInfo>& ethernetAdapterInfoList);

        static bool GertEthernetAdaptersIndexList(list<UINT32>& ethernetAdapterIndexList);

        static bool GetEthernetAdaptersPerformance(
            UINT32 ethernetAdapterIndex, 
            TEthernetAdaptersPerformance& ethernetAdapterPerformanceOut);

        //////////////////////////////////////////////////////////////////////////
        /*  网络使用率的计算方法：
            1.调用 GetEthernetAdaptersInfoList 先获取网卡信息 
            2.将网卡信息中的nIndex作为参数调用 GetEthernetAdaptersPerformance 获取性能数据
            3.第一次先记录 nInOctets + nOutOctets 的已收发的字节数
            4.间隔一段时间后，再调用 GetEthernetAdaptersPerformance 获得新收发字节数 (nInOctets + nOutOctets)
            5.新收发字节数 减去 旧的收发字节数，再除以 nSpeed ,就得到当前的网络使用率（计算时需都先转为 double ）
        */
        //////////////////////////////////////////////////////////////////////////

    private:

        CHWEthernetAdapter();
        virtual ~CHWEthernetAdapter();

    };
}
