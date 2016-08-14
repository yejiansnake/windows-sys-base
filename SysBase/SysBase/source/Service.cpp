#include "../SysBase_Interface.h"
#include "NTService.h"

#define WIN_SERVICE_STOP_PENDING_WAIT_TIME 10000

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CServiceImp

    class CServiceImp : private CNTService
    {
    public:

        //异常
        CServiceImp(
            IServiceHandler* pIServiceHandler, 
            const char* pServiceName = "SYSBASE SRV", 
            const char* pDisplayName = "SYSBASE SRV", 
            const char* pDesc = "SYSBASE SRV");

        virtual ~CServiceImp();

        bool Start(int nArgc = __argc, char** argv = __argv, bool bProcThread = false);

    protected:

        //////////////////////////////////////////////////////////////////////////

        void Run(DWORD argc, LPSTR*  argv);

        void Stop();

    private:

        CEvent m_StopEvent;

        IServiceHandler* m_pIServiceHandler;

        bool m_bProcThread;
    };

	//////////////////////////////////////////////////////////////////////////
	//CWinService

    CServiceImp::CServiceImp(
		IServiceHandler* pIServiceHandler, 
		const char* pServiceName, 
		const char* pDisplayName, 
		const char* pDesc) 
        : CNTService(pServiceName, pDisplayName, pDesc), m_StopEvent(FALSE)
    {
        m_pIServiceHandler = pIServiceHandler;
    }

    CServiceImp::~CServiceImp()
    {
		this->Stop();
    }

    bool CServiceImp::Start(int nArgc, char** argv, bool bProcThread)
    {
        m_bProcThread = bProcThread;

        //启动服务器
        return CNTService::RegisterService(nArgc, argv) ? true : false;
    }

    void CServiceImp::Run(DWORD argc, LPSTR* argv)
    {
        ReportStatus(SERVICE_START_PENDING);

        if (m_pIServiceHandler->OnRun(argc, argv))
        {
            ReportStatus(SERVICE_RUNNING);

            if (m_bProcThread)
            {
                m_pIServiceHandler->OnProc();
            }
            else
            {
                HANDLE hStopEvent = m_StopEvent.GetHandle();

                //消息循环
                while (WaitForSingleObject(hStopEvent, 10) != WAIT_OBJECT_0 ) 
                {
                }
            }

            ReportStatus(SERVICE_STOP_PENDING, WIN_SERVICE_STOP_PENDING_WAIT_TIME);
        }
        
        m_pIServiceHandler->OnStop();
    }

    void CServiceImp::Stop()
    {
        if (m_bProcThread)
        {
            m_pIServiceHandler->OnProcStop();
        }
        else
        {
            m_StopEvent.Set();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //CService

    CService::CService()
    {
        m_pImp = NULL;
    }

    CService::~CService()
    {
        if (m_pImp)
        {
            CServiceImp* pCServiceImp = (CServiceImp*)m_pImp;

            delete pCServiceImp;

            m_pImp = NULL;
        }
    }

    CService::ERROR_CODE CService::Start(
        IServiceHandler* pIServiceHandler, 
        const char* pServiceName, 
        const char* pDisplayName, 
        const char* pDesc,
        int nArgc, 
        char** argv,
        bool bProcThread)
    {
        if (m_pImp)
        {
            return CService::ERROR_CODE_START;
        }

        if (!pIServiceHandler
            || !pServiceName)
        {
            return CService::ERROR_CODE_PARAM;
        }

        if (pServiceName[0] == NULL)
        {
            return CService::ERROR_CODE_PARAM;
        }

        if (!pDisplayName)
        {
            pDisplayName = pServiceName;
        }

        if (!pDesc)
        {
            pDesc = pServiceName;
        }

        CServiceImp* pCServiceImp = new(std::nothrow) CServiceImp(pIServiceHandler,
            pServiceName, pDisplayName, pDesc);

        if (!pCServiceImp)
        {
            return CService::ERROR_CODE_SYSTEM;
        }

        if (!pCServiceImp->Start(nArgc, argv, bProcThread))
        {
            delete pCServiceImp;

            pCServiceImp = NULL;

            return CService::ERROR_CODE_SYSTEM;
        }

        m_pImp = pCServiceImp;

        return CService::ERROR_CODE_SUCCESS;
    }
}