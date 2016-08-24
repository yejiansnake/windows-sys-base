#pragma once

#include "../../SysBase/SysBase/SysBase_Interface.h"   

using namespace SysBase;

#ifdef _DEBUG
#ifdef X64
#pragma comment(lib, "../../SysBase/lib/SysBase_d_64.lib")
#else
#pragma comment(lib, "../../SysBase/lib/SysBase_d.lib")
#endif
#else
#ifdef X64
#pragma comment(lib, "../../SysBase/lib/SysBase_64.lib")
#else
#pragma comment(lib, "../../SysBase/lib/SysBase.lib")
#endif
#endif // _DEBUG