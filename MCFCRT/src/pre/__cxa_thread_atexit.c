// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "tls.h"

int __cxa_thread_atexit(void (*func)(void *), void *param, void *dso_handle){
	(void)dso_handle;
	return _MCFCRT_AtThreadExit((_MCFCRT_AtThreadExitCallback)func, (intptr_t)param) ? 0 : -1;
}
