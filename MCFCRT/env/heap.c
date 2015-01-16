// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "heap.h"
#include "heap_dbg.h"
#include "hooks.h"
#include "mcfwin.h"
#include "bail.h"
#include "../ext/unref_param.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define USE_DL_PREFIX
#include "../../External/dlmalloc/malloc.h"

void MCF_OnHeapAlloc(void *pBlock, size_t uBytes, const void *pRetAddr){
	UNREF_PARAM(pBlock);
	UNREF_PARAM(uBytes);
	UNREF_PARAM(pRetAddr);
}
void MCF_OnHeapDealloc(void *pBlock, const void *pRetAddr){
	UNREF_PARAM(pBlock);
	UNREF_PARAM(pRetAddr);
}

static CRITICAL_SECTION g_csHeapMutex;

bool __MCF_CRT_HeapInit(){
	if(!InitializeCriticalSectionEx(&g_csHeapMutex, 0x1000u,
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(1)
		CRITICAL_SECTION_NO_DEBUG_INFO
#else
		0
#endif
		))
	{
		return false;
	}
	return true;
}
void __MCF_CRT_HeapUninit(){
	DeleteCriticalSection(&g_csHeapMutex);
}

unsigned char *__MCF_CRT_HeapAlloc(size_t uSize, const void *pRetAddr){
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
	const size_t uRawSize = __MCF_CRT_HeapDbgGetRawSize(uSize);
	if(uRawSize < uSize){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
#else
	const size_t uRawSize = uSize;
#endif

	unsigned char *pRet = nullptr;
	EnterCriticalSection(&g_csHeapMutex);
	{
		do {
			unsigned char *const pRaw = dlmalloc(uRawSize);
			if(pRaw){
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
				pRet = __MCF_CRT_HeapDbgAddGuardsAndRegister(pRaw, uSize, pRetAddr);
#else
				pRet = pRaw;
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)
				memset(pRet, 0xCD, uSize);
#endif
				MCF_OnHeapAlloc(pRet, uSize, pRetAddr);
				break;
			}
		} while(MCF_OnBadAlloc());
	}
	LeaveCriticalSection(&g_csHeapMutex);
	if(!pRet){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	}
	return pRet;
}
unsigned char *__MCF_CRT_HeapReAlloc(void *pBlock, size_t uSize, const void *pRetAddr){
	if(!pBlock){
		MCF_CRT_Bail(L"__MCF_CRT_HeapReAlloc() 失败：传入了一个空指针。\n\n");
	}

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
	const size_t uRawSize = __MCF_CRT_HeapDbgGetRawSize(uSize);
	if(uRawSize < uSize){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
#else
	const size_t uRawSize = uSize;
#endif

	unsigned char *pRet = nullptr;
	EnterCriticalSection(&g_csHeapMutex);
	{
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
		unsigned char *pRawOriginal;
		const __MCF_HeapDbgBlockInfo *const pBlockInfo = __MCF_CRT_HeapDbgValidate(&pRawOriginal, pBlock, pRetAddr);
		const size_t uOriginalSize = pBlockInfo->uSize;
#else
		unsigned char *const pRawOriginal = pBlock;
		const size_t uOriginalSize = dlmalloc_usable_size(pRawOriginal);
#endif
		UNREF_PARAM(uOriginalSize);

		do {
			unsigned char *const pRaw = dlrealloc(pRawOriginal, uRawSize);
			if(pRaw){
				MCF_OnHeapDealloc(pBlock, pRetAddr);

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
				__MCF_CRT_HeapDbgUnregister(pBlockInfo);

				pRet = __MCF_CRT_HeapDbgAddGuardsAndRegister(pRaw, uSize, pRetAddr);
#else
				pRet = pRaw;
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)
				if(uOriginalSize < uSize){
					memset(pRet + uOriginalSize, 0xCD, uSize - uOriginalSize);
				}
#endif

				MCF_OnHeapAlloc(pRet, uSize, pRetAddr);
				break;
			}
		} while(MCF_OnBadAlloc());
	}
	LeaveCriticalSection(&g_csHeapMutex);
	if(!pRet){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	}
	return pRet;
}
void __MCF_CRT_HeapFree(void *pBlock, const void *pRetAddr){
	if(!pBlock){
		MCF_CRT_Bail(L"__MCF_CRT_HeapFree() 失败：传入了一个空指针。\n\n");
	}

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

	EnterCriticalSection(&g_csHeapMutex);
	{
		MCF_OnHeapDealloc(pBlock, pRetAddr);

		unsigned char *pRaw;
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
		const __MCF_HeapDbgBlockInfo *const pBlockInfo = __MCF_CRT_HeapDbgValidate(&pRaw, pBlock, pRetAddr);

		__MCF_CRT_HeapDbgUnregister(pBlockInfo);
#else
		pRaw = pBlock;
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)
		memset(pRaw, 0xFE, dlmalloc_usable_size(pRaw));
#endif
		dlfree(pRaw);
	}
	LeaveCriticalSection(&g_csHeapMutex);
}

bool MCF_OnBadAlloc(){
	return false;
}
