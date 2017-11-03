// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_C11THREAD_H_
#define __MCFCRT_ENV_C11THREAD_H_

// Compatibility layer for the ISO/IEC C11 standard.

#include "_crtdef.h"
#include "_mopthread.h"
#include "once_flag.h"
#include "mutex.h"
#include "condition_variable.h"
#include "clocks.h"
#include "xassert.h"
#include "expect.h"
#include <time.h> // Inclusion of <time.h> is mandatory according to ISO C.
#include <errno.h>

#ifndef __MCFCRT_C11THREAD_INLINE_OR_EXTERN
#  define __MCFCRT_C11THREAD_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

//-----------------------------------------------------------------------------
// 7.26.1 Introduction
//-----------------------------------------------------------------------------
#define ONCE_FLAG_INIT         { 0 }

// Thread
typedef _MCFCRT_STD uintptr_t thrd_t;
typedef int (*thrd_start_t)(void *);

// Condition variable
typedef struct __MCFCRT_c11thread_cnd {
	_MCFCRT_ConditionVariable __cond;
	_MCFCRT_STD uintptr_t __reserved[3];
} cnd_t;

// Mutex
typedef struct __MCFCRT_c11thread_mtx {
	int __mask;
	thrd_t __owner;
	_MCFCRT_STD size_t __count;
	_MCFCRT_Mutex __mutex;
	_MCFCRT_STD uintptr_t __reserved[3];
} mtx_t;

// Call once
// Note: This struct conforms to the Itanium ABI.
typedef struct __MCFCRT_c11thread_once_flag {
	_MCFCRT_OnceFlag __once;
} once_flag;

// Enumeration constants
enum {
	mtx_plain     = 0x0000,
	mtx_recursive = 0x0001,
	mtx_timed     = 0x0002,

	// These error codes are listed in the same order as that in N1570.
	thrd_timedout = ETIMEDOUT,
	thrd_success  = 0,
	thrd_busy     = EBUSY,
	thrd_error    = EPERM, // XXX: Why do we need this error?
	thrd_nomem    = ENOMEM,
};

//-----------------------------------------------------------------------------
// 7.26.2 Initialization functions
//-----------------------------------------------------------------------------
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_call_once(once_flag *__once, void (*__func)(void)) _MCFCRT_NOEXCEPT {
	const _MCFCRT_OnceResult __result = _MCFCRT_WaitForOnceFlagForever(&(__once->__once));
	if(_MCFCRT_EXPECT(__result == _MCFCRT_kOnceResultFinished)){
		return;
	}
	_MCFCRT_ASSERT(__result == _MCFCRT_kOnceResultInitial);
	(*__func)();
	_MCFCRT_SignalOnceFlagAsFinished(&(__once->__once));
}

#define call_once           __MCFCRT_call_once

