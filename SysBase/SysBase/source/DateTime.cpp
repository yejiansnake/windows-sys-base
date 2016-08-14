#include "../SysBase_Interface.h"
#include <sys/timeb.h>
#include <time.h>

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CDateTime

    CDateTime::CDateTime()
    {
        __timeb64 aTimeb64;

        _ftime64_s(&aTimeb64);

        struct tm tmLocalTime = {0};
        _localtime64_s(&tmLocalTime, &aTimeb64.time);

        m_nYear = tmLocalTime.tm_year + 1900;
        m_nMonth = tmLocalTime.tm_mon + 1;
        m_nDay = tmLocalTime.tm_mday;
        m_nDayOfWeek = tmLocalTime.tm_wday;
        m_nDayOfYear = tmLocalTime.tm_yday;
        m_nHour = tmLocalTime.tm_hour;
        m_nMinute = tmLocalTime.tm_min;
        m_nSecond = tmLocalTime.tm_sec;
        m_nMillisecond = aTimeb64.millitm;

        m_nTime = aTimeb64.time;
    }

    CDateTime::CDateTime(UINT16 nYear, UINT16 nMonth, UINT16 nDay) 
    {
        this->Init(nYear, nMonth, nDay);
    }

    CDateTime::CDateTime(UINT16 nYear, UINT16 nMonth, UINT16 nDay, UINT16 nHour, UINT16 nMinute, UINT16 nSecond) 
    {
        this->Init(nYear, nMonth, nDay, nHour, nMinute, nSecond);
    }

    CDateTime::CDateTime(UINT16 nYear, UINT16 nMonth, UINT16 nDay, UINT16 nHour, UINT16 nMinute, UINT16 nSecond, UINT16 nMillisecond)
    {
        this->Init(nYear, nMonth, nDay, nHour, nMinute, nSecond, nMillisecond);
    }

    CDateTime::CDateTime(INT64 nTime, UINT16 nMillisecond)
    {
        struct tm tmLocalTime = {0};
        _localtime64_s(&tmLocalTime, &nTime);

        m_nYear = tmLocalTime.tm_year + 1900;
        m_nMonth = tmLocalTime.tm_mon + 1;
        m_nDay = tmLocalTime.tm_mday;
        m_nDayOfWeek = tmLocalTime.tm_wday;
        m_nDayOfYear = tmLocalTime.tm_yday;
        m_nHour = tmLocalTime.tm_hour;
        m_nMinute = tmLocalTime.tm_min;
        m_nSecond = tmLocalTime.tm_sec;
        m_nMillisecond = nMillisecond;

        m_nTime = nTime;
    }

    CDateTime::CDateTime(SYSTEMTIME systemTime)
    {
        this->Init(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
    }

    CDateTime::~CDateTime()
    {

    }

    void CDateTime::Init(UINT16 nYear, UINT16 nMonth, UINT16 nDay, UINT16 nHour, UINT16 nMinute, UINT16 nSecond, UINT16 nMillisecond)
    {
        struct tm tmLocalTime = {0};
        tmLocalTime.tm_year = nYear - 1900;
        tmLocalTime.tm_mon = nMonth - 1;
        tmLocalTime.tm_mday = nDay;
        tmLocalTime.tm_hour = nHour;
        tmLocalTime.tm_min = nMinute;
        tmLocalTime.tm_sec = nMinute;

        m_nTime = _mktime64(&tmLocalTime);

        _localtime64_s(&tmLocalTime, &m_nTime);

        m_nYear = tmLocalTime.tm_year + 1900;
        m_nMonth = tmLocalTime.tm_mon + 1;
        m_nDay = tmLocalTime.tm_mday;
        m_nDayOfWeek = tmLocalTime.tm_wday;
        m_nDayOfYear = tmLocalTime.tm_yday;
        m_nHour = tmLocalTime.tm_hour;
        m_nMinute = tmLocalTime.tm_min;
        m_nSecond = tmLocalTime.tm_sec;
        m_nMillisecond = nMillisecond;
    }

    UINT16 CDateTime::GetYear()
    {
        return m_nYear;
    }

    UINT16 CDateTime::GetMonth()
    {
        return m_nMonth;
    }

    UINT16 CDateTime::GetDay()
    {
        return m_nDay;
    }

    UINT16 CDateTime::GetDayOfWeek()
    {
        return m_nDayOfWeek;
    }

    UINT16 CDateTime::GetDayOfYear()
    {
        return m_nDayOfYear;
    }

    UINT16 CDateTime::GetHour()
    {
        return m_nHour;
    }

    UINT16 CDateTime::GetMinute()
    {
        return m_nMinute;
    }

    UINT16 CDateTime::GetSecond()
    {
        return m_nSecond;
    }

    UINT16 CDateTime::GetMillisecond()
    {
        return m_nMillisecond;
    }

    INT64 CDateTime::GetTime()
    {
        return m_nTime;
    }
}