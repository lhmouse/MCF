// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_GTHREAD_H_
#define __MCFCRT_ENV_GTHREAD_H_

// 专门为 GCC 定制的兼容层。

#include "_crtdef.h"
#include <errno.h>
#include "../ext/assert.h"
#include "../ext/expect.h"
#include "thread_env.h"
#include "thread.h"
#include "once_flag.h"
#include "mutex.h"
#include "condition_variable.h"
#include "clocks.h"

#ifdef __GTHREADS
#	error __GTHREADS is already defined. (Thread model confliction detected?)
#endif

#define __GTHREADS 1

_MCFCRT_EXTERN_C_BEGIN

_MCFCRT_CONSTEXPR int __gthread_active_p(void) _MCFCRT_NOEXCEPT {
	return 1;
}

//-----------------------------------------------------------------------------
// Thread local storage
//-----------------------------------------------------------------------------
extern void __MCFCRT_GthreadTlsDestructor(_MCFCRT_STD intptr_t __nContext, void *__pStorage) _MCFCRT_NOEXCEPT;

typedef _MCFCRT_TlsKeyHandle __gthread_key_t;

static inline int __gthread_key_create(__gthread_key_t *__key_ret, void (*__destructor)(void *)) _MCFCRT_NOEXCEPT {
	const __gthread_key_t __key = _MCFCRT_TlsAllocKey(sizeof(void *), nullptr, __destructor ? &__MCFCRT_GthreadTlsDestructor : nullptr, (_MCFCRT_STD intptr_t)__destructor);
	if(!__key){
		return ENOMEM;
	}
	*__key_ret = __key;
	return 0;
}
static inline int __gthread_key_delete(__gthread_key_t __key) _MCFCRT_NOEXCEPT {
	_MCFCRT_TlsFreeKey(__key);
	return 0;
}
static inline void *__gthread_getspecific(__gthread_key_t __key) _MCFCRT_NOEXCEPT {
	void *__storage;
	const bool __success = _MCFCRT_TlsGet(__key, &__storage);
	if(!__success){
		return nullptr;
	}
	if(!__storage){
		return nullptr;
	}
	return *(void **)__storage;
}
static inline int __gthread_setspecific(__gthread_key_t __key, const void *__value) _MCFCRT_NOEXCEPT {
	void *__storage;
	const bool __success = _MCFCRT_TlsRequire(__key, &__storage);
	if(!__success){
		return ENOMEM;
	}
	_MCFCRT_ASSERT(__storage);
	*(void **)__storage = (void *)__value;
	return 0;
}

//-----------------------------------------------------------------------------
// Once
//-----------------------------------------------------------------------------
typedef _MCFCRT_OnceFlag __gthread_once_t;

#define __GTHREAD_ONCE_INIT    { 0 }

static inline int __gthread_once(__gthread_once_t *__flag, void (*__func)(void)) _MCFCRT_NOEXCEPT {
	const _MCFCRT_OnceResult __result = _MCFCRT_WaitForOnceFlagForever(__flag);
	if(_MCFCRT_EXPECT(__result == _MCFCRT_kOnceResultFinished)){
		return 0;
	}
	_MCFCRT_ASSERT(__result == _MCFCRT_kOnceResultInitial);
	(*__func)();
	_MCFCRT_SignalOnceFlagAsFinished(__flag);
	return 0;
}

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------
typedef _MCFCRT_Mutex __gthread_mutex_t;

#define __GTHREAD_MUTEX_INIT            { 0 }
#define __GTHREAD_MUTEX_INIT_FUNCTION   __gthread_mutex_init_function

