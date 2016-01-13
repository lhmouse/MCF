// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

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
void (*__MCFCRT_OnHeapAlloc)(void *, size_t , const void *)            = nullptr;
void (*__MCFCRT_OnHeapRealloc)(void *, void *, size_t, const void *)   = nullptr;
void (*__MCFCRT_OnHeapFree)(void *__pBlock, const void *)              = nullptr;
bool (*__MCFCRT_OnHeapBadAlloc)(const void *)                          = nullptr;

static CRITICAL_SECTION g_csHeapMutex;

bool __MCFCRT_HeapInit(){
	if(!InitializeCriticalSectionEx(&g_csHeapMutex, 0x1000u,
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
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
void __MCFCRT_HeapUninit(){
	DeleteCriticalSection(&g_csHeapMutex);
}

unsigned char *__MCFCRT_HeapAlloc(size_t uSize, const void *pRetAddr){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
	const size_t uRawSize = __MCFCRT_HeapDbgGetRawSize(uSize);
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
			unsigned char *const pRaw = __MCFCRT_ReallyAlloc(uRawSize);
			if(pRaw){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
				__MCFCRT_HeapDbgBlockInfo *const pBlockInfo = __MCFCRT_HeapDbgAllocateBlockInfo();
				if(!pBlockInfo){
					__MCFCRT_ReallyFree(pRaw);
					goto jFailed;
				}
				pRet = __MCFCRT_HeapDbgRegisterBlockInfo(pBlockInfo, pRaw, uSize, pRetAddr);
#elif __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
				pRet = __MCFCRT_HeapDbgAddBlockGuardsBasic(pRaw);
#else
				pRet = pRaw;
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
				memset(pRet, 0xCD, uSize);
#endif
			}
		}
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
	jFailed:
#endif
		LeaveCriticalSection(&g_csHeapMutex);

		if(pRet){
			if(__MCFCRT_OnHeapAlloc){
				(*__MCFCRT_OnHeapAlloc)(pRet, uSize, pRetAddr);
			}
			return pRet;
		}

		if(!(__MCFCRT_OnHeapBadAlloc && (*__MCFCRT_OnHeapBadAlloc)(pRetAddr))){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return nullptr;
		}
	}
}
unsigned char *__MCFCRT_HeapRealloc(void *pBlock, size_t uSize, const void *pRetAddr){
	if(!pBlock){
		MCFCRT_Bail(L"__MCFCRT_HeapRealloc() 失败：传入了一个空指针。\n\n");
	}

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
	const size_t uRawSize = __MCFCRT_HeapDbgGetRawSize(uSize);
	if(uRawSize < uSize){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
#else
	const size_t uRawSize = uSize;
#endif

	unsigned char *pRawOriginal;
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
#	if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
	__MCFCRT_HeapDbgBlockInfo *pBlockInfo;
#		if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
	size_t uOriginalSize;
#		endif
#	endif
	EnterCriticalSection(&g_csHeapMutex);
	{
#	if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
		pBlockInfo = __MCFCRT_HeapDbgValidateBlock(&pRawOriginal, pBlock, pRetAddr);
#		if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
		if(pBlockInfo){
			uOriginalSize = pBlockInfo->__uSize;
		} else {
			uOriginalSize = __MCFCRT_ReallyGetUsableSize(pRawOriginal);
		}
#		endif
#	else
		__MCFCRT_HeapDbgValidateBlockBasic(&pRawOriginal, pBlock, pRetAddr);
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
			unsigned char *const pRaw = __MCFCRT_ReallyRealloc(pRawOriginal, uRawSize);
			if(pRaw){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
				__MCFCRT_HeapDbgUnregisterBlockInfo(pBlockInfo);
				pRet = __MCFCRT_HeapDbgRegisterBlockInfo(pBlockInfo, pRaw, uSize, pRetAddr);
#elif __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
				pRet = __MCFCRT_HeapDbgAddBlockGuardsBasic(pRaw);
#else
				pRet = pRaw;
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
				if(uOriginalSize < uSize){
					memset(pRet + uOriginalSize, 0xCD, uSize - uOriginalSize);
				}
#endif
			}
		}
		LeaveCriticalSection(&g_csHeapMutex);

		if(pRet){
			if(__MCFCRT_OnHeapRealloc){
				(*__MCFCRT_OnHeapRealloc)(pRet, pBlock, uSize, pRetAddr);
			}
			return pRet;
		}

		if(!(__MCFCRT_OnHeapBadAlloc && (*__MCFCRT_OnHeapBadAlloc)(pRetAddr))){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return nullptr;
		}
	}
}
void __MCFCRT_HeapFree(void *pBlock, const void *pRetAddr){
	if(!pBlock){
		MCFCRT_Bail(L"__MCFCRT_HeapFree() 失败：传入了一个空指针。\n\n");
	}

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

	EnterCriticalSection(&g_csHeapMutex);
	{
		unsigned char *pRaw;
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
		__MCFCRT_HeapDbgBlockInfo *const pBlockInfo = __MCFCRT_HeapDbgValidateBlock(&pRaw, pBlock, pRetAddr);

		__MCFCRT_HeapDbgUnregisterBlockInfo(pBlockInfo);
		__MCFCRT_HeapDbgDeallocateBlockInfo(pBlockInfo);
#elif __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
		__MCFCRT_HeapDbgValidateBlockBasic(&pRaw, pBlock, pRetAddr);
#else
		pRaw = pBlock;
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
		memset(pRaw, 0xFE, __MCFCRT_ReallyGetUsableSize(pRaw));
#endif
		__MCFCRT_ReallyFree(pRaw);
	}
	LeaveCriticalSection(&g_csHeapMutex);

	if(__MCFCRT_OnHeapFree){
		(*__MCFCRT_OnHeapFree)(pBlock, pRetAddr);
	}
}

#define USE_DLMALLOC    1

#if USE_DLMALLOC
#	define USE_DL_PREFIX
#	include "../../External/dlmalloc/malloc.h"
#endif

void *__MCFCRT_ReallyAlloc(size_t uSize){
#if USE_DLMALLOC
	return dlmalloc(uSize);
#else
	return HeapAlloc(GetProcessHeap(), 0, uSize);
#endif
}
void *__MCFCRT_ReallyRealloc(void *pBlock, size_t uSize){
#if USE_DLMALLOC
	return dlrealloc(pBlock, uSize);
#else
	return HeapReAlloc(GetProcessHeap(), 0, pBlock, uSize);
#endif
}
void __MCFCRT_ReallyFree(void *pBlock){
#if USE_DLMALLOC
	dlfree(pBlock);
#else
	HeapFree(GetProcessHeap(), 0, pBlock);
#endif
}
size_t __MCFCRT_ReallyGetUsableSize(void *pBlock){
#if USE_DLMALLOC
	return dlmalloc_usable_size(pBlock);
#else
	return HeapSize(GetProcessHeap(), 0, pBlock);
#endif
}
