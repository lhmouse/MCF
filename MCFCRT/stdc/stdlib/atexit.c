// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/module.h"

typedef void (*__MCFCRT_AtExitProc)(void);

static void CrtAtEndModuleProc(intptr_t nContext){
	const __MCFCRT_AtExitProc pfnProc = (__MCFCRT_AtExitProc)nContext;
	(*pfnProc)();
}

int __wrap_atexit(__MCFCRT_AtExitProc func){
	if(!_MCFCRT_AtEndModule(&CrtAtEndModuleProc, (intptr_t)func)){
		return -1;
	}
	return 0;
}

__attribute__((__alias__("__wrap_atexit")))
int atexit(void (*func)(void));
