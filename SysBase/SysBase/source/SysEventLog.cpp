#include "../SysBase_Interface.h"

#pragma comment(lib, "Advapi32.lib")

#define SYS_EVENT_LOG_BUFFER_SIZE 1024 * 64

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////

    struct TEventLogID 
    {
        UINT16 nNoNeedToKnown;
        UINT16 nEventID;
    };

    class CSysEventLogImp
    {
    public:

        //////////////////////////////////////////////////////////////////////////66a

        CSysEventLogImp();

        virtual ~CSysEventLogImp();

        CSysEventLog::ERROR_CODE Open(const char* szSourceName);

        void Close();

        CSysEventLog::ERROR_CODE GetCount(UINT32& recordCount);

        CSysEventLog::ERROR_CODE Read(list<TSysEventLogInfo>& sysEventLogInfoList);

    private:

        HANDLE m_hEventLog;
    };

    //////////////////////////////////////////////////////////////////////////


    CSysEventLogImp::CSysEventLogImp()
    {
        m_hEventLog = NULL;
    }

    CSysEventLogImp::~CSysEventLogImp()
    {
        this->Close();
    }

    CSysEventLog::ERROR_CODE CSysEventLogImp::Open(const char* szSourceName)
    {
        if (!szSourceName)
        {
            return CSysEventLog::ERROR_CODE_PARAM;
        }

        if (m_hEventLog)
        {
            return CSysEventLog::ERROR_CODE_HAS_OPEN;
        }

        m_hEventLog = OpenEventLogA(NULL, szSourceName);      

        if (!m_hEventLog)
        {
            return CSysEventLog::ERROR_CODE_SYSTEM;
        }

        return CSysEventLog::ERROR_CODE_SUCCESS;
    }

    void CSysEventLogImp::Close()
    {
        if (!m_hEventLog)
        {
            return;
        }

        CloseEventLog(m_hEventLog);

        m_hEventLog = NULL;
    }

    CSysEventLog::ERROR_CODE CSysEventLogImp::GetCount(UINT32& recordCount)
    {
        if (!m_hEventLog)
        {
            return CSysEventLog::ERROR_CODE_NOT_OPEN;
        }

        DWORD dwThisRecord = 0;

        GetNumberOfEventLogRecords(m_hEventLog, &dwThisRecord);

        recordCount = dwThisRecord;

        return CSysEventLog::ERROR_CODE_SUCCESS;
    }

    CSysEventLog::ERROR_CODE CSysEventLogImp::Read(list<TSysEventLogInfo>& sysEventLogInfoList)
    {
        if (!m_hEventLog)
        {
            return CSysEventLog::ERROR_CODE_NOT_OPEN;
        }

        DWORD dwRead, dwNeeded, dwThisRecord = 0;
        PEVENTLOGRECORD pEventLogRecord = NULL; 
        char recvbuffer[SYS_EVENT_LOG_BUFFER_SIZE] = {0}; 

        pEventLogRecord = (PEVENTLOGRECORD)&recvbuffer; 

        while (ReadEventLogA(m_hEventLog,
            EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
            0,
            recvbuffer,
            SYS_EVENT_LOG_BUFFER_SIZE,
            &dwRead,
            &dwNeeded))
        {
            while (dwRead > 0) 
            {
                TSysEventLogInfo aTSysEventLogInfo = {0};

                TEventLogID* pTEventLogID = (TEventLogID*)&pEventLogRecord->EventID;
                aTSysEventLogInfo.nEventID = pTEventLogID->nEventID;
                aTSysEventLogInfo.nEventLevel = pEventLogRecord->EventType;
                aTSysEventLogInfo.nEventNumber = pEventLogRecord->RecordNumber;
                aTSysEventLogInfo.nEventCategory = pEventLogRecord->EventCategory;
                aTSysEventLogInfo.nTimeGenerated = pEventLogRecord->TimeGenerated;
                aTSysEventLogInfo.nTimeWritten = pEventLogRecord->TimeWritten;

                if (pEventLogRecord->UserSidOffset > 0 && pEventLogRecord->UserSidLength > 0)
                {
                    PSID pUserSID = (PSID)((PCHAR)pEventLogRecord + pEventLogRecord->UserSidOffset);

                    char szUserName[41] = {0};
                    DWORD dwNameSize = 41;
                    char szDomain[256] = {0};
                    DWORD dwDomainSize = 256;
                    SID_NAME_USE SNU;

                    if (LookupAccountSid(NULL, pUserSID, szUserName, &dwNameSize, szDomain, &dwDomainSize, &SNU))
                    {
                        aTSysEventLogInfo.strUserName = szUserName;
                    }
                }

                //SourceName
                WCHAR* pwzSourceNasme = (WCHAR*)((PCHAR)pEventLogRecord + sizeof(EVENTLOGRECORD));
                aTSysEventLogInfo.strSourceName = pwzSourceNasme;

                //////////////////////////////////////////////////////////////////////////
                /*  关于取描述信息的逻辑:
                    1.通过注册表的项： "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s" 
                        （其中 Application 为 OpenEventLog 的SourceName 参数）取 "EventMessageFile" 键值的信息(或 ParameterMessageFile 键值)，
                        如果能获取到字符串值，内容为 某 dll 的地址，并且可能同时存在多个地址（以 ; 号隔开）。
                    2.获取到的地址内容有可能保存的是系统环境变量，需要通过 GetEnvironmentVariable 获取环境变量
                        对应的实际字符串信息。
                    3.获得实际地址后，需要加载该DLL：

                        HANDLE hModule = LoadLibraryExW(  
                            szDllPath,                  //实际地址字符串信息
                            NULL,  
                            LOAD_LIBRARY_AS_DATAFILE); 
                        
                        //通过EventID获取对应的描述字符串信息（字符串中存在参数序列 %1, %2 ... %n）
                        if (hModule)
                        {
                            WCHAR szEventDesc[30240] = {0};

                            if (0 == FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE, 
                            hModule, 
                            pEventLogRecord->EventID,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                            szEventDesc,
                            30240,
                            NULL))
                        {
 
                             获得的参数序列：
                             WCHAR* pStrings = (LPWSTR) ((LPBYTE) pEventLogRecord + pEventLogRecord->StringOffset);
                             list<wstring> strValueList;    //获得的参数信息

                             for (UINT16 nIndex = 0; nIndex < pEventLogRecord->NumStrings; ++nIndex)
                             {
                                int nLen = (int)wcslen(pStrings);

                                if (nLen > 0)
                                {
                                    strValueList.push_back(pStrings);
                                }
                                    
                                pStrings += nLen + 1;
                             }

                            //再通过 FormatMessageW(FORMAT_MESSAGE_FROM_STRING, ...., 参数序列); 获得实际描述的内容
                        }
                    
                    4.实际的描述信息都为 UNICODE 字符
                 */
                //////////////////////////////////////////////////////////////////////////

                dwRead -= pEventLogRecord->Length; 

                sysEventLogInfoList.push_back(aTSysEventLogInfo);

                pEventLogRecord = (PEVENTLOGRECORD)((PCHAR)pEventLogRecord + pEventLogRecord->Length); 
            }
        }

        return CSysEventLog::ERROR_CODE_SUCCESS;
    }

    //////////////////////////////////////////////////////////////////////////
    //CSysEventLog

    CSysEventLog::CSysEventLog()
    {
        m_pImp = new(std::nothrow) CSysEventLogImp();
    }

    CSysEventLog::~CSysEventLog()
    {
        if (!m_pImp)
        {
            return;
        }

        CSysEventLogImp* pImp = (CSysEventLogImp*)m_pImp;

        delete pImp;

        m_pImp = NULL;
    }

    CSysEventLog::ERROR_CODE CSysEventLog::Open(const char* szSourceName)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSysEventLogImp* pImp = (CSysEventLogImp*)m_pImp;

        return pImp->Open(szSourceName);
    }

    void CSysEventLog::Close()
    {
        if (!m_pImp)
        {
            return;
        }

        CSysEventLogImp* pImp = (CSysEventLogImp*)m_pImp;

        pImp->Close();
    }

    CSysEventLog::ERROR_CODE CSysEventLog::GetCount(UINT32& recordCount)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSysEventLogImp* pImp = (CSysEventLogImp*)m_pImp;

        return pImp->GetCount(recordCount);
    }

    CSysEventLog::ERROR_CODE CSysEventLog::Read(list<TSysEventLogInfo>& sysEventLogInfoList)
    {
        if (!m_pImp)
        {
            return ERROR_CODE_SYSTEM;
        }

        CSysEventLogImp* pImp = (CSysEventLogImp*)m_pImp;

        return pImp->Read(sysEventLogInfoList);
    }
}