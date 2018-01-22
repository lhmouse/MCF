// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#define __MCFCRT_GTHREAD_TLS_INLINE_OR_EXTERN     extern inline
#include "gthread_tls.h"

void __MCFCRT_gthread_tls_destructor(intptr_t context, void *storage){
	void (*const destructor)(void *) = (void (*)(void *))context;

	void *const value = *(void **)storage;
	if(!value){
		return;
	}
	*(void **)storage = _MCFCRT_NULLPTR;
	(*destructor)(value);
}
