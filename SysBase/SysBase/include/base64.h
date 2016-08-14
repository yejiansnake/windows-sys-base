#pragma once

#include "SysBase.h"

namespace SysBase
{
    class CBase64
    {
    public:
        static UINT32 Encode(const void* pSource, UINT32 nSourceSize, char* pDest, UINT32 nDestLen);

        static UINT32 Decode(char const* pSource, UINT32 nSourceLen, void* pDest, UINT32 nDestSize);

        static UINT32 SafeUrlEncode(const void* pSource, UINT32 nSourceSize, char* pDest, UINT32 nDestLen);

        static UINT32 SafeUrlDecode(char const* pSource, UINT32 nSourceLen, void* pDest, UINT32 nDestSize);
    };

}
