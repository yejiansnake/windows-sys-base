#pragma once

#include "DS_Common.h"

namespace EM
{
    class CReloadThread : public CSingletonT<CReloadThread>, public CCustomThread
    {
    public:

        CReloadThread();

        virtual ~CReloadThread();

        //功能：启动
        //返回值：TRUE 成功；FALSE 失败
        bool Start();

        //功能：停止
        void Stop();

        //功能：马上重载
        void ReloadNow();

    protected:

        virtual void Proc();

    private:

        bool m_bStop;

        CEvent m_StopEvent;

        CAutoEvent m_ReLoadEvent;
    };
}
