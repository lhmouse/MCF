// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "heap.h"
#include "mcfwin.h"
#include "mutex.h"
#include "heap_debug.h"
#include "inline_mem.h"
#include "bail.h"

#define HEAP_DEBUG_MUTEX_SPIN_COUNT    1000u

static _MCFCRT_Mutex g_vMutex = { 0 };

#ifndef NDEBUG
#  undef __MCFCRT_HEAP_DEBUG
#  define __MCFCRT_HEAP_DEBUG     1
#endif

#ifdef __MCFCRT_HEAP_DEBUG
#  define HEAP_DEBUG_CALCULATE_SIZE_TO_ALLOC(n_)                 __MCFCRT_HeapDebugCalculateSizeToAlloc(n_)
#  define HEAP_DEBUG_REGISTER(ppb_, n_, ps_, ro_, ri_)           __MCFCRT_HeapDebugRegister(ppb_, n_, ps_, ro_, ri_)
#  define HEAP_DEBUG_VALIDATE_AND_UNREGISTER(pn_, pps_, pb_)     __MCFCRT_HeapDebugValidateAndUnregister(pn_, pps_, pb_)
#  define HEAP_DEBUG_UNDO_UNREGISTER(ps_)                        __MCFCRT_HeapDebugUndoUnregister(ps_)
#else
#  define HEAP_DEBUG_CALCULATE_SIZE_TO_ALLOC(n_)                 (size_t)(n_)
#  define HEAP_DEBUG_REGISTER(ppb_, n_, ps_, ro_, ri_)           (void)((*(ppb_) = (ps_)), (void)(n_), (void)(ro_), ri_)
#  define HEAP_DEBUG_VALIDATE_AND_UNREGISTER(pn_, pps_, pb_)     (bool)((void)(pn_), (*(pps_) = (pb_)), true)
#  define HEAP_DEBUG_UNDO_UNREGISTER(ps_)                        (void)(ps_)
#endif

