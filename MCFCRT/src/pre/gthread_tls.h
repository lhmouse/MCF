// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_PRE_GTHREAD_TLS_H_
#define __MCFCRT_PRE_GTHREAD_TLS_H_

// Compatibility layer for GCC.

#include "../env/gthread.h"
#include "tls.h"

#ifndef __MCFCRT_GTHREAD_TLS_INLINE_OR_EXTERN
#  define __MCFCRT_GTHREAD_TLS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

//-----------------------------------------------------------------------------
// Thread local storage
//-----------------------------------------------------------------------------
typedef _MCFCRT_TlsKeyHandle __gthread_key_t;

extern void __MCFCRT_gthread_tls_destructor(_MCFCRT_STD intptr_t __context, void *__storage) _MCFCRT_NOEXCEPT;

__MCFCRT_GTHREAD_TLS_INLINE_OR_EXTERN int __MCFCRT_gthread_key_create(__gthread_key_t *__key_ret, void (*__destructor)(void *)) _MCFCRT_NOEXCEPT {
	const __gthread_key_t __key = _MCFCRT_TlsAllocKey(sizeof(void *), _MCFCRT_NULLPTR, __destructor ? &__MCFCRT_gthread_tls_destructor : _MCFCRT_NULLPTR, (_MCFCRT_STD intptr_t)__destructor);
	if(!__key){
		return ENOMEM;
	}
	*__key_ret = __key;
	return 0;
}
__MCFCRT_GTHREAD_TLS_INLINE_OR_EXTERN int __MCFCRT_gthread_key_delete(__gthread_key_t __key) _MCFCRT_NOEXCEPT {
	_MCFCRT_TlsFreeKey(__key);
	return 0;
}

__MCFCRT_GTHREAD_TLS_INLINE_OR_EXTERN void *__MCFCRT_gthread_getspecific(__gthread_key_t __key) _MCFCRT_NOEXCEPT {
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
__MCFCRT_GTHREAD_TLS_INLINE_OR_EXTERN int __MCFCRT_gthread_setspecific(__gthread_key_t __key, const void *__value) _MCFCRT_NOEXCEPT {
	void *__storage;
	const bool __success = _MCFCRT_TlsRequire(__key, &__storage);
	if(!__success){
		return ENOMEM;
	}
	_MCFCRT_ASSERT(__storage);
	*(void **)__storage = (void *)__value;
	return 0;
}

#define __gthread_key_create   __MCFCRT_gthread_key_create
#define __gthread_key_delete   __MCFCRT_gthread_key_delete

#define __gthread_getspecific  __MCFCRT_gthread_getspecific
#define __gthread_setspecific  __MCFCRT_gthread_setspecific

_MCFCRT_EXTERN_C_END

#endif
