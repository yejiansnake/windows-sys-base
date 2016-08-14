#include "../SysBase_Interface.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////

    IFileMonitorWorkerHandler::IFileMonitorWorkerHandler()
    {

    }

    IFileMonitorWorkerHandler::~IFileMonitorWorkerHandler()
    {

    }

    //////////////////////////////////////////////////////////////////////////

    void TFileMonitorWorkerParam::SetFileName(const char* szFileName)
    {
        MultiByteToWideChar(CP_ACP, 0, szFileName, -1, wzFilePath, MAX_PATH);
    }

    //////////////////////////////////////////////////////////////////////////

    CFileMonitorWorker::CFileMonitorWorker()
    {
        m_bStop = true;
    }

    CFileMonitorWorker::~CFileMonitorWorker()
    {
        this->Stop();
    }

    bool CFileMonitorWorker::Start(TFileMonitorWorkerParam& aParam)
    {
        if (!m_bStop)
        {
            return true;
        }

        if (0 == wcslen(aParam.wzFilePath))
        {
            return false;
        }

        if (!aParam.bIsMonitorAttributes && !aParam.bIsMonitorLastWrite && !aParam.bIsMonitorSize)
        {
            return false;
        }

        if (!aParam.pHandler)
        {
            return false;
        }

        m_param = aParam;

        m_bStop = false;
        m_stopEvent.Reset();

        if (CCustomThread::ERROR_CODE_SUCCESS != CCustomThread::Start())
        {
            this->Stop();

            return false;
        }

        return true;    
    }

    void CFileMonitorWorker::Stop()
    {
        if (m_bStop)
        {
            return;
        }

        m_bStop = true;

        m_stopEvent.Set();

        CCustomThread::Stop();
    }

    void CFileMonitorWorker::Proc()
    {
        WCHAR wzDir[MAX_PATH] = {0};
        WIN32_FILE_ATTRIBUTE_DATA fileData = {0};

        swprintf_s(wzDir,MAX_PATH, m_param.wzFilePath);

        WCHAR* pPtr = wcsrchr(wzDir, L'\\');

        if (pPtr)
        {
            *pPtr = NULL;
        }

        DWORD dwNotify = 0;

        if (m_param.bIsMonitorAttributes)
        {
            dwNotify |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
        }

        if (m_param.bIsMonitorLastWrite)
        {
            dwNotify |= FILE_NOTIFY_CHANGE_LAST_WRITE;
        }

        if (m_param.bIsMonitorSize)
        {
            dwNotify |= FILE_NOTIFY_CHANGE_SIZE;
        }

        HANDLE hChangeHandle = FindFirstChangeNotificationW(wzDir, FALSE, dwNotify);

        if (INVALID_HANDLE_VALUE == hChangeHandle)
        {
            return;
        }

        if (!GetFileAttributesExW(m_param.wzFilePath, GetFileExInfoStandard, (LPVOID)&fileData))
        {
            return;
        }

        const int nCount = 2;
        const int nStopEventIndex = 0;
        const int nChangeHandleIndex = 1;

        HANDLE hHandles[nCount] = {0};
        hHandles[nStopEventIndex] = (HANDLE)m_stopEvent.GetHandle();
        hHandles[nChangeHandleIndex] = hChangeHandle;

        while (TRUE)
        {
            switch (WaitForMultipleObjects(nCount, hHandles, FALSE, INFINITE) - WAIT_OBJECT_0)
            {
            case WAIT_FAILED :
            case nStopEventIndex :
                {
                    FindCloseChangeNotification(hChangeHandle);

                    return;
                }    
                break;
            case nChangeHandleIndex :
                {
                    bool bChangeSize = false;
                    bool bChangeAttributes = false;
                    bool bChangeLastWrite = false;
                    WIN32_FILE_ATTRIBUTE_DATA tempfileData = {0};

                    if (!GetFileAttributesExW(m_param.wzFilePath, GetFileExInfoStandard, (LPVOID)&tempfileData))
                    {
                        continue;
                    }

                    if (m_param.bIsMonitorSize)
                    {
                        if (fileData.nFileSizeHigh != tempfileData.nFileSizeHigh
                            || fileData.nFileSizeLow != tempfileData.nFileSizeLow)
                        {
                            bChangeSize = true;

                            fileData.nFileSizeHigh = tempfileData.nFileSizeHigh;
                            fileData.nFileSizeLow = tempfileData.nFileSizeLow;
                        }
                    }

                    if (m_param.bIsMonitorAttributes)
                    {
                        if (fileData.dwFileAttributes != tempfileData.dwFileAttributes)
                        {
                            bChangeAttributes = true;

                            fileData.dwFileAttributes = tempfileData.dwFileAttributes;
                        }
                    }

                    if (m_param.bIsMonitorLastWrite)
                    {
                        if (fileData.ftLastWriteTime.dwLowDateTime != tempfileData.ftLastWriteTime.dwLowDateTime
                            || fileData.ftLastWriteTime.dwHighDateTime != tempfileData.ftLastWriteTime.dwHighDateTime)
                        {
                            bChangeLastWrite = true;

                            fileData.ftLastWriteTime = tempfileData.ftLastWriteTime;
                        }
                    }

                    if (bChangeSize || bChangeAttributes || bChangeLastWrite)
                    {
                        if (m_param.pHandler)
                        {
                            m_param.pHandler->OnChanged(bChangeSize, bChangeAttributes, bChangeLastWrite);
                        }
                    }

                    FindNextChangeNotification(hChangeHandle);
                }            
                break;
            default:
                return;
            }
        }
    }
}