#pragma once

#include "SysBase.h"

namespace SysBase
{
    struct TSysEventLogInfo 
    {
        UINT32 nEventNumber;        //事件编号
        UINT16 nEventLevel;         //事件级别 0:Error; 1:Warning; 2:Info; 3:Success Audit; 4:Failure Audit
        UINT16 nEventID;            //事件ID
        //UINT8 nEventSeverity;       //事件严重级别 0:成功;1:信息;2:警告;3:错误
        //UINT8 nEventCustomer;       //客户位 0:System; 1:Customer;
        UINT16 nEventCategory;      //事件类别
        UINT64 nTimeGenerated;      //事件提交时间（UTC）
        UINT64 nTimeWritten;        //事件被系统服务写入到日志中的时间（UTC）
        string strUserName;         //事件相关用户名（为系统进程事件时无用户名信息）
        wstring strSourceName;      //事件源
        //wstring strDesc;            //描述
    };

    class CSysEventLog
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_PARAM,
            ERROR_CODE_SYSTEM,
            ERROR_CODE_HAS_OPEN,
            ERROR_CODE_NOT_OPEN,
        };

        //////////////////////////////////////////////////////////////////////////66a

        CSysEventLog();

        virtual ~CSysEventLog();

        ERROR_CODE Open(const char* szSourceName);

        void Close();

        ERROR_CODE GetCount(UINT32& recordCount);

        ERROR_CODE Read(list<TSysEventLogInfo>& sysEventLogInfoList);

    private:

        void* m_pImp;
    };
}
