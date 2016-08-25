#pragma once

#include "DS_Common.h"

namespace EM
{
    class CLogLoader
    {
    public:

        CLogLoader();

        virtual ~CLogLoader();

        static void Init();

        //必须在配置加载后调用
        static void ResetLogLevelFromConfig();
    };
}