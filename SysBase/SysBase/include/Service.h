#pragma once

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //系统服务执行内容

    class IServiceHandler
    {
    public:

        IServiceHandler(){};
        virtual ~IServiceHandler(){};

        //事件:启动服务（这里启动相应的业务逻辑线程，不要在该事件中使用循环等待消息）
        //返回值: TRUE 进入消息等待 FALSE 退出
        virtual bool OnRun(int nArgc, char** argv) = 0;

        //事件: 接收到服务停止后
        virtual void OnStop() = 0;

        //事件：主线程处理(OnRun事件后，OnStop事件前 触发的时间，当 bProcThread 参数为 true 时有效)
        virtual void OnProc(){};

        //事件：当 OnProc 触发时接收到服务管理器提交的停止命令时响应的处理
        virtual void OnProcStop(){};
    };

    //////////////////////////////////////////////////////////////////////////
    //系统服务类

	//////////////////////////////////////////////////////////////////////////
	//CService
	
    class CService : CObject
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_SUCCESS = 0,
            ERROR_CODE_PARAM,
            ERROR_CODE_SYSTEM,			//系统级别错误
            ERROR_CODE_STOP,			//对象未启动或已停止
            ERROR_CODE_START,			//对象已启动
        };

        //////////////////////////////////////////////////////////////////////////

        CService();

        virtual ~CService();

        //功能:启动服务
        //参数:
        //      IServiceHandler* pIServiceHandler IN 服务处理细节
        //      const char* pServiceName IN 服务名称
        //      const char* pDisplayName IN 服务显示名称
        //      const char* pDesc IN 服务描述
        //      int nArgc IN 命令行参数个数
        //      char** argv IN 命令行参数信息
        //      bool bProcThread IN 是否自己管理主线程循环(true: IServiceHandler.OnProc 事件将被执行; false: 由CService对象托管主线程循环)
        //返回值：ERROR_CODE_SUCCESS 为成功，其余参考 ERROR_CODE 枚举
        ERROR_CODE Start(
            IServiceHandler* pIServiceHandler, 
            const char* pServiceName = "SYSBASE SRV", 
            const char* pDisplayName = "SYSBASE SRV", 
            const char* pDesc = "SYSBASE SRV",
            int nArgc = __argc, 
            char** argv = __argv,
            bool bProcThread = false);

    private:

        void* m_pImp;
    };
}