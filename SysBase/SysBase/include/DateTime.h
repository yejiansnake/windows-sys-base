#pragma once

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CDateTime

    class CDateTime
    {
    public:

        //默认为当前时间(Now)
        CDateTime();

        CDateTime(UINT16 nYear, UINT16 nMonth, UINT16 nDay);

        CDateTime(UINT16 nYear, UINT16 nMonth, UINT16 nDay, UINT16 nHour, UINT16 nMinute, UINT16 nSecond);

        CDateTime(UINT16 nYear, UINT16 nMonth, UINT16 nDay, UINT16 nHour, UINT16 nMinute, UINT16 nSecond, UINT16 nMillisecond);

        CDateTime(INT64 nTime, UINT16 nMillisecond);

        CDateTime(SYSTEMTIME systemTime);

        virtual ~CDateTime();

        UINT16 GetYear();
        UINT16 GetMonth();
        UINT16 GetDay();
        UINT16 GetDayOfWeek();
        UINT16 GetDayOfYear();
        UINT16 GetHour();
        UINT16 GetMinute();
        UINT16 GetSecond();
        UINT16 GetMillisecond();

        INT64 GetTime();

    protected:

        void Init(UINT16 nYear = 0, 
            UINT16 nMonth = 0, 
            UINT16 nDay = 0,
            UINT16 nHour = 0, 
            UINT16 nMinute = 0,
            UINT16 nSecond = 0, 
            UINT16 nMillisecond = 0);

    private:

        UINT16 m_nYear;             //年 since 1900
        UINT16 m_nMonth;            //月 [1,12]
        UINT16 m_nDay;              //日期 [1,31]
        UINT16 m_nDayOfWeek;        //一周中的第几天 since Sunday - [0,6]
        UINT16 m_nDayOfYear;        //一年中的第几天 since January 1 - [0,365]
        UINT16 m_nHour;             //小时 [0,23]
        UINT16 m_nMinute;           //分钟 [0,59]
        UINT16 m_nSecond;           //秒 [0,59]
        UINT16 m_nMillisecond;      //毫秒

        INT64 m_nTime;             //距离1970年1月1日0时0分0秒 的总秒数 ( UTC 时间 ) 
    };
}
