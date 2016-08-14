//////////////////////////////////////////////////////////////////////////
//作者:叶健
//功能：解析型通用协议构造与解析
//协议组织逻辑：
//总大小[4字节] + N * (ID[1字节] + 类型[1字节] + 大小[2字节:表示后面的Value信息大小；
//    当类型为数组时为Count含义，//表示数组中项的个数])
//特殊类型协议组织：
//OBJ:原项大小含义不变，OBJ类型的VALUE信息为ID重新定义的一组项集合（与父结点的ID无任何关系）[OBJ的VALUE内不包含总大小]
//OBJ_ARRAY:OBJ数组，原项内容大小字段含义变为数组的个数，具体大小由OBJ_ARRAY的总大小字段给出,
//          具体的协议组织为：总大小[4字节] + N * (INDEX[2字节：数组的索引值，从0开始] + 大小[2字节:本项值大小，
//          数组中的每个对象的值大小不一定相同])
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SysBase.h"

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    
    struct TDWBool
    {
        TDWBool()
        {
            btBool = 0;
        }

        bool ToBool()
        {
            return btBool == 0 ? false : true;
        }

        unsigned char btBool;   //0 为 FALSE(false);非0 为 TRUE(true)
    };

    struct TDWTime
    {
        TDWTime()
        {
            nTime = 0;
        }

        INT64 nTime;
    };

    //数据类型
    enum DW_DATA_TYPE
    {
        DW_DT_CHAR = 0,
        DW_DT_WCHAR,
        DW_DT_NULL,
        DW_DT_BOOL,
        DW_DT_INT8,
        DW_DT_UINT8,
        DW_DT_INT16,
        DW_DT_UINT16,
        DW_DT_INT32,
        DW_DT_UINT32,
        DW_DT_INT64,
        DW_DT_UINT64,
        DW_DT_TIME,
        DW_DT_FLOAT,
        DW_DT_DOUBLE,
        DW_DT_BYTE,
		DW_DT_UTF8,
        DW_DT_OBJ,
        DW_DT_OBJ_ARRAY,
    };

    //////////////////////////////////////////////////////////////////////////
    //CDWBuilder
 
    class CDWBuilder
    {
    public:

        //////////////////////////////////////////////////////////////////////////

        //错误码
        enum DW_ERROR
        {
            DW_ERROR_SUCCESS = 0,                      //成功
            DW_ERROR_ID_HAS_EXIST,                     //ID已存在
            DW_ERROR_DATA_OUT_OF_MAX_SIZE,             //设置的数据不能超过的最大值（65535）
            DW_ERROR_PARAM,                            //参数错误
            DW_ERROR_MEMORY_ALLOW_FAILED,              //分配系统内存错误
            DW_ERROR_PROTOCOL,                         //协议错误
            CMD_ERROR_OUT_OF_MAX_ITEM,                 //超出数据项范围（最大只支持256个数据项）
        };

        //////////////////////////////////////////////////////////////////////////

        CDWBuilder(UINT32 nMaxSize = 10240);

        virtual ~CDWBuilder();

        const void* GetBuffer();

        UINT32 GetDataSize();

        UINT32 GetBufferSize();

        void Clear();

        DW_ERROR AddItem(const char* pCharStr, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(const WCHAR* pWCharStr, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(const char* pCharStr, UINT16 uLength, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(const WCHAR* pWCharStr, UINT16 uLength, UINT8* pnIndex = NULL);

        //添加NULL值
        DW_ERROR AddItem(UINT8* pnIndex = NULL);

        DW_ERROR AddItem(TDWBool bValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(INT8 nValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(UINT8 uValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(INT16 nValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(UINT16 uValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(INT32 nValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(UINT32 uValue, UINT8* pnIndex = NULL);
        
        DW_ERROR AddItem(INT64 nValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(UINT64 uValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(TDWTime tValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(FLOAT fValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(DOUBLE dbValue, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(const void* pValue, UINT16 uSize, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(CDWBuilder& pObjectBuilder, UINT8* pnIndex = NULL);

        DW_ERROR AddItem(CDWBuilder* pBuilderArray, UINT16 uBuilderCount, UINT8* pnIndex = NULL);

    protected:

        DW_ERROR AddItem(DW_DATA_TYPE eType, const void* pValue, unsigned short uValueSize, UINT8* pnIndex = NULL);

    private:

        UINT32 m_nDataSize;                         //有效数据大小

        UINT32 m_nBufferSize;                       //缓冲区大小

        char m_buffer[10240];                       //默认缓冲区

        char* m_pbigBuffer;                         //自定义的大缓冲区

        char* m_pDestBuffer;                        //目标缓冲区

        UINT8 m_nItemCount;

        char* m_pWritePtr;

    };

    //////////////////////////////////////////////////////////////////////////
    //CDWParser

    class CDWParser
    {
    public:
        
        CDWParser();

        virtual ~CDWParser();

        bool Load(const void* pData, UINT32 nDataSize);

        //nIndex范围为：0 开始到 Count - 1
        const void* GetItemData(UINT8 nIndex, UINT32& nDataSize, DW_DATA_TYPE* outType = NULL);

		bool GetItemWCharData(UINT8 nIndex, UINT32& nDataSize,DW_DATA_TYPE* outType,WCHAR* szbuf,UINT32 ulCount);

        UINT32 GetDataSize();

        UINT32 GetCount();

    private:

        struct TDWItemInfo
        {
            UINT8 uType;
            UINT16 uCount;      //OBJ_ARRAY 的个数
            UINT32 uSize;       //当uType为DW_DT_OBJ_ARRAY时，该字段含义为Count
            char* pData;
        };

        typedef map<UINT8, TDWItemInfo> DataMap;

        DataMap m_DataMap;

        char* m_pBufferPtr;

        UINT32 m_nDataSize;
    };

    //////////////////////////////////////////////////////////////////////////
    //CDWObjectItemParser

    class CDWObjectItemParser
    {
    public:

        CDWObjectItemParser();

        virtual ~CDWObjectItemParser();

        bool Load(const void* pData, UINT32 nDataSize);

        //nIndex范围为：0 开始到 Count - 1
        const void* GetItemData(UINT8 nIndex, UINT32& nDataSize, DW_DATA_TYPE* outType = NULL);


		bool GetItemWCharData(UINT8 nIndex, UINT32& nDataSize,DW_DATA_TYPE* outType,WCHAR* szbuf,UINT32 ulCount);

        UINT32 GetDataSize();

        UINT32 GetCount();

    private:

        struct TDWItemInfo
        {
            UINT8 uType;
            UINT16 uCount;      //OBJ_ARRAY 的个数
            UINT32 uSize;
            char* pData;
        };

        typedef map<UINT8, TDWItemInfo> DataMap;

        DataMap m_DataMap;

        char* m_pBufferPtr;

        UINT32 m_nDataSize;
    };

    //////////////////////////////////////////////////////////////////////////
    //CDWOjbectArrayItemParser

    class CDWOjbectArrayItemParser
    {
    public:

        CDWOjbectArrayItemParser();

        virtual ~CDWOjbectArrayItemParser();

        bool Load(const void* pData, UINT32 nDataSize);

        //nRowIndex 范围为：0 开始到 Count - 1
        //nFieldIndex(字段索引) 范围为：0 开始到 Count - 1
        const void* GetItemData(UINT16 nRowIndex, UINT8 nFieldIndex, UINT32& nDataSize, DW_DATA_TYPE* outType = NULL);

        UINT32 GetDataSize();

        UINT16 GetRowCount();

        UINT32 GetFieldCount(UINT16 nRowIndex);

    private:

        struct TDWItemInfo
        {
            UINT8 uType;
            UINT16 uCount;      //OBJ_ARRAY 的个数
            UINT32 uSize;       //当uType为DW_DT_OBJ_ARRAY时，该字段含义为Count
            char* pData;
        };

        typedef map<UINT8, TDWItemInfo> DataMap;

        typedef vector<DataMap> RowMap;

        RowMap m_RowArray;

        char* m_pBufferPtr;

        UINT32 m_nDataSize;
    };
}