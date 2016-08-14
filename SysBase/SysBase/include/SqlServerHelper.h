#pragma once

#include "SysBase.h"

namespace SysBase
{
    struct TSqlServerInstanceInfo 
    {
        TSqlServerInstanceInfo()
        {
            nTcpPort = 0;
            nTcpDynamicPort = 0;
            nLanguageCode = 0;
        }

        string strName;             //实例名称
        string strVersion;          //版本信息（包含补丁信息）
        string strEdition;          //版本类型（企业版，开发版，标准版）
        string strCollation;        //排序规则
        string strSQLBinRoot;       //该实例的运行程序目录
        string strSQLDataRoot;      //该实例的数据目录
        string strSQLPath;          //该实例的目录
        string strSqlProgramDir;    //安装数据库的目录（泛）

        UINT32 nLanguageCode;       //语言编码

        UINT16 nTcpPort;            //TCP监听端口
        UINT16 nTcpDynamicPort;     //TCP动态监听端口

    };

    class CSqlServerHelper
    {
    public:

        static bool GetDBInstanceInfoList(list<TSqlServerInstanceInfo>& aSqlServerInstanceInfoList);

    private:

        CSqlServerHelper();

        virtual ~CSqlServerHelper();
    };
}
