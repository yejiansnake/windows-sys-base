#pragma once

#include "SysBase.h"

namespace SysBase
{
    struct TFastFileBlock
    {
        PCHAR pData;        //数据
        UINT32 nDataSize;   //有效数据大小
        UINT32 nIndex;      //数据块索引
    };

    class CFastFileQueue : public CObject
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        enum ERROR_CODE
        {
            ERROR_CODE_NONE = 0,
            ERROR_CODE_NO_INIT,         //未初始化
            ERROR_CODE_PARAM,           //参数错误
            ERROR_CODE_SYSTEM,          //内部资源创建失败或程序内逻辑失败
            ERROR_CODE_FILE,            //目标文件与当前初始化信息
            ERROR_CODE_FULL,            //没有空闲的数据空间
            ERROR_CODE_EMTPY,           //队列中没有任何有效数据
            ERROR_CODE_INDEX_OUT,       //索引值超出范围
            ERROR_CODE_INDEX_INVALID,   //索引值对应的数据块已经释放
        };
 
        //////////////////////////////////////////////////////////////////////////

        CFastFileQueue();

        virtual ~CFastFileQueue();

        ERROR_CODE Init(const char* lpFilePath, const char* lpName, UINT32 nBlockSize, UINT32 nBlockCount);

        ERROR_CODE Add(const void* pData, UINT32 nDataSize);

        ERROR_CODE GetData(TFastFileBlock& aFastFileBlock, UINT32 nWaitTime);

        ERROR_CODE FreeData(TFastFileBlock& aFastFileBlock);

        UINT32 GetQueueCount();

    private:

        void* m_pImp;
    };
}
