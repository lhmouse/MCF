// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_PRE_C11THREAD_TLS_H_
#define __MCFCRT_PRE_C11THREAD_TLS_H_

// Compatibility layer for the ISO/IEC C11 standard.

#include "../env/c11thread.h"
#include "tls.h"

#ifndef __MCFCRT_C11THREAD_TLS_INLINE_OR_EXTERN
#	define __MCFCRT_C11THREAD_TLS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

//-----------------------------------------------------------------------------
// 7.26.1 Introduction
//-----------------------------------------------------------------------------
#if !defined(__cplusplus)
#	define thread_local        _Thread_local
#endif

#define TSS_DTOR_ITERATIONS    1      // XXX: Do we really need to support this crap?

// Thread specific storage
typedef _MCFCRT_TlsKeyHandle tss_t; // typedef void *tss_t;
typedef void (*tss_dtor_t)(void *);

//-----------------------------------------------------------------------------
// 7.26.6 Thread-specific storage functions
//-----------------------------------------------------------------------------
extern void __MCFCRT_C11threadTlsDestructor(_MCFCRT_STD intptr_t __context, void *__storage) _MCFCRT_NOEXCEPT;

__MCFCRT_C11THREAD_TLS_INLINE_OR_EXTERN int __MCFCRT_tss_create(tss_t *__key_ret, tss_dtor_t __destructor) _MCFCRT_NOEXCEPT {
	const tss_t __key = _MCFCRT_TlsAllocKey(sizeof(void *), _MCFCRT_NULLPTR, __destructor ? &__MCFCRT_C11threadTlsDestructor : _MCFCRT_NULLPTR, (_MCFCRT_STD intptr_t)__destructor);
	if(!__key){
		return thrd_nomem;
	}
	*__key_ret = __key;
	return thrd_success;
}
__MCFCRT_C11THREAD_TLS_INLINE_OR_EXTERN void __MCFCRT_tss_delete(tss_t __key) _MCFCRT_NOEXCEPT {
	_MCFCRT_TlsFreeKey(__key);
}

__MCFCRT_C11THREAD_TLS_INLINE_OR_EXTERN void *__MCFCRT_tss_get(tss_t __key) _MCFCRT_NOEXCEPT {
	void *__storage;
	const bool __success = _MCFCRT_TlsGet(__key, &__storage);
	if(!__success){
		return _MCFCRT_NULLPTR;
	}
	if(!__storage){
		return _MCFCRT_NULLPTR;
	}
	return *(void **)__storage;
}
__MCFCRT_C11THREAD_TLS_INLINE_OR_EXTERN int __MCFCRT_tss_set(tss_t __key, void *__value) _MCFCRT_NOEXCEPT {
	void *__storage;
	const bool __success = _MCFCRT_TlsRequire(__key, &__storage);
	if(!__success){
		return thrd_error; // XXX: ISO C does not seem to allow `thrd_nomem` here.
	}
	_MCFCRT_ASSERT(__storage);
	*(void **)__storage = (void *)__value;
	return thrd_success;
}

#define tss_create  __MCFCRT_tss_create
#define tss_delete  __MCFCRT_tss_delete

#define tss_get     __MCFCRT_tss_get
#define tss_set     __MCFCRT_tss_set

_MCFCRT_EXTERN_C_END

#endif
