#pragma once

#include <io.h>
#include <stdio.h>
#include <stddef.h>
#include <Windows.h>

namespace SysBase
{
    class IMD5eEncodeFileAction
    {
    public:
        IMD5eEncodeFileAction(){};
        virtual ~IMD5eEncodeFileAction(){};

        virtual void EncodeStart(INT64 nParam, INT64 nFileSize) {};

        //编码中
        //参数:
        //      nParam 自定义参数
        //      nTotalSize 文件大小（总共需要编码的大小）
        //      nHasEncodeSize 已经编码的大小
        //      nEncodeSize 本次编码的大小
        virtual bool Encodeing(INT64 nParam, INT64 nTotalSize, INT64 nHasEncodeSize, INT64 nEncodeSize) { return true; };
        
        virtual void EncodeEnd(INT64 nParam) {};

        virtual void EncodeError(INT64 nParam, int errorCode) {};
    };

    class CMD5
    {
    public:

        static bool Encode(unsigned char md5Out[16], const void* pBuffer, unsigned int nBufferSize);

        static bool EncodeFileA(unsigned char md5Out[16], LPCSTR lpFilePath, IMD5eEncodeFileAction* pAction = NULL, INT64 nParam = -1);

        static bool EncodeFileW(unsigned char md5Out[16], LPCWSTR lpFilePath, IMD5eEncodeFileAction* pAction = NULL, INT64 nParam = -1);

        static char* ToString(char szMD5Out[32], unsigned char md5[16]);

    private:

        static bool EncodeFile(unsigned char md5Out[16], LPCSTR lpFilePath, bool isUnicode, IMD5eEncodeFileAction* pAction = NULL, INT64 nParam = -1);
    };
}
