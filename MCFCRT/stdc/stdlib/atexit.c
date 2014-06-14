// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/module.h"

static void AtExitWrapper(intptr_t nContext){
	(*((void (*)(void))nContext))();
}

int atexit(void (*func)(void)){
	return MCF_CRT_AtEndModule(&AtExitWrapper, (intptr_t)func);
}
