// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/module.h"

static void CrtAtModuleExitProc(intptr_t nContext){
	void (*const pfnProc)(void) = (void (*)(void))nContext;
	(*pfnProc)();
}

int atexit(void (*func)(void)){
	if(!_MCFCRT_AtModuleExit(&CrtAtModuleExitProc, (intptr_t)func)){
		return -1;
	}
	return 0;
}

__attribute__((__alias__("atexit")))
int __wrap_atexit(void (*func)(void));
