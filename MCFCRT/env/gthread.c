// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "gthread.h"

unsigned long __MCFCRT_GthreadTlsCostructor(intptr_t nContext, void *pStorage){
	(void)nContext;

	*(void **)pStorage = nullptr;
	return 0;
}
void __MCFCRT_GthreadTlsDestructor(intptr_t nContext, void *pStorage){
	void (*const pfnDestructor)(void *) = (void (*)(void *))nContext;

	void *const pData = *(void **)pStorage;
	if(!pData){
		return;
	}
	(*pfnDestructor)(pData);
}
