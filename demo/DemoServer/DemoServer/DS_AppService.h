#pragma once

#include "DS_Common.h"

namespace EM
{
    class CAppService : public SysBase::IServiceHandler
    {
    public:

        CAppService();

        virtual ~CAppService();

    protected:

        bool InitLib();

        bool LoadConfig();

        bool InitAppConfig();

        bool StartApp();

        void StopApp();

        //事件:启动服务（这里启动相应的业务逻辑线程，不要在该事件中使用循环等待消息）
        //返回值: TRUE 进入消息等待 FALSE 退出
        virtual bool OnRun(int nArgc, char** argv);

        //事件: 接收到服务停止后
        virtual void OnStop();
    };
}