#pragma once

#include "SysBase.h"

namespace SysBase
{
    class CSlowReadFileQueue : public CObject
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

        CSlowReadFileQueue();

        virtual ~CSlowReadFileQueue();

        ERROR_CODE Init(const char* lpFileDir, const char* lpName, UINT32 nBlockSize);

        ERROR_CODE GetData(PCHAR pBuffer, UINT32 nMaxBufferSize, UINT32& nDataSize, UINT32& nDataIndex);

        ERROR_CODE FreeData(UINT32 nDataIndex);

    private:

		void* m_pImp;
    };
}