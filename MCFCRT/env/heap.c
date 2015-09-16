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

// hooks.h
void (*__MCF_CRT_OnHeapAlloc)(void *, size_t , const void *)            = nullptr;
void (*__MCF_CRT_OnHeapRealloc)(void *, void *, size_t, const void *)   = nullptr;
void (*__MCF_CRT_OnHeapFree)(void *__pBlock, const void *)              = nullptr;
bool (*__MCF_CRT_OnHeapBadAlloc)(const void *)                          = nullptr;

static CRITICAL_SECTION g_csHeapMutex;

bool __MCF_CRT_HeapInit(){
	if(!InitializeCriticalSectionEx(&g_csHeapMutex, 0x1000u,
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(4)
		0
#else
		CRITICAL_SECTION_NO_DEBUG_INFO
#endif
		))
	{
		return false;
	}

	// 启用 FLH，但是忽略任何错误。
	ULONG ulMagic = 2;
	HeapSetInformation(GetProcessHeap(), HeapCompatibilityInformation, &ulMagic, sizeof(ulMagic));

	return true;
}
void __MCF_CRT_HeapUninit(){
	DeleteCriticalSection(&g_csHeapMutex);
}

unsigned char *__MCF_CRT_HeapAlloc(size_t uSize, const void *pRetAddr){
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)
	const size_t uRawSize = __MCF_CRT_HeapDbgGetRawSize(uSize);
	if(uRawSize < uSize){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
#else
	const size_t uRawSize = uSize;
#endif

	for(;;){
		unsigned char *pRet = nullptr;

		EnterCriticalSection(&g_csHeapMutex);
		{
			unsigned char *const pRaw = __MCF_CRT_ReallyAlloc(uRawSize);
			if(pRaw){
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
				__MCF_HeapDbgBlockInfo *const pBlockInfo = __MCF_CRT_HeapDbgAllocateBlockInfo();
				if(!pBlockInfo){
					__MCF_CRT_ReallyFree(pRaw);
					goto jFailed;
				}
				pRet = __MCF_CRT_HeapDbgRegisterBlockInfo(pBlockInfo, pRaw, uSize, pRetAddr);
#elif __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)
				pRet = __MCF_CRT_HeapDbgAddBlockGuardsBasic(pRaw);
#else
				pRet = pRaw;
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(4)
				memset(pRet, 0xCD, uSize);
#endif
			}
		}
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
	jFailed:
#endif
		LeaveCriticalSection(&g_csHeapMutex);

		if(pRet){
			if(__MCF_CRT_OnHeapAlloc){
				(*__MCF_CRT_OnHeapAlloc)(pRet, uSize, pRetAddr);
			}
			return pRet;
		}

		if(!(__MCF_CRT_OnHeapBadAlloc && (*__MCF_CRT_OnHeapBadAlloc)(pRetAddr))){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return nullptr;
		}
	}
}
unsigned char *__MCF_CRT_HeapRealloc(void *pBlock, size_t uSize, const void *pRetAddr){
	if(!pBlock){
		MCF_CRT_Bail(L"__MCF_CRT_HeapRealloc() 失败：传入了一个空指针。\n\n");
	}

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)
	const size_t uRawSize = __MCF_CRT_HeapDbgGetRawSize(uSize);
	if(uRawSize < uSize){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
#else
	const size_t uRawSize = uSize;
#endif

	unsigned char *pRawOriginal;
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)
#	if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
	__MCF_HeapDbgBlockInfo *pBlockInfo;
#		if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(4)
	size_t uOriginalSize;
#		endif
#	endif
	EnterCriticalSection(&g_csHeapMutex);
	{
#	if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
		pBlockInfo = __MCF_CRT_HeapDbgValidateBlock(&pRawOriginal, pBlock, pRetAddr);
#		if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(4)
		if(pBlockInfo){
			uOriginalSize = pBlockInfo->__uSize;
		} else {
			uOriginalSize = __MCF_CRT_ReallyGetUsableSize(pRawOriginal);
		}
#		endif
#	else
		__MCF_CRT_HeapDbgValidateBlockBasic(&pRawOriginal, pBlock, pRetAddr);
#	endif
	}
	LeaveCriticalSection(&g_csHeapMutex);
#else
	pRawOriginal = pBlock;
#endif

	for(;;){
		unsigned char *pRet = nullptr;

		EnterCriticalSection(&g_csHeapMutex);
		{
			unsigned char *const pRaw = __MCF_CRT_ReallyRealloc(pRawOriginal, uRawSize);
			if(pRaw){
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
				__MCF_CRT_HeapDbgUnregisterBlockInfo(pBlockInfo);
				pRet = __MCF_CRT_HeapDbgRegisterBlockInfo(pBlockInfo, pRaw, uSize, pRetAddr);
#elif __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)
				pRet = __MCF_CRT_HeapDbgAddBlockGuardsBasic(pRaw);
#else
				pRet = pRaw;
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(4)
				if(uOriginalSize < uSize){
					memset(pRet + uOriginalSize, 0xCD, uSize - uOriginalSize);
				}
#endif
			}
		}
		LeaveCriticalSection(&g_csHeapMutex);

		if(pRet){
			if(__MCF_CRT_OnHeapRealloc){
				(*__MCF_CRT_OnHeapRealloc)(pRet, pBlock, uSize, pRetAddr);
			}
			return pRet;
		}

		if(!(__MCF_CRT_OnHeapBadAlloc && (*__MCF_CRT_OnHeapBadAlloc)(pRetAddr))){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return nullptr;
		}
	}
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
		unsigned char *pRaw;
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
		__MCF_HeapDbgBlockInfo *const pBlockInfo = __MCF_CRT_HeapDbgValidateBlock(&pRaw, pBlock, pRetAddr);

		__MCF_CRT_HeapDbgUnregisterBlockInfo(pBlockInfo);
		__MCF_CRT_HeapDbgDeallocateBlockInfo(pBlockInfo);
#elif __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)
		__MCF_CRT_HeapDbgValidateBlockBasic(&pRaw, pBlock, pRetAddr);
#else
		pRaw = pBlock;
#endif

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(4)
		memset(pRaw, 0xFE, __MCF_CRT_ReallyGetUsableSize(pRaw));
#endif
		__MCF_CRT_ReallyFree(pRaw);
	}
	LeaveCriticalSection(&g_csHeapMutex);

	if(__MCF_CRT_OnHeapFree){
		(*__MCF_CRT_OnHeapFree)(pBlock, pRetAddr);
	}
}

#define USE_DLMALLOC	1

#if USE_DLMALLOC
#	define USE_DL_PREFIX
#	include "../../External/dlmalloc/malloc.h"
#endif

void *__MCF_CRT_ReallyAlloc(size_t uSize){
#if USE_DLMALLOC
	return dlmalloc(uSize);
#else
	return HeapAlloc(GetProcessHeap(), 0, uSize);
#endif
}
void *__MCF_CRT_ReallyRealloc(void *pBlock, size_t uSize){
#if USE_DLMALLOC
	return dlrealloc(pBlock, uSize);
#else
	return HeapReAlloc(GetProcessHeap(), 0, pBlock, uSize);
#endif
}
void __MCF_CRT_ReallyFree(void *pBlock){
#if USE_DLMALLOC
	dlfree(pBlock);
#else
	HeapFree(GetProcessHeap(), 0, pBlock);
#endif
}
size_t __MCF_CRT_ReallyGetUsableSize(void *pBlock){
#if USE_DLMALLOC
	return dlmalloc_usable_size(pBlock);
#else
	return HeapSize(GetProcessHeap(), 0, pBlock);
#endif
}
