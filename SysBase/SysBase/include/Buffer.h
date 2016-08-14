#pragma once

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //CBuffer 缓冲区管理类

    class CBuffer : public CObject
    {
    public:

		//////////////////////////////////////////////////////////////////////////

		enum ERROR_CODE
		{
			ERROR_CODE_SUCCESS = 0,
			ERROR_CODE_PARAM,
		};

		//////////////////////////////////////////////////////////////////////////

        //功能:构造函数(默认可使用缓冲区大小等于缓冲区大小)
        //参数:
        //      UINT32 nSize IN 缓冲区大小
        CBuffer(UINT32 nSize);

        virtual ~CBuffer();

        //功能:获取整个缓冲区大小
        //返回值: 缓冲区大小
        UINT32 GetSize();

		UINT32 GetDataSize();

		ERROR_CODE SetDataSize(UINT32 nDataSize);

        //功能:获取缓冲区指定地址
        //参数:
        //      UINT32 nOffset IN 相对缓冲区首地址的偏移量
        //返回值: 缓冲区指定地址
        char* GetData(UINT32 nOffset = 0);

		//功能:清空现有缓冲区的所有信息[但保留原来内存空间大小]
		void ClearData();

    protected:

		UINT32 m_nDataSize;		//有效数据大小

        UINT32 m_nSize;			//缓冲区大小

        char* m_pBuffer;		//缓冲区首地址
    };
}
