// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_GTHREAD_H_
#define __MCFCRT_ENV_GTHREAD_H_

// Compatibility layer for GCC.

#include "_crtdef.h"
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
typedef _MCFCRT_TlsKeyHandle __gthread_key_t;

extern int __MCFCRT_gthread_key_create(__gthread_key_t *__key_ret, void (*__destructor)(void *)) _MCFCRT_NOEXCEPT;
#define __gthread_key_create __MCFCRT_gthread_key_create
extern int __MCFCRT_gthread_key_delete(__gthread_key_t __key) _MCFCRT_NOEXCEPT;
#define __gthread_key_delete __MCFCRT_gthread_key_delete
extern void *__MCFCRT_gthread_getspecific(__gthread_key_t __key) _MCFCRT_NOEXCEPT;
#define __gthread_getspecific __MCFCRT_gthread_getspecific
extern int __MCFCRT_gthread_setspecific(__gthread_key_t __key, const void *__value) _MCFCRT_NOEXCEPT;
#define __gthread_setspecific __MCFCRT_gthread_setspecific

//-----------------------------------------------------------------------------
// Once
//-----------------------------------------------------------------------------
typedef _MCFCRT_OnceFlag __gthread_once_t;

#define __GTHREAD_ONCE_INIT    { 0 }

extern int __MCFCRT_gthread_once(__gthread_once_t *__flag, void (*__func)(void)) _MCFCRT_NOEXCEPT;
#define __gthread_once __MCFCRT_gthread_once

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------
typedef _MCFCRT_Mutex __gthread_mutex_t;

#define __GTHREAD_MUTEX_INIT            { 0 }
#define __GTHREAD_MUTEX_INIT_FUNCTION   __gthread_mutex_init_function

