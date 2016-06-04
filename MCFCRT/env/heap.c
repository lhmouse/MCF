// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "heap.h"
#include "heap_dbg.h"
#include "hooks.h"
#include "mutex.h"
#include "mcfwin.h"
#include "bail.h"
#include <errno.h>

// hooks.h
volatile _MCFCRT_HeapAllocCallback    _MCFCRT_pfnOnHeapAlloc     = nullptr;
volatile _MCFCRT_HeapReallocCallback  _MCFCRT_pfnOnHeapRealloc   = nullptr;
volatile _MCFCRT_HeapFreeCallback     _MCFCRT_pfnOnHeapFree      = nullptr;
volatile _MCFCRT_HeapBadAllocCallback _MCFCRT_pfnOnHeapBadAlloc  = nullptr;

static _MCFCRT_Mutex g_vHeapMutex = { 0 };

#define HEAP_MUTEX_SPIN_COUNT     ((unsigned)4000)

bool __MCFCRT_HeapInit(void){
	// 启用 FLH，但是忽略任何错误。
	ULONG ulMagic = 2;
	HeapSetInformation(GetProcessHeap(), HeapCompatibilityInformation, &ulMagic, sizeof(ulMagic));

	return true;
}
void __MCFCRT_HeapUninit(void){
}

unsigned char *__MCFCRT_HeapAlloc(size_t uSize, bool bFillsWithZero, const void *pRetAddr){
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

		_MCFCRT_WaitForMutexForever(&g_vHeapMutex, HEAP_MUTEX_SPIN_COUNT);
		{
			unsigned char *const pRaw = HeapAlloc(GetProcessHeap(), bFillsWithZero ? HEAP_ZERO_MEMORY : 0, uRawSize);
			if(pRaw){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
				__MCFCRT_HeapDbgBlockInfo *const pBlockInfo = __MCFCRT_HeapDbgAllocateBlockInfo();
				if(!pBlockInfo){
					HeapFree(GetProcessHeap(), 0, pRaw);
					goto jFailed;
				}
				pRet = __MCFCRT_HeapDbgRegisterBlockInfo(pBlockInfo, pRaw, uSize, pRetAddr);
#elif __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
				pRet = __MCFCRT_HeapDbgAddBlockGuardsBasic(pRaw);
#else
				pRet = pRaw;
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
				if(!bFillsWithZero){
					memset(pRet, 0xCD, uSize);
				}
#endif
			}
		}
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
	jFailed:
#endif
		_MCFCRT_SignalMutex(&g_vHeapMutex);

		if(pRet){
			if(_MCFCRT_pfnOnHeapAlloc){
				(*_MCFCRT_pfnOnHeapAlloc)(pRet, uSize, pRetAddr);
			}
			return pRet;
		}

		if(!(_MCFCRT_pfnOnHeapBadAlloc && (*_MCFCRT_pfnOnHeapBadAlloc)(pRetAddr))){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return nullptr;
		}
	}
}
unsigned char *__MCFCRT_HeapRealloc(void *pBlock, size_t uSize, bool bFillsWithZero, const void *pRetAddr){
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
	_MCFCRT_WaitForMutexForever(&g_vHeapMutex, HEAP_MUTEX_SPIN_COUNT);
	{
#	if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
		pBlockInfo = __MCFCRT_HeapDbgValidateBlock(&pRawOriginal, pBlock, pRetAddr);
#		if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
		if(pBlockInfo){
			uOriginalSize = pBlockInfo->__uSize;
		} else {
			uOriginalSize = HeapSize(GetProcessHeap(), 0, pRawOriginal);
		}
#		endif
#	else
		__MCFCRT_HeapDbgValidateBlockBasic(&pRawOriginal, pBlock, pRetAddr);
#	endif
	}
	_MCFCRT_SignalMutex(&g_vHeapMutex);
#else
	pRawOriginal = pBlock;
#endif

	for(;;){
		unsigned char *pRet = nullptr;

		_MCFCRT_WaitForMutexForever(&g_vHeapMutex, HEAP_MUTEX_SPIN_COUNT);
		{
			unsigned char *const pRaw = HeapReAlloc(GetProcessHeap(), bFillsWithZero ? HEAP_ZERO_MEMORY : 0, pRawOriginal, uRawSize);
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
				if(!bFillsWithZero && (uOriginalSize < uSize)){
					memset(pRet + uOriginalSize, 0xCD, uSize - uOriginalSize);
				}
#endif
			}
		}
		_MCFCRT_SignalMutex(&g_vHeapMutex);

		if(pRet){
			if(_MCFCRT_pfnOnHeapRealloc){
				(*_MCFCRT_pfnOnHeapRealloc)(pRet, pBlock, uSize, pRetAddr);
			}
			return pRet;
		}

		if(!(_MCFCRT_pfnOnHeapBadAlloc && (*_MCFCRT_pfnOnHeapBadAlloc)(pRetAddr))){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return nullptr;
		}
	}
}
void __MCFCRT_HeapFree(void *pBlock, const void *pRetAddr){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

	_MCFCRT_WaitForMutexForever(&g_vHeapMutex, HEAP_MUTEX_SPIN_COUNT);
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
		memset(pRaw, 0xFE, HeapSize(GetProcessHeap(), 0, pRaw));
#endif
		HeapFree(GetProcessHeap(), 0, pRaw);
	}
	_MCFCRT_SignalMutex(&g_vHeapMutex);

	if(_MCFCRT_pfnOnHeapFree){
		(*_MCFCRT_pfnOnHeapFree)(pBlock, pRetAddr);
	}
}