void *__MCFCRT_HeapAlloc(size_t uNewSize, bool bFillsWithZero, const void *pRetAddrOuter){
	_MCFCRT_WaitForMutexForever(&g_vMutex, HEAP_DEBUG_MUTEX_SPIN_COUNT);
	DWORD dwFlags = 0;
	dwFlags |= bFillsWithZero * (DWORD)HEAP_ZERO_MEMORY;
	const size_t uSizeToAlloc = HEAP_DEBUG_CALCULATE_SIZE_TO_ALLOC(uNewSize);
	void *pNewStorage = HeapAlloc(GetProcessHeap(), dwFlags, uSizeToAlloc);
	if(!pNewStorage){
		_MCFCRT_SignalMutex(&g_vMutex);
		return _MCFCRT_NULLPTR;
	}
	void *pNewBlock;
	HEAP_DEBUG_REGISTER(&pNewBlock, uNewSize, pNewStorage, pRetAddrOuter, __builtin_return_address(0));
#ifdef __MCFCRT_HEAP_DEBUG
	if(!bFillsWithZero && (uNewSize > 0)){
		_MCFCRT_inline_mempset_fwd(pNewBlock, 0xCA, uNewSize);
	}
#endif
	const _MCFCRT_HeapAllocCallback pfnCallback = _MCFCRT_GetHeapAllocCallback();
	if(pfnCallback){
		(*pfnCallback)(pNewBlock, uNewSize, _MCFCRT_NULLPTR, pRetAddrOuter, __builtin_return_address(0));
	}
	_MCFCRT_SignalMutex(&g_vMutex);
	return pNewBlock;
}
void *__MCFCRT_HeapRealloc(void *pOldBlock, size_t uNewSize, bool bFillsWithZero, const void *pRetAddrOuter){
	_MCFCRT_WaitForMutexForever(&g_vMutex, HEAP_DEBUG_MUTEX_SPIN_COUNT);
	size_t uOldSize;
	void *pOldStorage;
	if(!HEAP_DEBUG_VALIDATE_AND_UNREGISTER(&uOldSize, &pOldStorage, pOldBlock)){
		_MCFCRT_Bail(L"__MCFCRT_HeapRealloc() 检测到堆损坏，这通常是错误的内存写入操作导致的。");
	}
#ifdef __MCFCRT_HEAP_DEBUG
	if(uOldSize > uNewSize){
		_MCFCRT_inline_mempset_fwd((char *)pOldBlock + uNewSize, 0xDB, uOldSize - uNewSize);
	}
#endif
	DWORD dwFlags = 0;
	dwFlags |= bFillsWithZero * (DWORD)HEAP_ZERO_MEMORY;
	const size_t uSizeToAlloc = HEAP_DEBUG_CALCULATE_SIZE_TO_ALLOC(uNewSize);
	void *pNewStorage = HeapReAlloc(GetProcessHeap(), dwFlags, pOldStorage, uSizeToAlloc);
	if(!pNewStorage){
		HEAP_DEBUG_UNDO_UNREGISTER(pOldStorage);
		_MCFCRT_SignalMutex(&g_vMutex);
		return _MCFCRT_NULLPTR;
	}
	void *pNewBlock;
	HEAP_DEBUG_REGISTER(&pNewBlock, uNewSize, pNewStorage, pRetAddrOuter, __builtin_return_address(0));
#ifdef __MCFCRT_HEAP_DEBUG
	if(!bFillsWithZero && (uNewSize > uOldSize)){
		_MCFCRT_inline_mempset_fwd((char *)pNewBlock + uOldSize, 0xCC, uNewSize - uOldSize);
	}
#endif
	const _MCFCRT_HeapAllocCallback pfnCallback = _MCFCRT_GetHeapAllocCallback();
	if(pfnCallback){
		(*pfnCallback)(pNewBlock, uNewSize, pOldBlock, pRetAddrOuter, __builtin_return_address(0));
	}
	_MCFCRT_SignalMutex(&g_vMutex);
	return pNewBlock;
}
void __MCFCRT_HeapFree(void *pOldBlock, const void *pRetAddrOuter){
	_MCFCRT_WaitForMutexForever(&g_vMutex, HEAP_DEBUG_MUTEX_SPIN_COUNT);
	size_t uOldSize;
	void *pOldStorage;
	if(!HEAP_DEBUG_VALIDATE_AND_UNREGISTER(&uOldSize, &pOldStorage, pOldBlock)){
		_MCFCRT_Bail(L"__MCFCRT_HeapFree() 检测到堆损坏，这通常是错误的内存写入操作导致的。");
	}
#ifdef __MCFCRT_HEAP_DEBUG
	if(uOldSize > 0){
		_MCFCRT_inline_mempset_fwd(pOldBlock, 0xDD, uOldSize);
	}
#endif
	DWORD dwFlags = 0;
	HeapFree(GetProcessHeap(), dwFlags, pOldStorage);
	const _MCFCRT_HeapAllocCallback pfnCallback = _MCFCRT_GetHeapAllocCallback();
	if(pfnCallback){
		(*pfnCallback)(_MCFCRT_NULLPTR, 0, pOldBlock, pRetAddrOuter, __builtin_return_address(0));
	}
	_MCFCRT_SignalMutex(&g_vMutex);
}

static volatile _MCFCRT_HeapAllocCallback g_pfnHeapAllocCallback = _MCFCRT_NULLPTR;

_MCFCRT_HeapAllocCallback _MCFCRT_GetHeapAllocCallback(void){
	return __atomic_load_n(&g_pfnHeapAllocCallback, __ATOMIC_CONSUME);
}
_MCFCRT_HeapAllocCallback _MCFCRT_SetHeapAllocCallback(_MCFCRT_HeapAllocCallback pfnNewCallback){
	return __atomic_exchange_n(&g_pfnHeapAllocCallback, pfnNewCallback, __ATOMIC_RELEASE);
}
