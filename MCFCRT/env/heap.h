// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ENV_HEAP_H_
#define __MCF_CRT_ENV_HEAP_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern bool __MCF_CRT_HeapInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_HeapUninit(void) MCF_NOEXCEPT;

extern void *__MCF_CRT_ReallyAlloc(MCF_STD size_t __uSize) MCF_NOEXCEPT;
extern void *__MCF_CRT_ReallyRealloc(void *__pBlock, MCF_STD size_t __uSize) MCF_NOEXCEPT;
extern void __MCF_CRT_ReallyFree(void *__pBlock) MCF_NOEXCEPT;
extern MCF_STD size_t __MCF_CRT_ReallyGetUsableSize(void *__pBlock) MCF_NOEXCEPT;

extern unsigned char *__MCF_CRT_HeapAlloc(MCF_STD size_t __uSize, const void *__pRetAddr) MCF_NOEXCEPT;
extern unsigned char *__MCF_CRT_HeapRealloc(void *__pBlock /* not null */, MCF_STD size_t __uSize, const void *__pRetAddr) MCF_NOEXCEPT;
extern void __MCF_CRT_HeapFree(void *__pBlock /* not null */, const void *__pRetAddr) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
