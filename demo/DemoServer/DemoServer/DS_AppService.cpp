#include "DS_AppService.h"
#include "DS_Config.h"
#include "DS_Log.h"
#include "DS_StatisticsThread.h"
#include "DS_WorkThread.h"
#include "DS_NetServer.h"
#include "DS_ReloadThread.h"
#include "DS_ConfigMoniter.h"

namespace EM
{
    CAppService::CAppService()
    {
        CLogLoader::Init();
    }

    CAppService::~CAppService()
    {

    }

    bool CAppService::InitLib()
    {
        if (!CSocketLoader::InitSocket())
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, "CAppService::InitLib init socket lib failed !");

            return false;
        }

        return true;
    }

    bool CAppService::LoadConfig()
    {
        if (!CConfig::Instance().Load())
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, "CAppService::LoadConfig load config failed !");

            return false;
        }

        return true;
    }

    bool CAppService::InitAppConfig()
    {
        CLogLoader::ResetLogLevelFromConfig();

        return true;
    }

    bool CAppService::StartApp()
    {
        //////////////////////////////////////////////////////////////////////////
        //启动统计线程

        if (!CStatisticsThread::Instance().Start())
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, " CAppService::StartApp Start CStatisticsThread failed !");

            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //启动重载线程（定时加载一些配置数据等）

        if (!CReloadThread::Instance().Start())
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, " CAppService::StartApp Start CReloadThread failed !");

            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //启动配置文件监控

        if (!CConfigMoniter::Instance().Start())
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, " CAppService::StartApp Start CConfigMoniter failed !");

            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //启动工作线程

        if (!CWorkThread::Instance().Start())
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, " CAppService::StartApp Start CWorkThread failed !");

            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //启动收发数据模块

        if (!CNetServer::Instance().Start())
        {
            LOG_WRITE(LOG_KEY_SYSTEM, CLog::LOG_LEVEL_ERROR, " CAppService::StartApp Start CNetServer failed !");

            return false;
        }

        return true;
    }

    void CAppService::StopApp()
    {
        CNetServer::Instance().Stop();

        CWorkThread::Instance().Stop();

        CConfigMoniter::Instance().Stop();

        CReloadThread::Instance().Stop();

        CStatisticsThread::Instance().Stop();
    }

    bool CAppService::OnRun(int nArgc, char** argv)
    {
        //////////////////////////////////////////////////////////////////////////
        //初始化引用库

        if (!this->InitLib())
        {
            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //加载配置

        if (!this->LoadConfig())
        {
            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //APP 初始化各种参数配置

        if (!this->InitAppConfig())
        {
            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //APP 启动各个模块

        if (!this->StartApp())
        {
            return false;
        }

        return true;
    }

    void CAppService::OnStop()
    {
        this->StopApp();
    }
}