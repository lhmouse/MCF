// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/module.h"

int __wrap_atexit(void (__cdecl *func)(void)){
	return MCF_CRT_AtEndModule((void (__cdecl *)(intptr_t))func, 0);
}

__attribute__((__alias__("__wrap_atexit"))) int atexit(void (*func)(void));
