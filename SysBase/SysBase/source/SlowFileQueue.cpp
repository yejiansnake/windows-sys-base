#include "../SysBase_Interface.h"

namespace SysBase
{
    CSlowFileQueue::CSlowFileQueue()
    {

    }

    CSlowFileQueue::~CSlowFileQueue()
    {

    }

    CSlowFileQueue::ERROR_CODE CSlowFileQueue::Init(const char* lpFileDir, const char* lpWirteFileName, const char* lpExtName, const char* lpName, UINT32 nBlockSize, UINT32 nMaxFileSize, UINT32 nFileLiveTime)
    {
        CSlowWriteFileQueue::ERROR_CODE errorCode_Write = m_CSlowWriteFileQueue.Init(lpFileDir, lpWirteFileName, lpExtName, lpName, nBlockSize, nMaxFileSize, nFileLiveTime);

        if (errorCode_Write != CSlowWriteFileQueue::ERROR_CODE_NONE)
        {
            return (CSlowFileQueue::ERROR_CODE)errorCode_Write;
        }

        CSlowReadFileQueue::ERROR_CODE errorCode_Read = m_CSlowReadFileQueue.Init(lpFileDir, lpName, nBlockSize);

        if (errorCode_Read != CSlowReadFileQueue::ERROR_CODE_NONE)
        {
            return (CSlowFileQueue::ERROR_CODE)errorCode_Read;
        }

        return CSlowFileQueue::ERROR_CODE_NONE;
    }

    CSlowFileQueue::ERROR_CODE CSlowFileQueue::Add(const void* pData, UINT32 nDataSize)
    {
        CSlowWriteFileQueue::ERROR_CODE errorCode = m_CSlowWriteFileQueue.Add(pData, nDataSize);

        if (errorCode != CSlowWriteFileQueue::ERROR_CODE_NONE)
        {
            return (CSlowFileQueue::ERROR_CODE)errorCode;
        }

        return CSlowFileQueue::ERROR_CODE_NONE;
    }

    CSlowFileQueue::ERROR_CODE CSlowFileQueue::GetData(PCHAR pBuffer, UINT32 nMaxBufferSize, UINT32& nDataSize, UINT32& nDataIndex)
    {
        CSlowReadFileQueue::ERROR_CODE errorCode = m_CSlowReadFileQueue.GetData(pBuffer, nMaxBufferSize, nDataSize, nDataIndex);

        if (errorCode != CSlowReadFileQueue::ERROR_CODE_NONE)
        {
            return (CSlowFileQueue::ERROR_CODE)errorCode;
        }

        return CSlowFileQueue::ERROR_CODE_NONE;
    }

    CSlowFileQueue::ERROR_CODE CSlowFileQueue::FreeData(UINT32 nDataIndex)
    {
        CSlowReadFileQueue::ERROR_CODE errorCode = m_CSlowReadFileQueue.FreeData(nDataIndex);

        if (errorCode != CSlowReadFileQueue::ERROR_CODE_NONE)
        {
            return (CSlowFileQueue::ERROR_CODE)errorCode;
        }

        return CSlowFileQueue::ERROR_CODE_NONE;
    }
}