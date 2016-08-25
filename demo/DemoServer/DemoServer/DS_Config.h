#pragma once

#include "DS_Common.h"

namespace EM
{
    class CConfig : public CSingletonT<CConfig>
    {
    public:

        CConfig();

        virtual ~CConfig();

        bool Load();

        void Reload();

        const char* GetFilePath();

        //////////////////////////////////////////////////////////////////////////
        //System

        PCHAR GetLocalIP();

        UINT16 GetPort();

        UINT16 GetMaxConnectCount();

        UINT32 GetRecvBufferSize();

        UINT32 GetRecvBufferCount();

        UINT32 GetSendBufferSize();

        UINT32 GetSendBufferCount();

        UINT32 GetKeeAliveTimeSecond();

        UINT8 GetLogLevel();

        bool GetIsOnStatLog();

        UINT8 GetWorkThreadCount();

        UINT32 GetReloadTimeSecond();

        //////////////////////////////////////////////////////////////////////////
        //业务相关的

    protected:

        void WriteErrorLog(PCHAR szApp, PCHAR szKey);

    private:

        //////////////////////////////////////////////////////////////////////////

        CIniConfig m_config;

        //////////////////////////////////////////////////////////////////////////
        //System

        CHAR m_szLocalIP[50];

        UINT16 m_nPort;

        UINT16 m_nMaxConnectCount;

        UINT32 m_nRecvBufferSize;

        UINT32 m_nRescBufferCount;

        UINT32 m_nSendBufferSize;

        UINT32 m_nSendBufferCount;

        UINT32 m_nKeepAliveTimeSecond;

        UINT8 m_nLogLevel;

        bool m_bIsOnStatLog;

        UINT8 m_nWorkThreadCount;

        UINT32 m_nReloadTimeSecond;
    };
}