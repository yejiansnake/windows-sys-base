#include "DS_Config.h"

#define DS_CONFIG_APP_SYSTEM "System"
#define DS_CONFIG_KEY_SYSTDS_IP "IP"
#define DS_CONFIG_KEY_SYSTDS_PORT "Port"
#define DS_CONFIG_KEY_SYSTDS_MAX_CONNECT_COUNT "MaxConnectCount"
#define DS_CONFIG_KEY_SYSTDS_RECV_BUFFER_SIZE "RecvBufferSize"
#define DS_CONFIG_KEY_SYSTDS_RECV_BUFFER_COUNT "RecvBufferCount"
#define DS_CONFIG_KEY_SYSTDS_SEND_BUFFER_SIZE "SendBufferSize"
#define DS_CONFIG_KEY_SYSTDS_SEND_BUFFER_COUNT "SendBufferCount"
#define DS_CONFIG_KEY_SYSTDS_KEEP_ALIVE_TIME_SECOND "KeepAliveTimeSecond"
#define DS_CONFIG_KEY_SYSTDS_LOG_LEVEL "LogLevel"
#define DS_CONFIG_KEY_SYSTDS_IS_ON_STAT_LOG "IsOnStatLog"
#define DS_CONFIG_KEY_SYSTDS_WORK_THREAD_COUNT "WorkThreadCount"
#define DS_CONFIG_KEY_SYSTDS_RELOAD_TIME_SECOND "ReloadTimeSecond"

namespace EM
{
    CConfig::CConfig()
    {
        char szFilePath[MAX_PATH] = {0};
        ZeroMemory(szFilePath, MAX_PATH);

        CAppInfo appInfo;

        sprintf_s(szFilePath, MAX_PATH, "%s\\Config\\system.ini", appInfo.GetAppDirectory());

        m_config.Load(szFilePath);
    }

    CConfig::~CConfig()
    {

    }

    void CConfig::WriteErrorLog(PCHAR szApp, PCHAR szKey)
    {
        LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, "CConfig::Load get config %s:%s failed !", szApp, szKey);
    }

    bool CConfig::Load()
    {
        CHAR szTemp[1024] = {0};
        int nTemp = 0;

        //////////////////////////////////////////////////////////////////////////

        if (0 == m_config.GetString(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_IP, m_szLocalIP, 50, ""))
        {
            sprintf_s(m_szLocalIP, "0.0.0.0");
        }

        m_nPort = (UINT16)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_PORT, 0);

        if (0 == m_nPort)
        {
            this->WriteErrorLog(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_PORT);

            return false;
        }

        m_nMaxConnectCount = (UINT16)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_MAX_CONNECT_COUNT, 0);

        if (0 == m_nMaxConnectCount)
        {
            this->WriteErrorLog(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_MAX_CONNECT_COUNT);

            return false;
        }
        
        m_nRecvBufferSize = (UINT32)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_RECV_BUFFER_SIZE, 0);

        if (0 == m_nRecvBufferSize)
        {
            this->WriteErrorLog(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_RECV_BUFFER_SIZE);

            return false;
        }

        m_nRescBufferCount = (UINT32)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_RECV_BUFFER_COUNT, 0);

        if (0 == m_nRescBufferCount)
        {
            this->WriteErrorLog(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_RECV_BUFFER_SIZE);

            return false;
        }

        m_nSendBufferSize = (UINT32)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_SEND_BUFFER_SIZE, 0);

        if (0 == m_nSendBufferSize)
        {
            this->WriteErrorLog(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_SEND_BUFFER_SIZE);

            return false;
        }

        m_nSendBufferCount = (UINT32)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_SEND_BUFFER_COUNT, 0);

        if (0 == m_nSendBufferCount)
        {
            this->WriteErrorLog(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_SEND_BUFFER_COUNT);

            return false;
        }

        m_nKeepAliveTimeSecond = (UINT32)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_KEEP_ALIVE_TIME_SECOND, 0);

        if (0 == m_nKeepAliveTimeSecond)
        {
            m_nKeepAliveTimeSecond = 30;

            return false;
        }

        m_nLogLevel = (UINT8)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_LOG_LEVEL, 0);

        if (m_nLogLevel > 5)
        {
            m_nLogLevel = 5;
        }

        m_bIsOnStatLog = m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_LOG_LEVEL, 0) != 0 ? true : false;

        m_nWorkThreadCount = (UINT32)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_WORK_THREAD_COUNT, 0);

        if (0 == m_nWorkThreadCount)
        {
            this->WriteErrorLog(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_WORK_THREAD_COUNT);

            return false;
        }

        m_nReloadTimeSecond = (UINT32)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_RELOAD_TIME_SECOND, 0);

        //////////////////////////////////////////////////////////////////////////
        //业务相关

        return true;
    }

    void CConfig::Reload()
    {
        m_nLogLevel = (UINT8)m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_LOG_LEVEL, 0);

        if (0 == m_nLogLevel)
        {
            if (m_nLogLevel > 5)
            {
                m_nLogLevel = 5;
            }
        }

        m_bIsOnStatLog = m_config.GetInt(DS_CONFIG_APP_SYSTEM, DS_CONFIG_KEY_SYSTDS_LOG_LEVEL, 0) != 0 ? true : false;
    }

    const char* CConfig::GetFilePath()
    {
        return m_config.GetFilePath();
    }

    PCHAR CConfig::GetLocalIP()
    {
        return m_szLocalIP;
    }

    UINT16 CConfig::GetPort()
    {
        return m_nPort;
    }
    
    UINT16 CConfig::GetMaxConnectCount()
    {
        return m_nMaxConnectCount;
    }

    UINT32 CConfig::GetRecvBufferSize()
    {
        return m_nRecvBufferSize;
    }

    UINT32 CConfig::GetRecvBufferCount()
    {
        return m_nRescBufferCount;
    }

    UINT32 CConfig::GetSendBufferSize()
    {
        return m_nSendBufferSize;
    }

    UINT32 CConfig::GetSendBufferCount()
    {
        return m_nSendBufferCount;
    }

    UINT32 CConfig::GetKeeAliveTimeSecond()
    {
        return m_nKeepAliveTimeSecond;
    }

    UINT8 CConfig::GetLogLevel()
    {
        return m_nLogLevel;
    }

    bool CConfig::GetIsOnStatLog()
    {
        return m_bIsOnStatLog;
    }

    UINT8 CConfig::GetWorkThreadCount()
    {
        return m_nWorkThreadCount;
    }

    UINT32 CConfig::GetReloadTimeSecond()
    {
        return m_nReloadTimeSecond;
    }
}