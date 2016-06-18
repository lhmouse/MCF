// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_C11THREAD_H_
#define __MCFCRT_ENV_C11THREAD_H_

#ifndef __STDC_NO_THREADS__

// Compatibility layer for the ISO/IEC C11 standard.

#include "_crtdef.h"
#include "thread_env.h"
#include "thread.h"
#include "once_flag.h"
#include "mutex.h"
#include "condition_variable.h"
#include "clocks.h"

// Inclusion of <time.h> is mandatory according to ISO C.
#include <time.h>
#include <errno.h>

_MCFCRT_EXTERN_C_BEGIN

// 7.26.1 Introduction
#if !defined(__cplusplus) || (__cplusplus < 201103l)
#	define thread_local        _Thread_local
#endif

#define ONCE_FLAG_INIT         { 0 }
#define TSS_DTOR_ITERATIONS    1      // XXX: Do we really need to support this crap?

// Thread
typedef _MCFCRT_STD uintptr_t thrd_t;
typedef int (*thrd_start_t)(void *);

// Condition variable
typedef struct __MCFCRT_tagC11threadConditionVariable {
	union {
		_MCFCRT_STD uintptr_t __details[4]; // Placeholder for implementation-specific details.
		_MCFCRT_ConditionVariable __cond[1];
	};
} cnd_t;

// Mutex
typedef struct __MCFCRT_tagC11threadMutex {
	int __mask;
	volatile thrd_t __owner;
	size_t __count;
	union {
		_MCFCRT_STD uintptr_t __details[4]; // Placeholder for implementation-specific details.
		_MCFCRT_Mutex __mutex[1];
	};
} mtx_t;

// Thread specific storage
typedef _MCFCRT_TlsKeyHandle tss_t; // typedef void *tss_t;
typedef void (*tss_dtor_t)(void *);

// Call once
// Note: This union conforms to the Itanium ABI.
typedef union __MCFCRT_tagC11threadOnceFlag {
	_MCFCRT_STD uint64_t __details[1]; // Placeholder for implementation-specific details.
	unsigned char __disposed;
	_MCFCRT_OnceFlag __once[1];
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

// 7.26.2 Initialization functions
extern void call_once(once_flag *__once_c, void (*__func)(void)) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "call_once");

// 7.26.3 Condition variable functions
extern int cnd_init(cnd_t *__cond_c) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "cnd_init");
extern void cnd_destroy(cnd_t *__cond_c) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "cnd_destroy");

extern int cnd_timedwait(cnd_t *restrict __cond_c, mtx_t *restrict __mutex_c, const struct timespec *restrict __timeout) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "cnd_timedwait");
extern int cnd_wait(cnd_t *restrict __cond_c, mtx_t *restrict __mutex_c) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "cnd_wait");
extern int cnd_signal(cnd_t *__cond_c) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "cnd_signal");
extern int cnd_broadcast(cnd_t *__cond_c) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "cnd_broadcast");

// 7.26.4 Mutex functions
extern int mtx_init(mtx_t *__mutex_c, int __mask) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "mtx_init");
extern void mtx_destroy(mtx_t *__mutex_c) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "mtx_destroy");

extern int mtx_lock(mtx_t *__mutex_c) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "mtx_lock");
extern int mtx_timedlock(mtx_t *restrict __mutex_c, const struct timespec *restrict __timeout) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "mtx_timedlock");
extern int mtx_trylock(mtx_t *__mutex_c) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "mtx_trylock");
extern int mtx_unlock(mtx_t *__mutex_c) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "mtx_unlock");

// 7.26.5 Thread functions
extern int thrd_create(thrd_t *__tid_ret, thrd_start_t __proc, void *__param) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "thrd_create");
__attribute__((__noreturn__))
extern void thrd_exit(int __exit_code) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "thrd_exit");
extern int thrd_join(thrd_t __tid, int *__exit_code_ret) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "thrd_join");
extern int thrd_detach(thrd_t __tid) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "thrd_detach");

__attribute__((__const__))
extern thrd_t thrd_current(void) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "thrd_current");
extern int thrd_equal(thrd_t __tid1, thrd_t __tid2) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "thrd_equal");

extern int thrd_sleep(const struct timespec *__duration, struct timespec *__remaining) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "thrd_sleep");
extern void thrd_yield(void) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "thrd_yield");

// 7.26.6 Thread-specific storage functions
extern int tss_create(tss_t *__key_ret, tss_dtor_t __destructor) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "tss_create");
extern void tss_delete(tss_t __key) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "tss_delete");
extern void *tss_get(tss_t __key) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "tss_get");
extern int tss_set(tss_t __key, void *__value) _MCFCRT_NOEXCEPT
	__asm__("__MCFCRT_" "tss_set");

_MCFCRT_EXTERN_C_END

#ifndef __MCFCRT_C11THREAD_INLINE_OR_EXTERN
#	define __MCFCRT_C11THREAD_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif
#include "_c11thread_inl.h"

#endif // __STDC_NO_THREADS__

#endif