//-----------------------------------------------------------------------------
// 7.26.3 Condition variable functions
//-----------------------------------------------------------------------------
extern _MCFCRT_STD intptr_t __MCFCRT_c11thread_unlock_callback_mutex(_MCFCRT_STD intptr_t __context) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_c11thread_relock_callback_mutex(_MCFCRT_STD intptr_t __context, _MCFCRT_STD intptr_t __unlocked) _MCFCRT_NOEXCEPT;

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_init(cnd_t *__cond) _MCFCRT_NOEXCEPT {
	_MCFCRT_InitializeConditionVariable(&(__cond->__cond));
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_cnd_destroy(cnd_t *__cond) _MCFCRT_NOEXCEPT {
	(void)__cond;
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN _MCFCRT_STD uint64_t __MCFCRT_c11thread_translate_timeout(const struct timespec *_MCFCRT_RESTRICT __utc_timeout) _MCFCRT_NOEXCEPT {
	const double __utc_timeout_ms = (double)__utc_timeout->tv_sec * 1.0e3 + (double)__utc_timeout->tv_nsec / 1.0e6;
	const double __utc_now_ms = (double)_MCFCRT_GetUtcClock();
	const double __delta_ms = __utc_timeout_ms - __utc_now_ms;
	if(__delta_ms <= 0){
		return 0;
	}
	const _MCFCRT_STD uint64_t __mono_now_ms = _MCFCRT_GetFastMonoClock();
	// XXX: We assume the value returned by _MCFCRT_GetFastMonoClock() is the number of milliseconds since the system starts, which can hardly be greater than 0x1p48.
	//      All current implementations, especially clock_gettime() on Linux, are specified as that. This isn't guaranteed behavior nevertheless.
	const _MCFCRT_STD uint64_t __complement_ms = (1ull << 48) - 1 - __mono_now_ms;
	// Cast the uint64_t to int64_t for performance reasons. The compiler would otherwise produce more code since unsigned types can't be processed by x87 directly.
	if(__delta_ms >= (double)(_MCFCRT_STD int64_t)__complement_ms){
		return (_MCFCRT_STD uint64_t)-1;
	}
	return __mono_now_ms + (_MCFCRT_STD uint64_t)(_MCFCRT_STD int64_t)(__delta_ms + 0.999999);
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_timedwait(cnd_t *_MCFCRT_RESTRICT __cond, mtx_t *_MCFCRT_RESTRICT __mutex, const struct timespec *_MCFCRT_RESTRICT __timeout) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uint64_t __mono_timeout_ms = __MCFCRT_c11thread_translate_timeout(__timeout);
	if(!_MCFCRT_WaitForConditionVariable(&(__cond->__cond), &__MCFCRT_c11thread_unlock_callback_mutex, &__MCFCRT_c11thread_relock_callback_mutex, (_MCFCRT_STD intptr_t)__mutex, _MCFCRT_CONDITION_VARIABLE_SUGGESTED_SPIN_COUNT, __mono_timeout_ms)){
		return thrd_timedout;
	}
	return thrd_success;
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_wait(cnd_t *_MCFCRT_RESTRICT __cond, mtx_t *_MCFCRT_RESTRICT __mutex) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForConditionVariableForever(&(__cond->__cond), &__MCFCRT_c11thread_unlock_callback_mutex, &__MCFCRT_c11thread_relock_callback_mutex, (_MCFCRT_STD intptr_t)__mutex, _MCFCRT_CONDITION_VARIABLE_SUGGESTED_SPIN_COUNT);
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_signal(cnd_t *__cond) _MCFCRT_NOEXCEPT {
	_MCFCRT_SignalConditionVariable(&(__cond->__cond), 1);
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_cnd_broadcast(cnd_t *__cond) _MCFCRT_NOEXCEPT {
	_MCFCRT_BroadcastConditionVariable(&(__cond->__cond));
	return thrd_success;
}

#define cnd_init        __MCFCRT_cnd_init
#define cnd_destroy     __MCFCRT_cnd_destroy

#define cnd_timedwait   __MCFCRT_cnd_timedwait
#define cnd_wait        __MCFCRT_cnd_wait
#define cnd_signal      __MCFCRT_cnd_signal
#define cnd_broadcast   __MCFCRT_cnd_broadcast

//-----------------------------------------------------------------------------
// 7.26.4 Mutex functions
//-----------------------------------------------------------------------------
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_init(mtx_t *__mutex, int __mask) _MCFCRT_NOEXCEPT {
	__mutex->__mask  = __mask;
	__mutex->__owner = 0;
	__mutex->__count = 0;
	_MCFCRT_InitializeMutex(&(__mutex->__mutex));
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_mtx_destroy(mtx_t *__mutex) _MCFCRT_NOEXCEPT {
	(void)__mutex;
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_lock(mtx_t *__mutex) _MCFCRT_NOEXCEPT {
	if(__mutex->__mask & mtx_recursive){
		const _MCFCRT_STD uintptr_t __self = _MCFCRT_GetCurrentThreadId();
		const _MCFCRT_STD uintptr_t __old_owner = __atomic_load_n(&(__mutex->__owner), __ATOMIC_RELAXED);
		if(_MCFCRT_EXPECT_NOT(__old_owner != __self)){
			_MCFCRT_WaitForMutexForever(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
			__atomic_store_n(&(__mutex->__owner), __self, __ATOMIC_RELAXED);
		}
		const _MCFCRT_STD size_t __new_count = ++__mutex->__count;
		_MCFCRT_ASSERT(__new_count != 0);
	} else {
		_MCFCRT_WaitForMutexForever(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	}
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_timedlock(mtx_t *_MCFCRT_RESTRICT __mutex, const struct timespec *_MCFCRT_RESTRICT __timeout) _MCFCRT_NOEXCEPT  {
	if(!(__mutex->__mask & mtx_timed)){
		return thrd_error;
	}
	if(__mutex->__mask & mtx_recursive){
		const _MCFCRT_STD uintptr_t __self = _MCFCRT_GetCurrentThreadId();
		const _MCFCRT_STD uintptr_t __old_owner = __atomic_load_n(&(__mutex->__owner), __ATOMIC_RELAXED);
		if(_MCFCRT_EXPECT_NOT(__old_owner != __self)){
			const _MCFCRT_STD uint64_t __mono_timeout_ms = __MCFCRT_c11thread_translate_timeout(__timeout);
			if(!_MCFCRT_WaitForMutex(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, __mono_timeout_ms)){
				return thrd_timedout;
			}
			__atomic_store_n(&(__mutex->__owner), __self, __ATOMIC_RELAXED);
		}
		const _MCFCRT_STD size_t __new_count = ++__mutex->__count;
		_MCFCRT_ASSERT(__new_count != 0);
	} else {
		const _MCFCRT_STD uint64_t __mono_timeout_ms = __MCFCRT_c11thread_translate_timeout(__timeout);
		if(!_MCFCRT_WaitForMutex(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, __mono_timeout_ms)){
			return thrd_timedout;
		}
	}
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_trylock(mtx_t *__mutex) _MCFCRT_NOEXCEPT {
	if(__mutex->__mask & mtx_recursive){
		const _MCFCRT_STD uintptr_t __self = _MCFCRT_GetCurrentThreadId();
		const _MCFCRT_STD uintptr_t __old_owner = __atomic_load_n(&(__mutex->__owner), __ATOMIC_RELAXED);
		if(_MCFCRT_EXPECT_NOT(__old_owner != __self)){
			if(_MCFCRT_EXPECT_NOT(!_MCFCRT_WaitForMutex(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, 0))){
				return thrd_busy;
			}
			__atomic_store_n(&(__mutex->__owner), __self, __ATOMIC_RELAXED);
		}
		const _MCFCRT_STD size_t __new_count = ++__mutex->__count;
		_MCFCRT_ASSERT(__new_count != 0);
	} else {
		if(_MCFCRT_EXPECT_NOT(!_MCFCRT_WaitForMutex(&(__mutex->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT, 0))){
			return thrd_busy;
		}
	}
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_mtx_unlock(mtx_t *__mutex) _MCFCRT_NOEXCEPT {
	if(__mutex->__mask & mtx_recursive){
		_MCFCRT_DEBUG_CHECK(_MCFCRT_GetCurrentThreadId() == __atomic_load_n(&(__mutex->__owner), __ATOMIC_RELAXED));
		const _MCFCRT_STD size_t __new_count = --__mutex->__count;
		if(_MCFCRT_EXPECT_NOT(__new_count == 0)){
			__atomic_store_n(&(__mutex->__owner), 0, __ATOMIC_RELAXED);
			_MCFCRT_SignalMutex(&(__mutex->__mutex));
		}
	} else {
		_MCFCRT_SignalMutex(&(__mutex->__mutex));
	}
	return thrd_success;
}

#define mtx_init      __MCFCRT_mtx_init
#define mtx_destroy   __MCFCRT_mtx_destroy

#define mtx_lock      __MCFCRT_mtx_lock
#define mtx_timedlock __MCFCRT_mtx_timedlock
#define mtx_trylock   __MCFCRT_mtx_trylock
#define mtx_unlock    __MCFCRT_mtx_unlock

//-----------------------------------------------------------------------------
// 7.26.5 Thread functions
//-----------------------------------------------------------------------------
typedef struct __MCFCRT_c11thread_control {
	int (*__proc)(void *);
	void *__param;
	int __exit_code;
} __MCFCRT_c11thread_control_t;

extern void __MCFCRT_c11thread_mopthread_wrapper(void *__params) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_c11thread_mopthread_exit_modifier(void *__params, _MCFCRT_STD size_t __size_of_params, _MCFCRT_STD intptr_t __context) _MCFCRT_NOEXCEPT;

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_create(thrd_t *__tid_ret, thrd_start_t __proc, void *__param) _MCFCRT_NOEXCEPT {
	__MCFCRT_c11thread_control_t __control = { __proc, __param, (int)0xDEADBEEF };
	const _MCFCRT_STD uintptr_t __tid = __MCFCRT_MopthreadCreate(&__MCFCRT_c11thread_mopthread_wrapper, &__control, sizeof(__control));
	if(__tid == 0){
		return thrd_nomem; // XXX: We should have returned `EAGAIN` as what POSIX has specified but there isn't `thrd_again` in ISO C.
	}
	*__tid_ret = __tid;
	return thrd_success;
}
__attribute__((__noreturn__))
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_thrd_exit(int __exit_code) _MCFCRT_NOEXCEPT {
	__MCFCRT_MopthreadExit(&__MCFCRT_c11thread_mopthread_exit_modifier, __exit_code);
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_join(thrd_t __tid, int *__exit_code_ret) _MCFCRT_NOEXCEPT {
	if(__tid == _MCFCRT_GetCurrentThreadId()){
		return thrd_error; // XXX: EDEADLK
	}
	if(__exit_code_ret){
		__MCFCRT_c11thread_control_t __control;
		__builtin_memset(&__control, 0, sizeof(__control));
		_MCFCRT_STD size_t __bytes_copied = sizeof(__control);
		if(!__MCFCRT_MopthreadJoin(__tid, &__control, &__bytes_copied)){
			return thrd_error; // XXX: ESRCH
		}
		*__exit_code_ret = __control.__exit_code;
	} else {
		if(!__MCFCRT_MopthreadJoin(__tid, _MCFCRT_NULLPTR, _MCFCRT_NULLPTR)){
			return thrd_error; // XXX: ESRCH
		}
	}
	return thrd_success;
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_detach(thrd_t __tid) _MCFCRT_NOEXCEPT {
	if(__tid == _MCFCRT_GetCurrentThreadId()){
		return thrd_error; // XXX: EDEADLK
	}
	if(!__MCFCRT_MopthreadDetach(__tid)){
		return thrd_error; // XXX: ESRCH
	}
	return thrd_success;
}

__attribute__((__const__))
__MCFCRT_C11THREAD_INLINE_OR_EXTERN thrd_t __MCFCRT_thrd_current(void) _MCFCRT_NOEXCEPT {
	return _MCFCRT_GetCurrentThreadId();
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_equal(thrd_t __tid1, thrd_t __tid2) _MCFCRT_NOEXCEPT {
	return __tid1 == __tid2;
}

__MCFCRT_C11THREAD_INLINE_OR_EXTERN int __MCFCRT_thrd_sleep(const struct timespec *__duration, struct timespec *__remaining) _MCFCRT_NOEXCEPT {
	const double __delta_ms = (double)__duration->tv_sec * 1.0e3 + (double)__duration->tv_nsec / 1.0e6;
	const _MCFCRT_STD uint64_t __mono_begin_ms = _MCFCRT_GetFastMonoClock();
	const _MCFCRT_STD uint64_t __complement_ms = (1ull << 48) - 1 - __mono_begin_ms;
	_MCFCRT_STD uint64_t __mono_timeout_ms;
	if(__delta_ms >= (double)__complement_ms){
		__mono_timeout_ms = (_MCFCRT_STD uint64_t)-1;
	} else {
		__mono_timeout_ms = __mono_begin_ms + (_MCFCRT_STD uint64_t)__delta_ms;
	}
	_MCFCRT_Sleep(__mono_timeout_ms);

	if(__remaining){
		const _MCFCRT_STD uint64_t __mono_end_ms = _MCFCRT_GetFastMonoClock();
		const double __slept_ms = (double)__mono_end_ms - (double)__mono_begin_ms;
		const double __remaining_s = (__delta_ms - __slept_ms) / 1.0e3;

		long long __sec = 0;
		long __nsec = 0;
		if(__remaining_s > 0){
			__sec = (long long)__remaining_s;
			__nsec = (long)((__remaining_s - (double)__sec) * 1.0e9);
		}
		__remaining->tv_sec  = (_MCFCRT_STD time_t)__sec;
		__remaining->tv_nsec = __nsec;
	}
	return 0; // On Windows we don't support interruptable sleep.
}
__MCFCRT_C11THREAD_INLINE_OR_EXTERN void __MCFCRT_thrd_yield(void) _MCFCRT_NOEXCEPT {
	_MCFCRT_YieldThread();
}

#define thrd_create   __MCFCRT_thrd_create
#define thrd_exit     __MCFCRT_thrd_exit
#define thrd_join     __MCFCRT_thrd_join
#define thrd_detach   __MCFCRT_thrd_detach

#define thrd_current  __MCFCRT_thrd_current
#define thrd_equal    __MCFCRT_thrd_equal

#define thrd_sleep    __MCFCRT_thrd_sleep
#define thrd_yield    __MCFCRT_thrd_yield

_MCFCRT_EXTERN_C_END

#endif
