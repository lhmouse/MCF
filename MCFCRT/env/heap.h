// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_HEAP_H_
#define __MCFCRT_ENV_HEAP_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_HeapInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapUninit(void) _MCFCRT_NOEXCEPT;

extern unsigned char *__MCFCRT_HeapAlloc(_MCFCRT_STD size_t __uSize, bool __bFillsWithZero, const void *__pRetAddr) _MCFCRT_NOEXCEPT
	__attribute__((__malloc__));
extern unsigned char *__MCFCRT_HeapRealloc(void *__pBlock, _MCFCRT_STD size_t __uSize, bool __bFillsWithZero, const void *__pRetAddr) _MCFCRT_NOEXCEPT
	__attribute__((__nonnull__(1)));
extern void __MCFCRT_HeapFree(void *__pBlock, const void *__pRetAddr) _MCFCRT_NOEXCEPT
	__attribute__((__nonnull__(1)));

__attribute__((__always_inline__, __malloc__))
static inline void *_MCFCRT_malloc(_MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT {
	return __MCFCRT_HeapAlloc(__uSize, false,
		__builtin_return_address(0));
}
__attribute__((__always_inline__, __malloc__))
static inline void *_MCFCRT_realloc(void *__pBlock, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT {
	if(!__pBlock){
		return __MCFCRT_HeapAlloc(__uSize,
			false, __builtin_return_address(0));
	}
	return __MCFCRT_HeapRealloc(__pBlock, __uSize, false,
		__builtin_return_address(0));
}
__attribute__((__always_inline__, __malloc__))
static inline void *_MCFCRT_calloc(_MCFCRT_STD size_t __uCount, _MCFCRT_STD size_t __uBlockSize) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD size_t __uSize = 0;
	if((__uCount != 0) && (__uBlockSize != 0)){
		if(__uCount > SIZE_MAX / __uBlockSize){
			return nullptr;
		}
		__uSize = __uCount * __uBlockSize;
	}
	return __MCFCRT_HeapAlloc(__uSize, true,
		__builtin_return_address(0));
}
__attribute__((__always_inline__))
static inline void _MCFCRT_free(void *__pBlock) _MCFCRT_NOEXCEPT {
	if(!__pBlock){
		return;
	}
	__MCFCRT_HeapFree(__pBlock,
		__builtin_return_address(0));
}

_MCFCRT_EXTERN_C_END

#endif
