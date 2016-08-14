#pragma once

#include <Windows.h>
#include <map>
#include <list>

using namespace std;

namespace SysBase
{
    struct TFastFileCacheBlock
    {
        UINT32 nIndex;      //数据块索引
        PCHAR pData;        //数据
        UINT32 nDataSize;   //有效数据大小
    };

    class CFastFileCache
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

        CFastFileCache();

        virtual ~CFastFileCache();

        ERROR_CODE Init(const char* lpFilePath, const char* lpName, UINT32 nMaxWriteSize, UINT32 nMaxFileSize);

        ERROR_CODE Add(const void* pData, UINT32 nDataSize, DWORD* pdwIndexOut);

        const TFastFileCacheBlock* GetBlock(UINT32 nIndex);

        DWORD GetCurBlockCount(BOOL* pbIsEnd = NULL);

        void Clear();

    protected:

    private:

#pragma pack(1)

        //文件头
        struct TFileHead
        {
            CHAR szName[64];            //文件有效标识
            UINT32 nSize;               //文件能写入的总大小（可能实际比文件小）[固定值]
            UINT32 nHasWriteSize;       //已写入有效的数据总大小
            UINT32 nWriteCount;         //写入的数据包总数
            BYTE btEnd;                 //缓存是否不可在写入
        };

        //数据块头
        struct TBlockHead
        {
            UINT32 nIndex;
            UINT32 nDataSize;
        };

#pragma pack()

        //////////////////////////////////////////////////////////////////////////

        typedef map<DWORD, TFastFileCacheBlock> BlockMap;

        BlockMap m_BlockMap;

        CRITICAL_SECTION m_CriticalSection;

        HANDLE m_hFile;

        HANDLE m_hFileMap;

        LPVOID m_lpBuffer;

        TFileHead* m_pTFileHead;        //文件头

        BOOL m_bInit;

        DWORD m_dwMaxWriteSize;
    };
}