extern void __MCFCRT_gthread_mutex_init_function(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT;
#define __gthread_mutex_init_function __MCFCRT_gthread_mutex_init_function
extern int __MCFCRT_gthread_mutex_destroy(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT;
#define __gthread_mutex_destroy __MCFCRT_gthread_mutex_destroy

extern int __MCFCRT_gthread_mutex_trylock(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT;
#define __gthread_mutex_trylock __MCFCRT_gthread_mutex_trylock
extern int __MCFCRT_gthread_mutex_lock(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT;
#define __gthread_mutex_lock __MCFCRT_gthread_mutex_lock
extern int __MCFCRT_gthread_mutex_unlock(__gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT;
#define __gthread_mutex_unlock __MCFCRT_gthread_mutex_unlock

//-----------------------------------------------------------------------------
// Recursive mutex
//-----------------------------------------------------------------------------
typedef struct __MCFCRT_tagGthreadRecursiveMutex {
	volatile _MCFCRT_STD uintptr_t __owner;
	_MCFCRT_STD size_t __count;
	__gthread_mutex_t __mutex;
} __gthread_recursive_mutex_t;

#define __GTHREAD_RECURSIVE_MUTEX_INIT            { 0, 0, __GTHREAD_MUTEX_INIT }
#define __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION   __gthread_recursive_mutex_init_function

extern void __MCFCRT_gthread_recursive_mutex_init_function(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT;
#define __gthread_recursive_mutex_init_function __MCFCRT_gthread_recursive_mutex_init_function
extern int __MCFCRT_gthread_recursive_mutex_destroy(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT;
#define __gthread_recursive_mutex_destroy __MCFCRT_gthread_recursive_mutex_destroy

extern int __MCFCRT_gthread_recursive_mutex_trylock(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT;
#define __gthread_recursive_mutex_trylock __MCFCRT_gthread_recursive_mutex_trylock
extern int __MCFCRT_gthread_recursive_mutex_lock(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT;
#define __gthread_recursive_mutex_lock __MCFCRT_gthread_recursive_mutex_lock
extern int __MCFCRT_gthread_recursive_mutex_unlock(__gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT;
#define __gthread_recursive_mutex_unlock __MCFCRT_gthread_recursive_mutex_unlock

//-----------------------------------------------------------------------------
// Condition variable
//-----------------------------------------------------------------------------
#define __GTHREAD_HAS_COND 1

typedef _MCFCRT_ConditionVariable __gthread_cond_t;

#define __GTHREAD_COND_INIT             { 0 }
#define __GTHREAD_COND_INIT_FUNCTION    __gthread_cond_init_function

extern void __MCFCRT_gthread_cond_init_function(__gthread_cond_t *__cond) _MCFCRT_NOEXCEPT;
#define __gthread_cond_init_function __MCFCRT_gthread_cond_init_function
extern int __MCFCRT_gthread_cond_destroy(__gthread_cond_t *__cond) _MCFCRT_NOEXCEPT;
#define __gthread_cond_destroy __MCFCRT_gthread_cond_destroy

extern int __MCFCRT_gthread_cond_wait(__gthread_cond_t *__cond, __gthread_mutex_t *__mutex) _MCFCRT_NOEXCEPT;
#define __gthread_cond_wait __MCFCRT_gthread_cond_wait
extern int __MCFCRT_gthread_cond_wait_recursive(__gthread_cond_t *__cond, __gthread_recursive_mutex_t *__recur_mutex) _MCFCRT_NOEXCEPT;
#define __gthread_cond_wait_recursive __MCFCRT_gthread_cond_wait_recursive
extern int __MCFCRT_gthread_cond_signal(__gthread_cond_t *__cond) _MCFCRT_NOEXCEPT;
#define __gthread_cond_signal __MCFCRT_gthread_cond_signal
extern int __MCFCRT_gthread_cond_broadcast(__gthread_cond_t *__cond) _MCFCRT_NOEXCEPT;
#define __gthread_cond_broadcast __MCFCRT_gthread_cond_broadcast

//-----------------------------------------------------------------------------
// Thread
//-----------------------------------------------------------------------------
#define __GTHREADS_CXX0X 1

typedef _MCFCRT_STD uintptr_t __gthread_t;

extern int __MCFCRT_gthread_create(__gthread_t *__tid_ret, void *(*__proc)(void *), void *__param) _MCFCRT_NOEXCEPT;
#define __gthread_create __MCFCRT_gthread_create
extern int __MCFCRT_gthread_join(__gthread_t __tid, void **restrict __exit_code_ret) _MCFCRT_NOEXCEPT;
#define __gthread_join __MCFCRT_gthread_join
extern int __MCFCRT_gthread_detach(__gthread_t __tid) _MCFCRT_NOEXCEPT;
#define __gthread_detach __MCFCRT_gthread_detach

extern int __MCFCRT_gthread_equal(__gthread_t __tid1, __gthread_t __tid2) _MCFCRT_NOEXCEPT;
#define __gthread_equal __MCFCRT_gthread_equal
__attribute__((__const__))
extern __gthread_t __MCFCRT_gthread_self(void) _MCFCRT_NOEXCEPT;
#define __gthread_self __MCFCRT_gthread_self
extern int __MCFCRT_gthread_yield(void) _MCFCRT_NOEXCEPT;
#define __gthread_yield __MCFCRT_gthread_yield

typedef struct __MCFCRT_tagGthreadTime {
	_MCFCRT_STD int64_t __seconds;
	long __nanoseconds;
} __gthread_time_t;

extern int __MCFCRT_gthread_mutex_timedlock(__gthread_mutex_t *restrict __mutex, const __gthread_time_t *restrict __timeout) _MCFCRT_NOEXCEPT;
#define __gthread_mutex_timedlock __MCFCRT_gthread_mutex_timedlock
extern int __MCFCRT_gthread_recursive_mutex_timedlock(__gthread_recursive_mutex_t *restrict __recur_mutex, const __gthread_time_t *restrict __timeout) _MCFCRT_NOEXCEPT;
#define __gthread_recursive_mutex_timedlock __MCFCRT_gthread_recursive_mutex_timedlock

extern int __MCFCRT_gthread_cond_timedwait(__gthread_cond_t *restrict __cond, __gthread_mutex_t *restrict __mutex, const __gthread_time_t *restrict __timeout) _MCFCRT_NOEXCEPT;
#define __gthread_cond_timedwait __MCFCRT_gthread_cond_timedwait

_MCFCRT_EXTERN_C_END

#ifndef __MCFCRT_GTHREAD_INLINE_OR_EXTERN
#	define __MCFCRT_GTHREAD_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif
#include "_gthread_inl.h"

#endif
