// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_HEAP_H_
#define __MCFCRT_ENV_HEAP_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

__attribute__((__malloc__))
extern void *__MCFCRT_HeapAlloc(_MCFCRT_STD size_t __uSizeNew, bool __bFillsWithZero, const void *__pRetAddrOuter) _MCFCRT_NOEXCEPT;
__attribute__((__nonnull__(1)))
extern void *__MCFCRT_HeapRealloc(void *__pBlockOld, _MCFCRT_STD size_t __uSizeNew, bool __bFillsWithZero, const void *__pRetAddrOuter) _MCFCRT_NOEXCEPT;
__attribute__((__nonnull__(1)))
extern void __MCFCRT_HeapFree(void *__pBlockOld, const void *__pRetAddrOuter) _MCFCRT_NOEXCEPT;

typedef void (*_MCFCRT_HeapCallback)(void *__pBlockNew, _MCFCRT_STD size_t __uSizeNew, void *__pBlockOld, const void *__pRetAddrOuter, const void *__pRetAddrInner);

extern _MCFCRT_HeapCallback _MCFCRT_GetHeapCallback(void) _MCFCRT_NOEXCEPT;
extern _MCFCRT_HeapCallback _MCFCRT_SetHeapCallback(_MCFCRT_HeapCallback __pfnNewCallback) _MCFCRT_NOEXCEPT;

__attribute__((__always_inline__, __malloc__))
static inline void *_MCFCRT_malloc(_MCFCRT_STD size_t  __size) _MCFCRT_NOEXCEPT {
	return __MCFCRT_HeapAlloc(__size, false,
		__builtin_return_address(0));
}
__attribute__((__always_inline__))
static inline void *_MCFCRT_realloc(void *__ptr, _MCFCRT_STD size_t __size) _MCFCRT_NOEXCEPT {
	if(!__ptr){
		return __MCFCRT_HeapAlloc(__size, false,
			__builtin_return_address(0));
	}
	return __MCFCRT_HeapRealloc(__ptr, __size, false,
		__builtin_return_address(0));
}
__attribute__((__always_inline__, __malloc__))
static inline void *_MCFCRT_calloc(_MCFCRT_STD size_t __nmemb, _MCFCRT_STD size_t __size) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD size_t __size_total = 0;
	if((__nmemb != 0) && (__size != 0) && __builtin_mul_overflow(__nmemb, __size, &__size_total)){
		return _MCFCRT_NULLPTR;
	}
	return __MCFCRT_HeapAlloc(__size_total, true,
		__builtin_return_address(0));
}
__attribute__((__always_inline__))
static inline void _MCFCRT_free(void *__ptr) _MCFCRT_NOEXCEPT {
	if(!__ptr){
		return;
	}
	__MCFCRT_HeapFree(__ptr,
		__builtin_return_address(0));
}

_MCFCRT_EXTERN_C_END

#endif
