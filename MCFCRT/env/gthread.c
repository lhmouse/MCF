// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "gthread.h"

void __MCFCRT_GthreadTlsDestructor(intptr_t nContext, void *pStorage){
	void (*const pfnDestructor)(void *) = (void (*)(void *))nContext;

	void *const pData = *(void **)pStorage;
	if(!pData){
		return;
	}
	(*pfnDestructor)(pData);
}

intptr_t __MCFCRT_GthreadUnlockCallbackMutex(intptr_t __context){
	__gthread_mutex_t *const __mutex = (__gthread_mutex_t *)__context;

	__gthread_mutex_unlock(__mutex);
	return 1;
}
void __MCFCRT_GthreadRelockCallbackMutex(intptr_t __context, intptr_t __unlocked){
	__gthread_mutex_t *const __mutex = (__gthread_mutex_t *)__context;

	_MCFCRT_ASSERT((size_t)__unlocked == 1);
	__gthread_mutex_lock(__mutex);
}

intptr_t __MCFCRT_GthreadUnlockCallbackRecursiveMutex(intptr_t __context){
	__gthread_recursive_mutex_t *const __recur_mutex = (__gthread_recursive_mutex_t *)__context;
	_MCFCRT_ASSERT(_MCFCRT_GetCurrentThreadId() == __atomic_load_n(&(__recur_mutex->__owner), __ATOMIC_RELAXED));

	const size_t __old_count = __recur_mutex->__count;
	__recur_mutex->__count = 0;
	__atomic_store_n(&(__recur_mutex->__owner), 0, __ATOMIC_RELAXED);

	__gthread_mutex_unlock(&(__recur_mutex->__mutex));
	return (intptr_t)__old_count;
}
void __MCFCRT_GthreadRelockCallbackRecursiveMutex(intptr_t __context, intptr_t __unlocked){
	__gthread_recursive_mutex_t *const __recur_mutex = (__gthread_recursive_mutex_t *)__context;

	_MCFCRT_ASSERT((size_t)__unlocked >= 1);
	__gthread_mutex_lock(&(__recur_mutex->__mutex));

	const uintptr_t __self = _MCFCRT_GetCurrentThreadId();
	__atomic_store_n(&(__recur_mutex->__owner), __self, __ATOMIC_RELAXED);
	__recur_mutex->__count = (size_t)__unlocked;
}
