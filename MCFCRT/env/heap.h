// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_HEAP_H__
#define __MCF_CRT_HEAP_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern unsigned long __MCF_CRT_HeapInitialize(void) __MCF_NOEXCEPT;
extern void __MCF_CRT_HeapUninitialize(void) __MCF_NOEXCEPT;

extern unsigned char *__MCF_CRT_HeapAlloc(__MCF_STD size_t uSize, const void *pRetAddr) __MCF_NOEXCEPT;
extern unsigned char *__MCF_CRT_HeapReAlloc(unsigned char *pBlock /* NON-NULL */, __MCF_STD size_t uSize, const void *pRetAddr) __MCF_NOEXCEPT;
extern void __MCF_CRT_HeapFree(unsigned char *pBlock /* NON-NULL */, const void *pRetAddr) __MCF_NOEXCEPT;

typedef struct tagBadAllocHandler {
	int (*pfnProc)(__MCF_STD intptr_t);
	__MCF_STD intptr_t nContext;
} __MCF_BAD_ALLOC_HANDLER;

extern __MCF_BAD_ALLOC_HANDLER __MCF_GetBadAllocHandler(void) __MCF_NOEXCEPT;
extern __MCF_BAD_ALLOC_HANDLER __MCF_SetBadAllocHandler(__MCF_BAD_ALLOC_HANDLER NewHandler) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
