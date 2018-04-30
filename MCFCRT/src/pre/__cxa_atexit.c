// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "module.h"

int __cxa_atexit(void (*func)(void *), void *param, void *dso_handle){
	(void)dso_handle;
	return _MCFCRT_AtModuleExit((_MCFCRT_AtModuleExitCallback)(intptr_t)func, (intptr_t)param) ? 0 : -1;
}
