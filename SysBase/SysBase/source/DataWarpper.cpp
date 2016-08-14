#include "../SysBase_Interface.h"

//////////////////////////////////////////////////////////////////////////

#define DW_DEFAULT_BUFFER_SIZE 10240
#define DW_MAX_ITEM_DATA_SIZE 65535
#define DW_MAX_ITEM_COUNT 256

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////
    //结构

#pragma pack(1)
    
    struct TDWValueInfo 
    {
        UINT8 nID;
        UINT8 uType;
        UINT16 uSize;       //当uType为DW_DT_OBJ_ARRAY时，该字段含义为Count
    };

    struct TDWArrayItemInfo
    {
        UINT16 uIndex;
        UINT16 uSize;
    };

#pragma pack()

    //////////////////////////////////////////////////////////////////////////
    //CDWBuilder

    CDWBuilder::CDWBuilder(UINT32 nMaxSize)
    {
        m_nItemCount = 0;

        m_nDataSize = sizeof(UINT32);

        m_pbigBuffer = NULL;

        if (nMaxSize > DW_DEFAULT_BUFFER_SIZE)
        {
            m_pbigBuffer = new char[nMaxSize];//这里不检测任何内存问题

            m_nBufferSize = nMaxSize;

            m_pDestBuffer = m_pbigBuffer;
        }
        else
        {
            m_nBufferSize = DW_DEFAULT_BUFFER_SIZE;

            m_pDestBuffer = m_buffer;
        }

        m_pWritePtr = m_pDestBuffer + sizeof(int);

        memset(m_pDestBuffer, 0, m_nBufferSize);

        *((UINT32*)m_pDestBuffer) = m_nDataSize;
    }

    CDWBuilder::~CDWBuilder()
    {
        if (m_pbigBuffer)
        {
            delete [] m_pbigBuffer;
        }
    }

    const void* CDWBuilder::GetBuffer()
    {
        return m_pDestBuffer;
    }

    UINT32 CDWBuilder::GetDataSize()
    {
        return m_nDataSize;
    }

    UINT32 CDWBuilder::GetBufferSize()
    {
        return m_nBufferSize;
    }

    void CDWBuilder::Clear()
    {
        m_nItemCount = 0;

        m_nDataSize = sizeof(UINT32);

        memset(m_pDestBuffer, 0, m_nBufferSize);

        *((UINT32*)m_pDestBuffer) = m_nDataSize;
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(const char* pCharStr, UINT8* pnIndex)
    {
        UINT16 nLen = (UINT16)strlen(pCharStr);

        return this->AddItem(pCharStr, nLen, pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(const WCHAR* pWCharStr, UINT8* pnIndex)
    {
        UINT16 nLen = (UINT16)wcslen(pWCharStr);

        return this->AddItem(pWCharStr, nLen, pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(const char* pCharOrStr, UINT16 uLength, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_CHAR, pCharOrStr, uLength, pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(const WCHAR* pWCharOrWStr, UINT16 uLength, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_WCHAR, pWCharOrWStr, uLength * 2, pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_NULL, NULL, 0, pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(TDWBool bValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_BOOL, &bValue, sizeof(bValue), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(INT8 nValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_INT8, &nValue, sizeof(INT8), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(UINT8 uValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_UINT8, &uValue, sizeof(UINT8), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(INT16 nValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_INT16, &nValue, sizeof(INT16), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(UINT16 uValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_UINT16, &uValue, sizeof(UINT16), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(INT32 nValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_INT32, &nValue, sizeof(INT32), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(UINT32 uValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_UINT32, &uValue, sizeof(UINT32), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(INT64 nValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_INT64, &nValue, sizeof(INT64), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(UINT64 uValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_UINT64, &uValue, sizeof(UINT64), pnIndex);
    }

     CDWBuilder::DW_ERROR CDWBuilder::AddItem(TDWTime tValue, UINT8* pnIndex)
     {
        return this->AddItem(DW_DT_TIME, &tValue, sizeof(TDWTime), pnIndex);
     }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(FLOAT fValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_FLOAT, &fValue, sizeof(FLOAT), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(DOUBLE dbValue, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_DOUBLE, &dbValue, sizeof(DOUBLE), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(const void* pValue, UINT16 uSize, UINT8* pnIndex)
    {
        return this->AddItem(DW_DT_BYTE, pValue, uSize, pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(CDWBuilder& aObjectBuilder, UINT8* pnIndex)
    {
        UINT32 nSize = aObjectBuilder.GetDataSize();

        const char* pBuffer = (const char*)aObjectBuilder.GetBuffer();

        if (nSize > DW_MAX_ITEM_DATA_SIZE)  //项的大小不能超过UINT16最大值
        {
            return DW_ERROR_DATA_OUT_OF_MAX_SIZE;
        }

        return this->AddItem(DW_DT_OBJ, (const void*)(pBuffer + sizeof(UINT32)), nSize - (UINT32)sizeof(UINT32), pnIndex);
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(CDWBuilder* pBuilderArray, UINT16 uBuilderCount, UINT8* pnIndex)
    {
        if (uBuilderCount > 0 && !pBuilderArray)
        {
            return DW_ERROR_PARAM;
        }

        if (DW_MAX_ITEM_COUNT <= m_nItemCount)
        {
            return CMD_ERROR_OUT_OF_MAX_ITEM;
        }

        UINT16 uIndex = 0;

        //////////////////////////////////////////////////////////////////////////
        //检测长度

        UINT32 uValueSize = 0;

        for (uIndex = 0; uIndex < uBuilderCount; ++uIndex)
        {
            if (pBuilderArray[uIndex].GetDataSize() > DW_MAX_ITEM_DATA_SIZE)
            {
                return DW_ERROR_DATA_OUT_OF_MAX_SIZE;
            }

            uValueSize += pBuilderArray[uIndex].GetDataSize() + 4;  //增加的4个字节为index 与 size
        }

        uValueSize += 4;    //增加的4个字节为数据信息总大小需要的存储空间

        //////////////////////////////////////////////////////////////////////////
        //如果增加的项长度比之前的多,则重新分配内存

        if (m_nDataSize + sizeof(TDWValueInfo) + uValueSize > m_nBufferSize)
        {
            UINT32 nNewBufferSize = m_nDataSize + sizeof(TDWValueInfo) + uValueSize + m_nBufferSize;

            char *pNewBuffer = new(std::nothrow) char[nNewBufferSize];

            if (!pNewBuffer)
            {
                return DW_ERROR_MEMORY_ALLOW_FAILED;
            }

            memset(pNewBuffer, 0, nNewBufferSize);

            m_nBufferSize = nNewBufferSize;

            if (m_pbigBuffer)
            {
                memcpy(pNewBuffer, m_pbigBuffer, m_nDataSize);

                delete [] m_pbigBuffer;
            }
            else
            {
                memcpy(pNewBuffer, m_buffer, m_nDataSize);
            }

            m_pbigBuffer = pNewBuffer;

            m_pDestBuffer = m_pbigBuffer;

            m_pWritePtr = m_pDestBuffer + m_nDataSize;
        }

        //////////////////////////////////////////////////////////////////////////
        //生成新的ID

        UINT8 nID = m_nItemCount;

        if (pnIndex)
        {
            *pnIndex = nID;
        }

        m_nItemCount++;

        //////////////////////////////////////////////////////////////////////////
        //构造协议[无任何数据]

        if (0 == uBuilderCount)
        {
            //写入数据协议
            TDWValueInfo* pValueInfo = (TDWValueInfo*)m_pWritePtr;

            pValueInfo->nID = nID;
            pValueInfo->uType = (UINT8)DW_DT_OBJ_ARRAY;
            pValueInfo->uSize = 0;  //此时为Count

            m_pWritePtr += sizeof(TDWValueInfo);

            UINT32 uValue = 0;

            memcpy(m_pWritePtr, &uValue, sizeof(UINT32));

            m_pWritePtr += sizeof(UINT32);

            m_nDataSize += sizeof(TDWValueInfo) + sizeof(UINT32);

            UINT32* pTotalSize = (UINT32*)m_pDestBuffer;
            *pTotalSize = m_nDataSize;

            return DW_ERROR_SUCCESS;
        }

        //////////////////////////////////////////////////////////////////////////
        //构造协议[有数据]

        //写入数据协议
        TDWValueInfo* pValueInfo = (TDWValueInfo*)m_pWritePtr;

        pValueInfo->nID = nID;
        pValueInfo->uType = (UINT8)DW_DT_OBJ_ARRAY;
        pValueInfo->uSize = uBuilderCount;  //此时为Count

        m_pWritePtr += sizeof(TDWValueInfo);

        m_nDataSize += sizeof(TDWValueInfo);

        UINT32* pArrayTotalSize = (UINT32*)m_pWritePtr;
        *pArrayTotalSize = sizeof(UINT32);

        m_pWritePtr += sizeof(UINT32);

        m_nDataSize += sizeof(UINT32);

        for (uIndex = 0; uIndex < uBuilderCount; ++uIndex)
        {
            TDWArrayItemInfo* pTDWArrayItemInfo = (TDWArrayItemInfo*)m_pWritePtr;

            pTDWArrayItemInfo->uIndex = uIndex;

            pTDWArrayItemInfo->uSize = (UINT16)(pBuilderArray[uIndex].GetDataSize() - sizeof(UINT32));

            m_pWritePtr += sizeof(TDWArrayItemInfo);

            m_nDataSize += sizeof(TDWArrayItemInfo);

            const char* pData = (const char*)pBuilderArray[uIndex].GetBuffer() + sizeof(UINT32);

            memcpy(m_pWritePtr, pData, pTDWArrayItemInfo->uSize);

            m_pWritePtr += pTDWArrayItemInfo->uSize;

            m_nDataSize += pTDWArrayItemInfo->uSize;

            *pArrayTotalSize += sizeof(TDWArrayItemInfo) + pTDWArrayItemInfo->uSize;
        }

        UINT32* pTotalSize = (UINT32*)m_pDestBuffer;
        *pTotalSize = m_nDataSize;

        return DW_ERROR_SUCCESS;
    }

    CDWBuilder::DW_ERROR CDWBuilder::AddItem(DW_DATA_TYPE eType, const void* pValue, unsigned short uValueSize, UINT8* pnIndex)
    {
        if (uValueSize > 0)
        {
            if (!pValue)
            {
                return DW_ERROR_PARAM;
            }
        }

        if (DW_MAX_ITEM_COUNT <= m_nItemCount)
        {
            return CMD_ERROR_OUT_OF_MAX_ITEM;
        }

        UINT32* pTotalSize = (UINT32*)m_pDestBuffer;

        //////////////////////////////////////////////////////////////////////////
        //如果增加的项长度比之前的多,则重新分配内存

        if (m_nDataSize + sizeof(TDWValueInfo) + uValueSize > m_nBufferSize)
        {
            UINT32 nNewBufferSize = m_nDataSize + sizeof(TDWValueInfo) + uValueSize + m_nBufferSize;

            char *pNewBuffer = new(std::nothrow) char[nNewBufferSize];

            if (!pNewBuffer)
            {
                return DW_ERROR_MEMORY_ALLOW_FAILED;
            }

            memset(pNewBuffer, 0, nNewBufferSize);

            m_nBufferSize = nNewBufferSize;

            if (m_pbigBuffer)
            {
                memcpy_s(pNewBuffer, nNewBufferSize, m_pbigBuffer, m_nDataSize);

                delete [] m_pbigBuffer;

                m_pbigBuffer = NULL;
            }
            else
            {
                memcpy_s(pNewBuffer, nNewBufferSize, m_buffer, m_nDataSize);
            }

            m_pbigBuffer = pNewBuffer;

            m_pDestBuffer = m_pbigBuffer;

            m_pWritePtr = m_pDestBuffer + m_nDataSize;

            pTotalSize = (UINT32*)m_pDestBuffer;
        }

        //////////////////////////////////////////////////////////////////////////

        UINT8 nID = m_nItemCount;

        if (pnIndex)
        {
            *pnIndex = nID;
        }

        m_nItemCount++;

        //写入数据协议
        TDWValueInfo* pValueInfo = (TDWValueInfo*)m_pWritePtr;

        pValueInfo->nID = nID;
        pValueInfo->uType = (UINT8)eType;
        pValueInfo->uSize = uValueSize;

        m_pWritePtr += sizeof(TDWValueInfo);

        if (uValueSize > 0)
        {
            memcpy(m_pWritePtr, pValue, uValueSize);
        }

        m_pWritePtr += uValueSize;

        m_nDataSize += sizeof(TDWValueInfo) + uValueSize;

        *pTotalSize = m_nDataSize;

        return DW_ERROR_SUCCESS;
    }

    //////////////////////////////////////////////////////////////////////////
    //CDWParser

    CDWParser::CDWParser()
    {
        m_pBufferPtr = NULL;

        m_nDataSize = 0;
    }

    CDWParser::~CDWParser()
    {
        m_pBufferPtr = NULL;

        m_nDataSize = 0;

        m_DataMap.clear();
    }

    bool CDWParser::Load(const void* pData, UINT32 nDataSize)
    {
        m_pBufferPtr = (char*)pData;

        m_nDataSize = 0;

        m_DataMap.clear();

        if (*((UINT32*)pData) != nDataSize || nDataSize < sizeof(UINT32))
        {
            return false;
        }

        if (0 == *((UINT32*)pData))
        {
            return true;
        }

        m_nDataSize = nDataSize;

        UINT32 uCurSize = 0;

        char* pReadPtr = m_pBufferPtr + sizeof(UINT32);
        uCurSize += sizeof(UINT32);

        while (uCurSize < nDataSize)
        {
            if (uCurSize + sizeof(TDWValueInfo) > nDataSize)
            {
                return false;
            }

            TDWValueInfo* pTDWValueInfo = (TDWValueInfo*)pReadPtr;

            if ((UINT8)DW_DT_OBJ == pTDWValueInfo->uType) //对象
            {
                if (uCurSize + pTDWValueInfo->uSize > nDataSize)
                {
                    return false;
                }

                TDWItemInfo aTDWItemInfo = {0};

                aTDWItemInfo.uSize = pTDWValueInfo->uSize;
                aTDWItemInfo.uType = pTDWValueInfo->uType;

                aTDWItemInfo.pData = pReadPtr + sizeof(TDWValueInfo);

                m_DataMap.insert(DataMap::value_type(pTDWValueInfo->nID, aTDWItemInfo));

                pReadPtr += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
                uCurSize += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
            }
            else if ((UINT8)DW_DT_OBJ_ARRAY == pTDWValueInfo->uType)  //对象数组
            {
                if (uCurSize + sizeof(TDWValueInfo) + sizeof(UINT32) > nDataSize)
                {
                    return false;
                }

                UINT32* pArrayTotalSize = (UINT32*)(pReadPtr + sizeof(TDWValueInfo));

                if (*pArrayTotalSize < sizeof(UINT32))
                {
                    return false;
                }

                if (uCurSize + sizeof(TDWValueInfo) + *pArrayTotalSize > nDataSize)
                {
                    return false;
                }

                TDWItemInfo aTDWItemInfo = {0};

                aTDWItemInfo.uSize = *pArrayTotalSize + sizeof(UINT16);    //大小信息加上前面的RowCount值大小
                aTDWItemInfo.uType = pTDWValueInfo->uType;
                aTDWItemInfo.uCount = pTDWValueInfo->uSize;

                aTDWItemInfo.pData = pReadPtr + sizeof(TDWValueInfo) - sizeof(UINT16);    //数组的信息，数据初始从行数信息开始（RowCount + 总大小 + N*(...)）

                m_DataMap.insert(DataMap::value_type(pTDWValueInfo->nID, aTDWItemInfo));

                pReadPtr += sizeof(TDWValueInfo) + aTDWItemInfo.uSize - sizeof(UINT16);
                uCurSize += sizeof(TDWValueInfo) + aTDWItemInfo.uSize - sizeof(UINT16);
            }
            else
            {
                if (uCurSize + pTDWValueInfo->uSize > nDataSize)
                {
                    return false;
                }

                TDWItemInfo aTDWItemInfo = {0};

                aTDWItemInfo.uSize = pTDWValueInfo->uSize;
                aTDWItemInfo.uType = pTDWValueInfo->uType;

                aTDWItemInfo.pData = pReadPtr + sizeof(TDWValueInfo);

                m_DataMap.insert(DataMap::value_type(pTDWValueInfo->nID, aTDWItemInfo));

                pReadPtr += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
                uCurSize += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
            }
        }

        return true;
    }

    const void* CDWParser::GetItemData(UINT8 nIndex, UINT32& nDataSize, DW_DATA_TYPE* outType)
    {
        DataMap::iterator entity = m_DataMap.find(nIndex);

        if (entity == m_DataMap.end())
        {
            nDataSize = 0;

            return NULL;
        }
		DW_DATA_TYPE temp = (DW_DATA_TYPE)entity->second.uType;

        if (outType)
        {
            *outType = (DW_DATA_TYPE)entity->second.uType;
        }

		
        nDataSize = entity->second.uSize;

        return entity->second.pData;
    }
	bool CDWParser::GetItemWCharData(UINT8 nIndex, UINT32& nDataSize,DW_DATA_TYPE* outType,WCHAR* szbuf,UINT32 ulCount)
	{
	    DataMap::iterator entity = m_DataMap.find(nIndex);

        if (entity == m_DataMap.end())
        {
            nDataSize = 0;

			return false;
        }
		DW_DATA_TYPE Type = (DW_DATA_TYPE)entity->second.uType;
		UINT32 wcharLen = ulCount*sizeof(WCHAR);
		nDataSize = entity->second.uSize;
		if (Type == DW_DT_WCHAR)
		{
			if (entity->second.uSize>wcharLen)
			{
				memcpy(szbuf,entity->second.pData,wcharLen);
			}else
			{
				memcpy(szbuf,entity->second.pData,entity->second.uSize);
			}
			
			return true;
		}else
		if (Type == DW_DT_UTF8)
		{
			MultiByteToWideChar( CP_UTF8, 0, entity->second.pData, -1, szbuf, ulCount);  
			if (outType)
			{
				*outType = DW_DT_WCHAR;
			}
			nDataSize = (entity->second.uSize*sizeof(DW_DT_WCHAR));
			return true;
		}else
		{
			return false;
		}
	
		return true;
	}
    UINT32 CDWParser::GetDataSize()
    {
        return m_nDataSize;
    }

    UINT32 CDWParser::GetCount()
    {
        return (UINT32)m_DataMap.size();
    }

    //////////////////////////////////////////////////////////////////////////
    //CDWObjectItemParser

    CDWObjectItemParser::CDWObjectItemParser()
    {
        m_pBufferPtr = NULL;

        m_nDataSize = 0;
    }

    CDWObjectItemParser::~CDWObjectItemParser()
    {
        m_pBufferPtr = NULL;

        m_nDataSize = 0;

        m_DataMap.clear();
    }

    bool CDWObjectItemParser::Load(const void* pData, UINT32 nDataSize)
    {
        m_pBufferPtr = (char*)pData;

        m_DataMap.clear();

        m_nDataSize = 0;

        if (0 == nDataSize)
        {
            return false;
        }

        m_nDataSize = nDataSize;

        UINT32 uCurSize = 0;

        char* pReadPtr = m_pBufferPtr;

        while (uCurSize < nDataSize)
        {
            if (uCurSize + sizeof(TDWValueInfo) > nDataSize)
            {
                return false;
            }

            TDWValueInfo* pTDWValueInfo = (TDWValueInfo*)pReadPtr;

            if ((UINT8)DW_DT_OBJ == pTDWValueInfo->uType) //对象
            {
                if (uCurSize + pTDWValueInfo->uSize > nDataSize)
                {
                    return false;
                }

                TDWItemInfo aTDWItemInfo = {0};

                aTDWItemInfo.uSize = pTDWValueInfo->uSize;
                aTDWItemInfo.uType = pTDWValueInfo->uType;
		
                aTDWItemInfo.pData = pReadPtr + sizeof(TDWValueInfo);

                m_DataMap.insert(DataMap::value_type(pTDWValueInfo->nID, aTDWItemInfo));

                pReadPtr += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
                uCurSize += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
            }
            else if ((UINT8)DW_DT_OBJ_ARRAY == pTDWValueInfo->uType)  //对象数组
            {
                if (uCurSize + sizeof(TDWValueInfo) + sizeof(UINT32) > nDataSize)
                {
                    return false;
                }

                UINT32* pArrayTotalSize = (UINT32*)(pReadPtr + sizeof(TDWValueInfo));

                if (*pArrayTotalSize < sizeof(UINT32))
                {
                    return false;
                }

                if (uCurSize + sizeof(TDWValueInfo) + *pArrayTotalSize > nDataSize)
                {
                    return false;
                }

                TDWItemInfo aTDWItemInfo = {0};

                aTDWItemInfo.uSize = *pArrayTotalSize + sizeof(UINT16);    //大小信息加上前面的RowCount值大小
                aTDWItemInfo.uType = pTDWValueInfo->uType;
                aTDWItemInfo.uCount = pTDWValueInfo->uSize;

                aTDWItemInfo.pData = pReadPtr + sizeof(TDWValueInfo) - sizeof(UINT16);    //数组的信息，数据初始从行数信息开始（RowCount + 总大小 + N*(...)）

                m_DataMap.insert(DataMap::value_type(pTDWValueInfo->nID, aTDWItemInfo));

                pReadPtr += sizeof(TDWValueInfo) + aTDWItemInfo.uSize - sizeof(UINT16);
                uCurSize += sizeof(TDWValueInfo) + aTDWItemInfo.uSize - sizeof(UINT16);
            }
            else
            {
                if (uCurSize + pTDWValueInfo->uSize > nDataSize)
                {
                    return false;
                }

                TDWItemInfo aTDWItemInfo = {0};

                aTDWItemInfo.uSize = pTDWValueInfo->uSize;
                aTDWItemInfo.uType = pTDWValueInfo->uType;

                aTDWItemInfo.pData = pReadPtr + sizeof(TDWValueInfo);

                m_DataMap.insert(DataMap::value_type(pTDWValueInfo->nID, aTDWItemInfo));

                pReadPtr += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
                uCurSize += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
            }
        }

        return true;
    }

    const void* CDWObjectItemParser::GetItemData(UINT8 nIndex, UINT32& nDataSize, DW_DATA_TYPE* outType)
    {
        DataMap::iterator entity = m_DataMap.find(nIndex);

        if (entity == m_DataMap.end())
        {
            nDataSize = 0;

            return NULL;
        }

        if (outType)
        {
            *outType = (DW_DATA_TYPE)entity->second.uType;
        }

        nDataSize = entity->second.uSize;

        return entity->second.pData;
    }


	bool CDWObjectItemParser::GetItemWCharData(UINT8 nIndex, UINT32& nDataSize,DW_DATA_TYPE* outType,WCHAR* szbuf,UINT32 ulCount)
	{
	    DataMap::iterator entity = m_DataMap.find(nIndex);

        if (entity == m_DataMap.end())
        {
            nDataSize = 0;

			return false;
        }
		DW_DATA_TYPE Type = (DW_DATA_TYPE)entity->second.uType;
		UINT32 wcharLen = ulCount*sizeof(WCHAR);
		nDataSize = entity->second.uSize;
		if (Type == DW_DT_WCHAR)
		{
			if (entity->second.uSize>wcharLen)
			{
				memcpy(szbuf,entity->second.pData,wcharLen);
			}else
			{
				memcpy(szbuf,entity->second.pData,entity->second.uSize);
			}
			
			return true;
		}else
		if (Type == DW_DT_UTF8)
		{
			MultiByteToWideChar( CP_UTF8, 0, entity->second.pData, -1, szbuf, ulCount);  
			if (outType)
			{
				*outType = DW_DT_WCHAR;
			}
			nDataSize = (entity->second.uSize*sizeof(DW_DT_WCHAR));
			return true;
		}else
		{
			return false;
		}
		return true;
	}
    UINT32 CDWObjectItemParser::GetDataSize()
    {
        return m_nDataSize;
    }

    UINT32 CDWObjectItemParser::GetCount()
    {
        return (UINT32)m_DataMap.size();
    }

    //////////////////////////////////////////////////////////////////////////
    //CDWOjbectArrayItemParser

    CDWOjbectArrayItemParser::CDWOjbectArrayItemParser()
    {
        m_pBufferPtr = NULL;

        m_nDataSize = 0;
    }

    CDWOjbectArrayItemParser::~CDWOjbectArrayItemParser()
    {
        m_pBufferPtr = NULL;

        m_nDataSize = 0;

        m_RowArray.clear();
    }

    bool CDWOjbectArrayItemParser::Load(const void* pData, UINT32 nDataSize)
    {
        m_pBufferPtr = (char*)pData;

        m_RowArray.clear();

        m_nDataSize = 0;

        if (nDataSize < sizeof(UINT32) + sizeof(UINT16))
        {
            return false;
        }

        UINT16 uRowCount = *(UINT16*)pData;

        UINT32 uTotalSize = *(UINT32*)((char*)pData + sizeof(UINT16));

        if (uTotalSize != (nDataSize - sizeof(UINT16)))
        {
            return false;
        }

        if (0 == uRowCount)
        {
            return false;
        }

        if (0 == *((UINT32*)pData))
        {
            return true;
        }

        m_nDataSize = nDataSize;

        //////////////////////////////////////////////////////////////////////////
        //行数
        m_RowArray.resize(uRowCount);

        //////////////////////////////////////////////////////////////////////////
        //解析数据
        UINT32 uCurSize = 0;

        char* pReadPtr = m_pBufferPtr + sizeof(UINT16) + sizeof(UINT32);
        uCurSize += sizeof(UINT16) + sizeof(UINT32);

        UINT16 nRowIndex = 0;

        while (uCurSize < nDataSize)
        {
            //////////////////////////////////////////////////////////////////////////
            //解析行
            if (uCurSize + sizeof(TDWArrayItemInfo) > nDataSize)
            {
                return false;
            }

            TDWArrayItemInfo* pTDWArrayItemInfo = (TDWArrayItemInfo*)pReadPtr;

            pReadPtr += sizeof(TDWArrayItemInfo);
            uCurSize += sizeof(TDWArrayItemInfo);

            DataMap& aDataMap = m_RowArray[nRowIndex];
            nRowIndex++;

            int uRowCurSize = 0;

            //////////////////////////////////////////////////////////////////////////
            //解析行内容

            while (uRowCurSize < pTDWArrayItemInfo->uSize)
            {
                if (uRowCurSize + sizeof(TDWValueInfo) > pTDWArrayItemInfo->uSize)
                {
                    return false;
                }

                TDWValueInfo* pTDWValueInfo = (TDWValueInfo*)pReadPtr;

                if ((UINT8)DW_DT_OBJ == pTDWValueInfo->uType) //对象
                {
                    if (uRowCurSize + pTDWValueInfo->uSize > pTDWArrayItemInfo->uSize)
                    {
                        return false;
                    }

                    TDWItemInfo aTDWItemInfo = {0};

                    aTDWItemInfo.uSize = pTDWValueInfo->uSize;
                    aTDWItemInfo.uType = pTDWValueInfo->uType;

                    aTDWItemInfo.pData = pReadPtr + sizeof(TDWValueInfo);

                    aDataMap.insert(DataMap::value_type(pTDWValueInfo->nID, aTDWItemInfo));

                    pReadPtr += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
                    uRowCurSize += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
                }
                else if ((UINT8)DW_DT_OBJ_ARRAY == pTDWValueInfo->uType)  //对象数组
                {
                    if (uRowCurSize + sizeof(TDWValueInfo) + sizeof(UINT32) > pTDWArrayItemInfo->uSize)
                    {
                        return false;
                    }

                    UINT32* pArrayTotalSize = (UINT32*)(pReadPtr + sizeof(TDWValueInfo));

                    if (*pArrayTotalSize < sizeof(UINT32))
                    {
                        return false;
                    }

                    if (uRowCurSize + sizeof(TDWValueInfo) + *pArrayTotalSize > pTDWArrayItemInfo->uSize)
                    {
                        return false;
                    }

                    TDWItemInfo aTDWItemInfo = {0};

                    aTDWItemInfo.uSize = *pArrayTotalSize + sizeof(UINT16);    //大小信息加上前面的RowCount值大小
                    aTDWItemInfo.uType = pTDWValueInfo->uType;
                    aTDWItemInfo.uCount = pTDWValueInfo->uSize;

                    aTDWItemInfo.pData = pReadPtr + sizeof(TDWValueInfo) - sizeof(UINT16);    //数组的信息，数据初始从行数信息开始（RowCount + 总大小 + N*(...)）

                    aDataMap.insert(DataMap::value_type(pTDWValueInfo->nID, aTDWItemInfo));

                    pReadPtr += sizeof(TDWValueInfo) + aTDWItemInfo.uSize - sizeof(UINT16);
                    uRowCurSize += sizeof(TDWValueInfo) + aTDWItemInfo.uSize - sizeof(UINT16);
                }
                else
                {
                    if (uRowCurSize + pTDWValueInfo->uSize > pTDWArrayItemInfo->uSize)
                    {
                        return false;
                    }

                    TDWItemInfo aTDWItemInfo = {0};

                    aTDWItemInfo.uSize = pTDWValueInfo->uSize;
                    aTDWItemInfo.uType = pTDWValueInfo->uType;

                    aTDWItemInfo.pData = pReadPtr + sizeof(TDWValueInfo);

                    aDataMap.insert(DataMap::value_type(pTDWValueInfo->nID, aTDWItemInfo));

                    pReadPtr += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
                    uRowCurSize += sizeof(TDWValueInfo) + aTDWItemInfo.uSize;
                }
            }

            uCurSize += pTDWArrayItemInfo->uSize;
        }

        return true;
    }

    const void* CDWOjbectArrayItemParser::GetItemData(UINT16 nRowIndex, UINT8 nFieldIndex, UINT32& nDataSize, DW_DATA_TYPE* outType)
    {
        if ((UINT32)nRowIndex >= m_RowArray.size())
        {
            return NULL;
        }

        DataMap& aDataMap = m_RowArray[nRowIndex];

        DataMap::iterator entity = aDataMap.find(nFieldIndex);

        if (entity == aDataMap.end())
        {
            nDataSize = 0;

            return NULL;
        }

        if (outType)
        {
            *outType = (DW_DATA_TYPE)entity->second.uType;
        }

        nDataSize = entity->second.uSize;

        return entity->second.pData;
    }

    UINT32 CDWOjbectArrayItemParser::GetDataSize()
    {
        return m_nDataSize;
    }

    UINT16 CDWOjbectArrayItemParser::GetRowCount()
    {
        return (UINT16)m_RowArray.size();
    }

    UINT32 CDWOjbectArrayItemParser::GetFieldCount(UINT16 nRowIndex)
    {
        if ((UINT32)nRowIndex >= m_RowArray.size())
        {
            return 0;
        }

        return (UINT32)m_RowArray[nRowIndex].size();
    }
}