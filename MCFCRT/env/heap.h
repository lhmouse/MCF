// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_HEAP_H_
#define __MCFCRT_ENV_HEAP_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_HeapInit(void) MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapUninit(void) MCFCRT_NOEXCEPT;

extern void *__MCFCRT_ReallyAlloc(MCFCRT_STD size_t __uSize) MCFCRT_NOEXCEPT;
extern void *__MCFCRT_ReallyRealloc(void *__pBlock, MCFCRT_STD size_t __uSize) MCFCRT_NOEXCEPT;
extern void __MCFCRT_ReallyFree(void *__pBlock) MCFCRT_NOEXCEPT;
extern MCFCRT_STD size_t __MCFCRT_ReallyGetUsableSize(void *__pBlock) MCFCRT_NOEXCEPT;

extern unsigned char *__MCFCRT_HeapAlloc(MCFCRT_STD size_t __uSize, const void *__pRetAddr) MCFCRT_NOEXCEPT;
extern unsigned char *__MCFCRT_HeapRealloc(void *__pBlock /* not null */, MCFCRT_STD size_t __uSize, const void *__pRetAddr) MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapFree(void *__pBlock /* not null */, const void *__pRetAddr) MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
