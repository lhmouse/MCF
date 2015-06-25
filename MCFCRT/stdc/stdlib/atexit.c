// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/module.h"

int __wrap_atexit(void (*func)(void)){
	// Windows 上 x86 __cdecl 和 x64 都约定调用者清栈，因此可以直接转换函数指针。
	return MCF_CRT_AtEndModule((void (*)(intptr_t))func, 0);
}

__attribute__((__alias__("__wrap_atexit")))
int atexit(void (*func)(void));
