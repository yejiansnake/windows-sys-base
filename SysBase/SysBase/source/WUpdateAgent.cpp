#include "../SysBase_Interface.h"

#define SYSBASE_WINDOWS_WUA_NAME L"wuauserv"

namespace SysBase
{
    CWUpdateAgent::CWUpdateAgent()
    {
        ::CoInitialize(NULL);

        m_pIAutomaticUpdates = NULL;

        HRESULT hr = CoCreateInstance(
            CLSID_AutomaticUpdates,
            NULL,
            CLSCTX_INPROC,
            IID_IAutomaticUpdates, 
            (LPVOID*)&m_pIAutomaticUpdates);
    }

    CWUpdateAgent::~CWUpdateAgent()
    {
        if (!m_pIAutomaticUpdates)
        {
            return;
        }

        m_pIAutomaticUpdates->Release();

        m_pIAutomaticUpdates = NULL;

        ::CoUninitialize();
    }

    CWUpdateAgent::ERROR_CODE CWUpdateAgent::GetServiceStatus(CWUpdateAgent::SERVICE_STATUS& serviceStatus)
    {
        TServiceInfoW aInfo;

        if (CServicesHelper::ERROR_CODE_SUCCESS != CServicesHelper::GetServiceInfo(SYSBASE_WINDOWS_WUA_NAME, aInfo))
        {
            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        if (SYSBASE_SERVICE_STATE_RUNNING == aInfo.curState)
        {
            serviceStatus = CWUpdateAgent::SERVICE_STATUS_ON;
        }
        else
        {
            serviceStatus = CWUpdateAgent::SERVICE_STATUS_OFF;
        }

        return CWUpdateAgent::ERROR_CODE_SUCCESS;
    }

    CWUpdateAgent::ERROR_CODE CWUpdateAgent::EnableService()
    {
        if (!m_pIAutomaticUpdates)
        {
            return CWUpdateAgent::ERROR_CODE_INIT_FAILED;
        }

        HRESULT hr = m_pIAutomaticUpdates->EnableService();
        if (S_FALSE == hr)
        {
            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        return CWUpdateAgent::ERROR_CODE_SUCCESS;
    }

    CWUpdateAgent::ERROR_CODE CWUpdateAgent::DiableService()
    {
        if (CServicesHelper::ERROR_CODE_SUCCESS != CServicesHelper::StopService(SYSBASE_WINDOWS_WUA_NAME))
        {
            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        return CWUpdateAgent::ERROR_CODE_SUCCESS;
    }

    CWUpdateAgent::ERROR_CODE CWUpdateAgent::GetNotiyType(
        CWUpdateAgent::NOTIFY_TYPE& notifyType, 
        CWUpdateAgent::AUTO_UPDATE_DAY* pAutoUpdateDay, 
        INT16* pnAutoUpdateTime)
    {
        if (!m_pIAutomaticUpdates)
        {
            return CWUpdateAgent::ERROR_CODE_INIT_FAILED;
        }

        IAutomaticUpdatesSettings* pIAutomaticUpdatesSettings = NULL;

        HRESULT hr = m_pIAutomaticUpdates->get_Settings(&pIAutomaticUpdatesSettings);

        if (S_FALSE == hr)
        {
            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        AutomaticUpdatesNotificationLevel retval = aunlNotConfigured;

        hr = pIAutomaticUpdatesSettings->get_NotificationLevel(&retval);

        if (aunlScheduledInstallation == retval)
        {
            if (pAutoUpdateDay)
            {
                AutomaticUpdatesScheduledInstallationDay autoDay;

                if (S_OK == pIAutomaticUpdatesSettings->get_ScheduledInstallationDay(&autoDay))
                {
                    switch (autoDay)
                    {
                    case ausidEveryDay:
                        {
                            *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_EVERY;
                        }
                        break;
                    case ausidEverySunday:
                        {
                            *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_SUNDAY;
                        }
                        break;
                    case ausidEveryMonday:
                        {
                            *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_MONDAY;
                        }
                        break;
                    case ausidEveryTuesday:
                        {
                            *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_TUESDAY;
                        }
                        break;
                    case ausidEveryWednesday:
                        {
                            *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_WEDNESDAY; 
                        }
                        break;
                    case ausidEveryThursday:
                        {
                            *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_THURSDAY;
                        }
                        break;
                    case ausidEveryFriday:
                        {
                            *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_FRIDAY;
                        }
                        break;
                    case ausidEverySaturday:
                        {
                            *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_SATURDAY;
                        }
                        break;
                    default:
                        *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_NONE;
                        break;
                    }
                }
                else
                {
                    *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_NONE;
                }
            }

            if (pnAutoUpdateTime)
            {
                long autoTime;

                if (S_OK == pIAutomaticUpdatesSettings->get_ScheduledInstallationTime(&autoTime))
                {
                    *pnAutoUpdateTime = (INT16)autoTime;
                }
                else
                {
                    *pnAutoUpdateTime = -1;
                }
            }
        }
        else
        {
            if (pAutoUpdateDay)
            {
                *pAutoUpdateDay = CWUpdateAgent::AUTO_UPDATE_NONE;
            }

            if (pnAutoUpdateTime)
            {
                *pnAutoUpdateTime = -1;
            }
        }

        pIAutomaticUpdatesSettings->Release();

        pIAutomaticUpdatesSettings =NULL;

        if (S_FALSE == hr)
        {
            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        switch (retval)
        {
        case aunlNotConfigured:
            {
                notifyType = CWUpdateAgent::NOTIFY_TYPE_NOT_CONFIGURED;
            }
            break;
        case aunlDisabled:
            {
                notifyType = CWUpdateAgent::NOTIFY_TYPE_DISABLED;
            }
            break;
        case aunlNotifyBeforeDownload:
            {
                notifyType = CWUpdateAgent::NOTIFY_TYPE_BEFORE_DOWNLOAD;
            }
            break;
        case aunlNotifyBeforeInstallation:
            {
                notifyType = CWUpdateAgent::NOTIFY_TYPE_BEFORE_INSTALL;
            }
            break;
        case aunlScheduledInstallation:
            {
                notifyType = CWUpdateAgent::NOTIFY_TYPE_SCHEDULED_INSTALL;
            }
            break;
        default:
            notifyType = CWUpdateAgent::NOTIFY_TYPE_NOT_CONFIGURED;
            break;
        }

        return CWUpdateAgent::ERROR_CODE_SUCCESS;
    }

    CWUpdateAgent::ERROR_CODE CWUpdateAgent::SetNotiyType(
        CWUpdateAgent::NOTIFY_TYPE notifyType, 
        CWUpdateAgent::AUTO_UPDATE_DAY* pAutoUpdateDay, 
        INT16* pnAutoUpdateTime)
    {
        if (!m_pIAutomaticUpdates)
        {
            return CWUpdateAgent::ERROR_CODE_INIT_FAILED;
        }

        IAutomaticUpdatesSettings* pIAutomaticUpdatesSettings = NULL;

        HRESULT hr = m_pIAutomaticUpdates->get_Settings(&pIAutomaticUpdatesSettings);

        if (S_FALSE == hr)
        {
            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        AutomaticUpdatesNotificationLevel retval = aunlNotConfigured;

        switch (notifyType)
        {
        case CWUpdateAgent::NOTIFY_TYPE_NOT_CONFIGURED:
            {
                retval = aunlNotConfigured;
            }
            break;
        case CWUpdateAgent::NOTIFY_TYPE_DISABLED:
            {
                retval = aunlDisabled;
            }
            break;
        case CWUpdateAgent::NOTIFY_TYPE_BEFORE_DOWNLOAD:
            {
                retval = aunlNotifyBeforeDownload;
            }
            break;
        case CWUpdateAgent::NOTIFY_TYPE_BEFORE_INSTALL:
            {
                retval = aunlNotifyBeforeInstallation;
            }
            break;
        case CWUpdateAgent::NOTIFY_TYPE_SCHEDULED_INSTALL:
            {
                retval = aunlScheduledInstallation;
            }
            break;
        default:
            return CWUpdateAgent::ERROR_CODE_PARAM;
        }

        hr = pIAutomaticUpdatesSettings->put_NotificationLevel(retval);

        if (aunlScheduledInstallation == retval)
        {
            if (pAutoUpdateDay)
            {
                AutomaticUpdatesScheduledInstallationDay autoDay;

                switch (*pAutoUpdateDay)
                {
                case CWUpdateAgent::AUTO_UPDATE_EVERY :
                    {
                        autoDay = ausidEveryDay;
                    }
                    break;
                case CWUpdateAgent::AUTO_UPDATE_SUNDAY:
                    {
                        autoDay = ausidEverySunday;
                    }
                    break;
                case CWUpdateAgent::AUTO_UPDATE_MONDAY:
                    {
                        autoDay = ausidEveryMonday;
                    }
                    break;
                case CWUpdateAgent::AUTO_UPDATE_TUESDAY:
                    {
                        autoDay = ausidEveryTuesday;
                    }
                    break;
                case CWUpdateAgent::AUTO_UPDATE_WEDNESDAY:
                    {
                        autoDay = ausidEveryWednesday; 
                    }
                    break;
                case CWUpdateAgent::AUTO_UPDATE_THURSDAY:
                    {
                        autoDay = ausidEveryThursday;
                    }
                    break;
                case CWUpdateAgent::AUTO_UPDATE_FRIDAY:
                    {
                        autoDay = ausidEveryFriday;
                    }
                    break;
                case CWUpdateAgent::AUTO_UPDATE_SATURDAY:
                    {
                        autoDay = ausidEverySaturday;
                    }
                    break;
                default:
                    return CWUpdateAgent::ERROR_CODE_PARAM;
                }

                pIAutomaticUpdatesSettings->put_ScheduledInstallationDay(autoDay);
            }

            if (pnAutoUpdateTime)
            {
                long autoTime = *pnAutoUpdateTime;
                pIAutomaticUpdatesSettings->put_ScheduledInstallationTime(autoTime);
            }
        }

        pIAutomaticUpdatesSettings->Save();

        pIAutomaticUpdatesSettings->Release();

        pIAutomaticUpdatesSettings = NULL;

        if (S_FALSE == hr)
        {
            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        return CWUpdateAgent::ERROR_CODE_SUCCESS;
    }

    CWUpdateAgent::ERROR_CODE CWUpdateAgent::GetLastUpdateTime(bool bHasUpdate, UINT64& nTime)
    {
        IUpdateSession* pIUpdateSession = NULL;
        IUpdateSearcher* pIUpdateSearcher = NULL;
        IUpdateHistoryEntryCollection* pIUpdateHistoryEntryCollection = NULL;
        IUpdateHistoryEntry* pIUpdateHistoryEntry = NULL;

        HRESULT hr = CoCreateInstance(
            CLSID_UpdateSession,
            NULL,
            CLSCTX_INPROC,
            IID_IUpdateSession, 
            (LPVOID*)&pIUpdateSession);

        if (S_FALSE == hr)
        {
            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        hr = pIUpdateSession->CreateUpdateSearcher(&pIUpdateSearcher);

        if (S_FALSE == hr)
        {
            pIUpdateSession->Release();

            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        LONG lCount = 0;

        hr = pIUpdateSearcher->GetTotalHistoryCount(&lCount);

        if (S_FALSE == hr)
        {
            pIUpdateSearcher->Release();
            pIUpdateSession->Release();

            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        if (0 == lCount)
        {
            pIUpdateSearcher->Release();
            pIUpdateSession->Release();

            bHasUpdate = false;
            nTime = 0;

            return CWUpdateAgent::ERROR_CODE_SUCCESS;
        }

        hr = pIUpdateSearcher->QueryHistory(0, 1, &pIUpdateHistoryEntryCollection);

        if (S_FALSE == hr)
        {
            pIUpdateSearcher->Release();
            pIUpdateSession->Release();

            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        hr = pIUpdateHistoryEntryCollection->get_Item(0, &pIUpdateHistoryEntry);

        if (S_FALSE == hr)
        {
            pIUpdateHistoryEntryCollection->Release();
            pIUpdateSearcher->Release();
            pIUpdateSession->Release();

            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        DATE lastDate = 0;
        hr = pIUpdateHistoryEntry->get_Date(&lastDate);

        if (S_FALSE == hr)
        {
            pIUpdateHistoryEntry->Release();
            pIUpdateHistoryEntryCollection->Release();
            pIUpdateSearcher->Release();
            pIUpdateSession->Release();

            return CWUpdateAgent::ERROR_CODE_SYSTEM;
        }

        pIUpdateHistoryEntry->Release();
        pIUpdateHistoryEntryCollection->Release();
        pIUpdateSearcher->Release();
        pIUpdateSession->Release();

        bHasUpdate = true;

        //第一次获得的是UTC时间，需要转换为本地时间
        SYSTEMTIME systemTime = {0};

        VariantTimeToSystemTime(lastDate, &systemTime);

        FILETIME fileTime = {0};
        SystemTimeToFileTime(&systemTime, &fileTime);

        FILETIME localfileTime = {0};
        FileTimeToLocalFileTime(&fileTime, &localfileTime);

        FileTimeToSystemTime(&localfileTime, &systemTime);

        CDateTime aCDateTime(systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
            systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);

        nTime = aCDateTime.GetTime();

        return CWUpdateAgent::ERROR_CODE_SUCCESS;
    }

    bool CWUpdateAgent::GetWSUSUrl(string& strUrl)
    {
        char szRegPath[] = "SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate";
        char szBuffer[1024] = {0};
        DWORD dwBufferSize = 1024;
        HKEY RegKey = NULL;

        // 由注册表中取得各项 CPU 信息
        if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_QUERY_VALUE, &RegKey))
        {
            return false;
        }

        if (ERROR_SUCCESS != RegQueryValueExA(RegKey, "WUServer", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize))
        {
            return false;
        }

        szBuffer[1023] = NULL;

        strUrl = szBuffer;

        return true;
    }
}