#include "DS_Log.h"
#include "DS_Config.h"

namespace EM
{
    CLogLoader::CLogLoader()
    {

    }

    CLogLoader::~CLogLoader()
    {

    }

    void CLogLoader::Init()
    {
        CAppInfo appInfo;

        char szFilePath[MAX_PATH] = {0};

        sprintf_s(szFilePath, MAX_PATH, "%s\\Log\\System\\System.log", appInfo.GetAppDirectory());

        LOG_INIT(LOG_KEY_SYSTEM, szFilePath, CLog::LOG_LEVEL_INFO);

        LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_INFO, "CLogLoader::CLogLoader AppVersion:%d", APP_VER);

        sprintf_s(szFilePath, MAX_PATH, "%s\\Log\\Client\\Client.log", appInfo.GetAppDirectory());

        LOG_INIT(LOG_KEY_CLIENT, szFilePath, CLog::LOG_LEVEL_INFO);

        LOG_WRITE(LOG_KEY_CLIENT, CLog::LOG_LEVEL_INFO, "CLogLoader::CLogLoader AppVersion:%d", APP_VER);

        sprintf_s(szFilePath, MAX_PATH, "%s\\Log\\Statistics\\Statistics.log", appInfo.GetAppDirectory());

        LOG_INIT(LOG_KEY_STATISTICS, szFilePath, CLog::LOG_LEVEL_INFO);

        LOG_WRITE(LOG_KEY_STATISTICS, CLog::LOG_LEVEL_INFO, "CLogLoader::CLogLoader AppVersion:%d", APP_VER);
    }

    void CLogLoader::ResetLogLevelFromConfig()
    {
        LOG_SET_LEVEL(LOG_KEY_SYSTEM, (CLog::LOG_LEVEL)CConfig::Instance().GetLogLevel());

        if (CConfig::Instance().GetIsOnStatLog())
        {
            LOG_SET_LEVEL(LOG_KEY_STATISTICS, CLog::LOG_LEVEL_INFO);
        }
        else
        {
            LOG_SET_LEVEL(LOG_KEY_STATISTICS, CLog::LOG_LEVEL_NONE);
        }

        LOG_SET_LEVEL(LOG_KEY_CLIENT, (CLog::LOG_LEVEL)CConfig::Instance().GetLogLevel());
    }
}