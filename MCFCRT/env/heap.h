// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_HEAP_H_
#define __MCFCRT_ENV_HEAP_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_HeapInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapUninit(void) _MCFCRT_NOEXCEPT;

extern unsigned char *__MCFCRT_HeapAlloc(_MCFCRT_STD size_t __uSize, bool __bFillsWithZero, const void *__pRetAddr) _MCFCRT_NOEXCEPT;
extern unsigned char *__MCFCRT_HeapRealloc(void *__pBlock /* not null */, _MCFCRT_STD size_t __uSize, bool __bFillsWithZero, const void *__pRetAddr) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapFree(void *__pBlock /* not null */, const void *__pRetAddr) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
