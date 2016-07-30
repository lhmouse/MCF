// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "crt_module.h"

bool _MCFCRT_AtCrtModuleExit(_MCFCRT_AtModuleExitCallback pfnProc, intptr_t nContext){
	return _MCFCRT_AtModuleExit(pfnProc, nContext);
}
