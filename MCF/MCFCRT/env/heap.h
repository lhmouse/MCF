// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_HEAP_H__
#define __MCF_CRT_HEAP_H__

#include "_crtdef.h"

__MCF_CRT_EXTERN unsigned long __MCF_CRT_HeapInitialize();
__MCF_CRT_EXTERN void __MCF_CRT_HeapUninitialize();

__MCF_CRT_EXTERN unsigned char *__MCF_CRT_HeapAlloc(__MCF_STD size_t uSize, const void *pRetAddr);
__MCF_CRT_EXTERN unsigned char *__MCF_CRT_HeapReAlloc(unsigned char *pBlock /* NON-NULL */, __MCF_STD size_t uSize, const void *pRetAddr);
__MCF_CRT_EXTERN void __MCF_CRT_HeapFree(unsigned char *pBlock /* NON-NULL */, const void *pRetAddr);

typedef struct tagBadAllocHandler {
	int (*pfnProc)(__MCF_STD intptr_t);
	__MCF_STD intptr_t nContext;
} __MCF_BAD_ALLOC_HANDLER;

__MCF_CRT_EXTERN __MCF_BAD_ALLOC_HANDLER __MCF_GetBadAllocHandler();
__MCF_CRT_EXTERN __MCF_BAD_ALLOC_HANDLER __MCF_SetBadAllocHandler(__MCF_BAD_ALLOC_HANDLER NewHandler);

#endif