static inline void __gthread_mutex_init_function(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT {
	_MCFCRT_InitializeMutex(__mutex);
}
static inline int __gthread_mutex_destroy(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT {
	(void)__mutex;
	return 0;
}

static inline int __gthread_mutex_trylock(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT {
	const bool __success = _MCFCRT_WaitForMutex(__mutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, 0);
	if(_MCFCRT_EXPECT_NOT(!__success)){
		return EBUSY;
	}
	return 0;
}
static inline int __gthread_mutex_lock(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForMutexForever(__mutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	return 0;
}
static inline int __gthread_mutex_unlock(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT {
	_MCFCRT_SignalMutex(__mutex);
	return 0;
}

//-----------------------------------------------------------------------------
// Recursive mutex
//-----------------------------------------------------------------------------
typedef struct __MCFCRT_tagGthreadRecursiveMutex {
	volatile _MCFCRT_STD uintptr_t __owner;
	_MCFCRT_STD size_t __count;
	__gthread_mutex_t __mutex;
} __gthread_recursive_mutex_t;

#define __GTHREAD_RECURSIVE_MUTEX_INIT            { 0, 0, { 0 } }
#define __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION   __gthread_recursive_mutex_init_function

static inline void __gthread_recursive_mutex_init_function(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT {
	__recur_mutex->__owner = 0;
	__recur_mutex->__count = 0;
	__gthread_mutex_init_function(&(__recur_mutex->__mutex)); // This guarantees release semantics so we need no atomic operations above.
}
static inline int __gthread_recursive_mutex_destroy(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT {
	__gthread_mutex_destroy(&(__recur_mutex->__mutex));
	return 0;
}

static inline int __gthread_recursive_mutex_trylock(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uintptr_t __self = _MCFCRT_GetCurrentThreadId();
	const _MCFCRT_STD uintptr_t __old_owner = __atomic_load_n(&(__recur_mutex->__owner), __ATOMIC_RELAXED);
	if(_MCFCRT_EXPECT_NOT(__old_owner != __self)){
		if(__old_owner != 0){
			return EBUSY;
		}
		const int __error = __gthread_mutex_trylock(&(__recur_mutex->__mutex));
		if(__error != 0){
			return __error;
		}
		__atomic_store_n(&(__recur_mutex->__owner), __self, __ATOMIC_RELAXED);
	}
	const _MCFCRT_STD size_t __new_count = ++__recur_mutex->__count;
	_MCFCRT_ASSERT(__new_count != 0);
	return 0;
}
static inline int __gthread_recursive_mutex_lock(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uintptr_t __self = _MCFCRT_GetCurrentThreadId();
	const _MCFCRT_STD uintptr_t __old_owner = __atomic_load_n(&(__recur_mutex->__owner), __ATOMIC_RELAXED);
	if(_MCFCRT_EXPECT_NOT(__old_owner != __self)){
		__gthread_mutex_lock(&(__recur_mutex->__mutex));
		__atomic_store_n(&(__recur_mutex->__owner), __self, __ATOMIC_RELAXED);
	}
	const _MCFCRT_STD size_t __new_count = ++__recur_mutex->__count;
	_MCFCRT_ASSERT(__new_count != 0);
	return 0;
}
static inline int __gthread_recursive_mutex_unlock(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT {
	_MCFCRT_ASSERT(_MCFCRT_GetCurrentThreadId() == __atomic_load_n(&(__recur_mutex->__owner), __ATOMIC_RELAXED));
	const _MCFCRT_STD size_t __new_count = --__recur_mutex->__count;
	if(_MCFCRT_EXPECT_NOT(__new_count == 0)){
		__atomic_store_n(&(__recur_mutex->__owner), 0, __ATOMIC_RELAXED);
		__gthread_mutex_unlock(&(__recur_mutex->__mutex));
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Condition variable
//-----------------------------------------------------------------------------
#define __GTHREAD_HAS_COND 1

extern _MCFCRT_STD intptr_t __MCFCRT_GthreadUnlockCallbackMutex(_MCFCRT_STD intptr_t __context) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_GthreadRelockCallbackMutex(_MCFCRT_STD intptr_t __context, _MCFCRT_STD intptr_t __unlocked) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD intptr_t __MCFCRT_GthreadUnlockCallbackRecursiveMutex(_MCFCRT_STD intptr_t __context) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_GthreadRelockCallbackRecursiveMutex(_MCFCRT_STD intptr_t __context, _MCFCRT_STD intptr_t __unlocked) _MCFCRT_NOEXCEPT;

typedef _MCFCRT_ConditionVariable __gthread_cond_t;

#define __GTHREAD_COND_INIT             { 0 }
#define __GTHREAD_COND_INIT_FUNCTION    __gthread_cond_init_function

static inline void __gthread_cond_init_function(__gthread_cond_t *__cond) _MCFCRT_NOEXCEPT {
	_MCFCRT_InitializeConditionVariable(__cond);
}
static inline int __gthread_cond_destroy(__gthread_cond_t *__cond) _MCFCRT_NOEXCEPT {
	(void)__cond;
	return 0;
}

static inline int __gthread_cond_wait(__gthread_cond_t *__cond, __gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForConditionVariableForever(__cond, &__MCFCRT_GthreadUnlockCallbackMutex, &__MCFCRT_GthreadRelockCallbackMutex, (_MCFCRT_STD intptr_t)__mutex);
	return 0;
}
static inline int __gthread_cond_wait_recursive(__gthread_cond_t *__cond, __gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForConditionVariableForever(__cond, &__MCFCRT_GthreadUnlockCallbackRecursiveMutex, &__MCFCRT_GthreadRelockCallbackRecursiveMutex, (_MCFCRT_STD intptr_t)__recur_mutex);
	return 0;
}
static inline int __gthread_cond_signal(__gthread_cond_t *__cond) _MCFCRT_NOEXCEPT {
	_MCFCRT_SignalConditionVariable(__cond, 1);
	return 0;
}
static inline int __gthread_cond_broadcast(__gthread_cond_t *__cond) _MCFCRT_NOEXCEPT {
	_MCFCRT_BroadcastConditionVariable(__cond);
	return 0;
}

//-----------------------------------------------------------------------------
// Thread
//-----------------------------------------------------------------------------
#define __GTHREADS_CXX0X 1

typedef _MCFCRT_STD uintptr_t __gthread_t;

extern _MCFCRT_STD uintptr_t __MCFCRT_GthreadCreateJoinable(void *(*__proc)(void *), void *__param) _MCFCRT_NOEXCEPT;
extern bool __MCFCRT_GthreadJoin(_MCFCRT_STD uintptr_t __tid, void **restrict __exit_code_ret) _MCFCRT_NOEXCEPT;
extern bool __MCFCRT_GthreadDetach(_MCFCRT_STD uintptr_t __tid) _MCFCRT_NOEXCEPT;

static inline int __gthread_create(__gthread_t *__tid_ret, void *(*__proc)(void *), void *__param) _MCFCRT_NOEXCEPT {
	const __gthread_t __tid = __MCFCRT_GthreadCreateJoinable(__proc, __param);
	if(__tid == 0){
		return EAGAIN;
	}
	*__tid_ret = __tid;
	return 0;
}
static inline int __gthread_join(__gthread_t __tid, void **restrict __exit_code_ret) _MCFCRT_NOEXCEPT {
	if(__tid == _MCFCRT_GetCurrentThreadId()){
		return EDEADLK;
	}
	if(!__MCFCRT_GthreadJoin(__tid, __exit_code_ret)){
		return ESRCH;
	}
	return 0;
}
static inline int __gthread_detach(__gthread_t __tid) _MCFCRT_NOEXCEPT {
	if(__tid == _MCFCRT_GetCurrentThreadId()){
		return EDEADLK;
	}
	if(!__MCFCRT_GthreadDetach(__tid)){
		return ESRCH;
	}
	return 0;
}

_MCFCRT_CONSTEXPR int __gthread_equal(__gthread_t __tid1, __gthread_t __tid2) _MCFCRT_NOEXCEPT {
	return __tid1 == __tid2;
}
static inline __gthread_t __gthread_self(void) _MCFCRT_NOEXCEPT {
	return _MCFCRT_GetCurrentThreadId();
}
static inline int __gthread_yield(void) _MCFCRT_NOEXCEPT {
	_MCFCRT_YieldThread();
	return 0;
}

typedef struct __MCFCRT_tagGthreadTime {
	_MCFCRT_STD int64_t __seconds;
	long __nanoseconds;
} __gthread_time_t;

static inline _MCFCRT_STD uint64_t __MCFCRT_GthreadTranslateTimeout(const __gthread_time_t *__utc_timeout) _MCFCRT_NOEXCEPT {
	const double __utc_timeout_ms = (double)__utc_timeout->__seconds * 1.0e3 + (double)__utc_timeout->__nanoseconds / 1.0e6;
	const double __utc_now_ms = (double)_MCFCRT_GetUtcClock();
	const double __delta_ms = __utc_timeout_ms - __utc_now_ms;
	if(__delta_ms <= 0){
		return 0;
	}
	const _MCFCRT_STD uint64_t __mono_now_ms = _MCFCRT_GetFastMonoClock();
	const _MCFCRT_STD uint64_t __complement_ms = (1ull << 48) - 1 - __mono_now_ms;
	if(__delta_ms >= (double)__complement_ms){
		return (_MCFCRT_STD uint64_t)-1;
	}
	return __mono_now_ms + (_MCFCRT_STD uint64_t)__delta_ms;
}

static inline int __gthread_mutex_timedlock(__gthread_mutex_t *__mutex, const __gthread_time_t *__timeout) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uint64_t __mono_timeout_ms = __MCFCRT_GthreadTranslateTimeout(__timeout);
	if(!_MCFCRT_WaitForMutex(__mutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, __mono_timeout_ms)){
		return ETIMEDOUT;
	}
	return 0;
}
static inline int __gthread_recursive_mutex_timedlock(__gthread_recursive_mutex_t *__recur_mutex, const __gthread_time_t *__timeout) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uintptr_t __self = _MCFCRT_GetCurrentThreadId();
	const _MCFCRT_STD uintptr_t __old_owner = __atomic_load_n(&(__recur_mutex->__owner), __ATOMIC_RELAXED);
	if(_MCFCRT_EXPECT_NOT(__old_owner != __self)){
		if(__old_owner != 0){
			return EBUSY;
		}
		const int __error = __gthread_mutex_timedlock(&(__recur_mutex->__mutex), __timeout);
		if(__error != 0){
			return __error;
		}
		__atomic_store_n(&(__recur_mutex->__owner), __self, __ATOMIC_RELAXED);
	}
	const _MCFCRT_STD size_t __new_count = ++__recur_mutex->__count;
	_MCFCRT_ASSERT(__new_count != 0);
	return 0;
}

static inline int __gthread_cond_timedwait(__gthread_cond_t *__cond, __gthread_mutex_t *__mutex, const __gthread_time_t *__timeout) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uint64_t __mono_timeout_ms = __MCFCRT_GthreadTranslateTimeout(__timeout);
	if(!_MCFCRT_WaitForConditionVariable(__cond, &__MCFCRT_GthreadUnlockCallbackMutex, &__MCFCRT_GthreadRelockCallbackMutex, (_MCFCRT_STD intptr_t)__mutex, __mono_timeout_ms)){
		return ETIMEDOUT;
	}
	return 0;
}

_MCFCRT_EXTERN_C_END

#endif
