// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "heap.h"
#include "heap_dbg.h"
#include "hooks.h"
#include "mcfwin.h"
#include "../ext/unref_param.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef __MCF_CRT_NO_DLMALLOC

#	define USE_DL_PREFIX
#	include "../../External/dlmalloc/malloc.h"

#	define	MCF_MALLOC(cb)		dlmalloc(cb)
#	define	MCF_REALLOC(p, cb)	dlrealloc((p), (cb))
#	define	MCF_FREE(p)			dlfree(p)

#else

#	define	MCF_MALLOC(cb)		HeapAlloc(GetProcessHeap(), 0, (cb))
#	define	MCF_REALLOC(p, cb)	HeapReAlloc(GetProcessHeap(), 0, (p), (cb))
#	define	MCF_FREE(p)			HeapFree(GetProcessHeap(), 0, (p))

#endif

void MCF_OnHeapAlloc(void *pBlock, size_t uBytes, const void *pRetAddr){
	UNREF_PARAM(pBlock);
	UNREF_PARAM(uBytes);
	UNREF_PARAM(pRetAddr);
}
void MCF_OnHeapDealloc(void *pBlock, const void *pRetAddr){
	UNREF_PARAM(pBlock);
	UNREF_PARAM(pRetAddr);
}

static CRITICAL_SECTION		g_csHeapLock;

bool __MCF_CRT_HeapInit(){
	if(!InitializeCriticalSectionEx(
		&g_csHeapLock, 0x1000u,
#ifdef NDEBUG
		CRITICAL_SECTION_NO_DEBUG_INFO
#else
		0
#endif
	)){
		return false;
	}
	return true;
}
void __MCF_CRT_HeapUninit(){
	DeleteCriticalSection(&g_csHeapLock);
}

unsigned char *__MCF_CRT_HeapAlloc(size_t uSize, const void *pRetAddr){
#ifdef __MCF_CRT_HEAPDBG_ON
	const size_t uRawSize = __MCF_CRT_HeapDbgGetRawSize(uSize);
	if(uRawSize < uSize){
		return NULL;
	}
#else
	UNREF_PARAM(pRetAddr);

	const size_t uRawSize = uSize;
#endif

	unsigned char *pRet = NULL;
	EnterCriticalSection(&g_csHeapLock);
	{
		do {
			unsigned char *const pRaw = MCF_MALLOC(uRawSize);
			if(pRaw){
#ifdef __MCF_CRT_HEAPDBG_ON
				__MCF_CRT_HeapDbgAddGuardsAndRegister(&pRet, pRaw, uSize, pRetAddr);
				memset(pRet, 0xCD, uSize);
#else
				pRet = pRaw;
#endif
				break;
			}
		} while(MCF_OnBadAlloc());

		MCF_OnHeapAlloc(pRet, uSize, pRetAddr);
	}
	LeaveCriticalSection(&g_csHeapLock);
	return pRet;
}
unsigned char *__MCF_CRT_HeapReAlloc(void *pBlock /* NON-NULL */, size_t uSize, const void *pRetAddr){
#ifdef __MCF_CRT_HEAPDBG_ON
	const size_t uRawSize = __MCF_CRT_HeapDbgGetRawSize(uSize);
	if(uSize & ~uRawSize & ((size_t)1 << (sizeof(size_t) * 8 - 1))){
		return NULL;
	}
#else
	UNREF_PARAM(pRetAddr);

	const size_t uRawSize = uSize;
#endif

	unsigned char *pRet = NULL;
	EnterCriticalSection(&g_csHeapLock);
	{
#ifdef __MCF_CRT_HEAPDBG_ON
		unsigned char *pRawOriginal;
		const __MCF_HEAPDBG_BLOCK_INFO *const pBlockInfo = __MCF_CRT_HeapDbgValidate(&pRawOriginal, pBlock, pRetAddr);
#else
		unsigned char *const pRawOriginal = pBlock;
#endif

		MCF_OnHeapDealloc(pBlock, pRetAddr);

		do {
			unsigned char *const pRaw = MCF_REALLOC(pRawOriginal, uRawSize);
			if(pRaw){
#ifdef __MCF_CRT_HEAPDBG_ON
				const size_t uOriginalSize = pBlockInfo->uSize;
				__MCF_CRT_HeapDbgUnregister(pBlockInfo);

				__MCF_CRT_HeapDbgAddGuardsAndRegister(&pRet, pRaw, uSize, pRetAddr);
				if(uOriginalSize < uSize){
					memset(pRet + uOriginalSize, 0xCD, uSize - uOriginalSize);
				}
#else
				pRet = pRaw;
#endif
				break;
			}
		} while(MCF_OnBadAlloc());

		MCF_OnHeapAlloc(pRet, uSize, pRetAddr);
	}
	LeaveCriticalSection(&g_csHeapLock);
	return pRet;
}
void __MCF_CRT_HeapFree(void *pBlock /* NON-NULL */, const void *pRetAddr){
	EnterCriticalSection(&g_csHeapLock);
	{
#ifdef __MCF_CRT_HEAPDBG_ON
		unsigned char *pRaw;
		const __MCF_HEAPDBG_BLOCK_INFO *const pBlockInfo = __MCF_CRT_HeapDbgValidate(&pRaw, pBlock, pRetAddr);

		memset(pBlock, 0xFE, pBlockInfo->uSize);

		__MCF_CRT_HeapDbgUnregister(pBlockInfo);
#else
		UNREF_PARAM(pRetAddr);

		unsigned char *const pRaw = pBlock;
#endif

		MCF_OnHeapDealloc(pBlock, pRetAddr);

		MCF_FREE(pRaw);
	}
	LeaveCriticalSection(&g_csHeapLock);
}

bool MCF_OnBadAlloc(){
	return false;
}
