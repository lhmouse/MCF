// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "heap.h"
#include "heap_dbg.h"
#include "hooks.h"
#include "mutex.h"
#include "mcfwin.h"
#include "bail.h"
#include "inline_mem.h"

static _MCFCRT_Mutex g_vHeapMutex = { 0 };

#define HEAP_MUTEX_SPIN_COUNT     4000u

bool __MCFCRT_HeapInit(void){
	// 启用 FLH，但是忽略任何错误。
	ULONG ulMagic = 2;
	HeapSetInformation(GetProcessHeap(), HeapCompatibilityInformation, &ulMagic, sizeof(ulMagic));

	return true;
}
void __MCFCRT_HeapUninit(void){
}

void *__MCFCRT_HeapAlloc(size_t uSize, bool bFillsWithZero, const void *pbyRetAddr){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
	const size_t uRawSize = __MCFCRT_HeapDbgGetRawSize(uSize);
	if(uRawSize < uSize){
		return _MCFCRT_NULLPTR;
	}
#else
	const size_t uRawSize = uSize;
#endif

	for(;;){
		unsigned char *pbyRet = _MCFCRT_NULLPTR;

		_MCFCRT_WaitForMutexForever(&g_vHeapMutex, HEAP_MUTEX_SPIN_COUNT);
		{
			unsigned char *const pbyRaw = HeapAlloc(GetProcessHeap(), bFillsWithZero ? HEAP_ZERO_MEMORY : 0, uRawSize);
			if(pbyRaw){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
				__MCFCRT_HeapDbgBlockInfo *const pBlockInfo = __MCFCRT_HeapDbgAllocateBlockInfo();
				if(!pBlockInfo){
					HeapFree(GetProcessHeap(), 0, pbyRaw);
					goto jFailed;
				}
				pbyRet = __MCFCRT_HeapDbgRegisterBlockInfo(pBlockInfo, pbyRaw, uSize, pbyRetAddr);
#elif __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
				pbyRet = __MCFCRT_HeapDbgAddBlockGuardsBasic(pbyRaw);
#else
				pbyRet = pbyRaw;
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
				if(!bFillsWithZero){
					_MCFCRT_inline_mempset(pbyRet, 0xCD, uSize);
				}
#endif
			}
		}
	jFailed: __attribute__((__unused__))
		;
		_MCFCRT_SignalMutex(&g_vHeapMutex);

		if(pbyRet){
			if(_MCFCRT_pfnOnHeapAlloc){
				(*_MCFCRT_pfnOnHeapAlloc)(pbyRet, uSize, pbyRetAddr);
			}
			return pbyRet;
		}

		if(!(_MCFCRT_pfnOnHeapBadAlloc && (*_MCFCRT_pfnOnHeapBadAlloc)(pbyRetAddr))){
			return _MCFCRT_NULLPTR;
		}
	}
}
void *__MCFCRT_HeapRealloc(void *pBlock, size_t uSize, bool bFillsWithZero, const void *pbyRetAddr){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
	const size_t uRawSize = __MCFCRT_HeapDbgGetRawSize(uSize);
	if(uRawSize < uSize){
		return _MCFCRT_NULLPTR;
	}
#else
	const size_t uRawSize = uSize;
#endif

	unsigned char *pbyRawOriginal;
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
		pBlockInfo = __MCFCRT_HeapDbgValidateBlock(&pbyRawOriginal, pBlock, pbyRetAddr);
#		if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
		if(pBlockInfo){
			uOriginalSize = pBlockInfo->__uSize;
		} else {
			uOriginalSize = HeapSize(GetProcessHeap(), 0, pbyRawOriginal);
		}
#		endif
#	else
		__MCFCRT_HeapDbgValidateBlockBasic(&pbyRawOriginal, pBlock, pbyRetAddr);
#	endif
	}
	_MCFCRT_SignalMutex(&g_vHeapMutex);
#else
	pbyRawOriginal = pBlock;
#endif

	for(;;){
		unsigned char *pbyRet = _MCFCRT_NULLPTR;

		_MCFCRT_WaitForMutexForever(&g_vHeapMutex, HEAP_MUTEX_SPIN_COUNT);
		{
			unsigned char *const pbyRaw = HeapReAlloc(GetProcessHeap(), bFillsWithZero ? HEAP_ZERO_MEMORY : 0, pbyRawOriginal, uRawSize);
			if(pbyRaw){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
				__MCFCRT_HeapDbgUnregisterBlockInfo(pBlockInfo);
				pbyRet = __MCFCRT_HeapDbgRegisterBlockInfo(pBlockInfo, pbyRaw, uSize, pbyRetAddr);
#elif __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
				pbyRet = __MCFCRT_HeapDbgAddBlockGuardsBasic(pbyRaw);
#else
				pbyRet = pbyRaw;
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
				if(!bFillsWithZero && (uOriginalSize < uSize)){
					_MCFCRT_inline_mempset(pbyRet + uOriginalSize, 0xCD, uSize - uOriginalSize);
				}
#endif
			}
		}
		_MCFCRT_SignalMutex(&g_vHeapMutex);

		if(pbyRet){
			if(_MCFCRT_pfnOnHeapRealloc){
				(*_MCFCRT_pfnOnHeapRealloc)(pbyRet, pBlock, uSize, pbyRetAddr);
			}
			return pbyRet;
		}

		if(!(_MCFCRT_pfnOnHeapBadAlloc && (*_MCFCRT_pfnOnHeapBadAlloc)(pbyRetAddr))){
			return _MCFCRT_NULLPTR;
		}
	}
}
void __MCFCRT_HeapFree(void *pBlock, const void *pbyRetAddr){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(1)
	SetLastError(0xDEADBEEF);
#endif

	_MCFCRT_WaitForMutexForever(&g_vHeapMutex, HEAP_MUTEX_SPIN_COUNT);
	{
		unsigned char *pbyRaw;
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
		__MCFCRT_HeapDbgBlockInfo *const pBlockInfo = __MCFCRT_HeapDbgValidateBlock(&pbyRaw, pBlock, pbyRetAddr);

		__MCFCRT_HeapDbgUnregisterBlockInfo(pBlockInfo);
		__MCFCRT_HeapDbgDeallocateBlockInfo(pBlockInfo);
#elif __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
		__MCFCRT_HeapDbgValidateBlockBasic(&pbyRaw, pBlock, pbyRetAddr);
#else
		pbyRaw = pBlock;
#endif

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(4)
		_MCFCRT_inline_mempset(pbyRaw, 0xFE, HeapSize(GetProcessHeap(), 0, pbyRaw));
#endif
		HeapFree(GetProcessHeap(), 0, pbyRaw);
	}
	_MCFCRT_SignalMutex(&g_vHeapMutex);

	if(_MCFCRT_pfnOnHeapFree){
		(*_MCFCRT_pfnOnHeapFree)(pBlock, pbyRetAddr);
	}
}
