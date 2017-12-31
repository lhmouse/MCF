// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "heap.h"
#include "mcfwin.h"
#include "mutex.h"
#include "heap_debug.h"
#include "inline_mem.h"
#include "bail.h"

#ifndef NDEBUG
#  undef __MCFCRT_HEAP_DEBUG
#  define __MCFCRT_HEAP_DEBUG     1
#endif

static inline void *Underlying_malloc_zf(size_t size, bool zero_fill){
	return LocalAlloc(zero_fill ? LPTR : LMEM_FIXED, size);
}
static inline void *Underlying_realloc_zf(void *ptr, size_t size, bool zero_fill){
	return LocalReAlloc(ptr, size, zero_fill ? LPTR : LMEM_FIXED);
}
static inline void Underlying_free(void *ptr){
	LocalFree(ptr);
}

static inline void InvokeHeapCallback(void *pBlockNew, size_t uSizeNew, void *pBlockOld, const void *pRetAddrOuter, const void *pRetAddrInner){
	const _MCFCRT_HeapCallback pfnCallback = _MCFCRT_GetHeapCallback();
	if(!pfnCallback){
		return;
	}
	(*pfnCallback)(pBlockNew, uSizeNew, pBlockOld, pRetAddrOuter, pRetAddrInner);
}

void *__MCFCRT_HeapAlloc(size_t uSizeNew, bool bFillsWithZero, const void *pRetAddrOuter){
	size_t uSizeToAlloc;
	void *pStorageNew, *pBlockNew;

#ifdef __MCFCRT_HEAP_DEBUG
	// Clobber the per-thread error code unconditionally in debug mode.
	SetLastError(0xDEADBEEF);
	// Include the size of additional debug information if requested.
	uSizeToAlloc = __MCFCRT_HeapDebugCalculateSizeToAlloc(uSizeNew);
#else
	uSizeToAlloc = uSizeNew;
#endif
	// Perform the allocation.
	pStorageNew = Underlying_malloc_zf(uSizeToAlloc, bFillsWithZero);
	if(!pStorageNew){
		return _MCFCRT_NULLPTR;
	}
#ifdef __MCFCRT_HEAP_DEBUG
	// Register it and adjust the pointer.
	__MCFCRT_HeapDebugRegister(&pBlockNew, uSizeNew, pStorageNew, pRetAddrOuter, __builtin_return_address(0));
	if(!bFillsWithZero && (uSizeNew > 0)){
		// If any bytes have been allocated, poison those that are considered uninitialized.
		_MCFCRT_inline_mempset_fwd(pBlockNew, 0xCA, uSizeNew);
	}
#else
	pBlockNew = pStorageNew;
#endif

	// Invoke the heap callback in the end, if any.
	InvokeHeapCallback(pBlockNew, uSizeNew, _MCFCRT_NULLPTR, pRetAddrOuter, __builtin_return_address(0));
	return pBlockNew;
}
void *__MCFCRT_HeapRealloc(void *pBlockOld, size_t uSizeNew, bool bFillsWithZero, const void *pRetAddrOuter){
	size_t uSizeOld, uSizeToAlloc;
	void *pStorageOld, *pStorageNew, *pBlockNew;

#ifdef __MCFCRT_HEAP_DEBUG
	// Clobber the per-thread error code unconditionally in debug mode.
	SetLastError(0xDEADBEEF);
	// Make sure the old block is not corrupted.
	if(!__MCFCRT_HeapDebugValidateAndUnregister(&uSizeOld, &pStorageOld, pBlockOld)){
		_MCFCRT_Bail(L"__MCFCRT_HeapRealloc() 检测到堆损坏，这通常是错误的内存写入操作导致的。");
	}
	if(uSizeOld > uSizeNew){
		// If the block is to be shrinked, poison bytes that are to be discarded.
		_MCFCRT_inline_mempset_fwd((unsigned char *)pBlockOld + uSizeNew, 0xDB, uSizeOld - uSizeNew);
	}
	// Include the size of additional debug information if requested.
	uSizeToAlloc = __MCFCRT_HeapDebugCalculateSizeToAlloc(uSizeNew);
#else
	(void)uSizeOld;
	pStorageOld = pBlockOld;
	uSizeToAlloc = uSizeNew;
#endif
	// Perform the reallocation.
	pStorageNew = Underlying_realloc_zf(pStorageOld, uSizeToAlloc, bFillsWithZero);
	if(!pStorageNew){
#ifdef __MCFCRT_HEAP_DEBUG
		// Stuff it back...
		__MCFCRT_HeapDebugUndoUnregister(pStorageOld);
#endif
		return _MCFCRT_NULLPTR;
	}
#ifdef __MCFCRT_HEAP_DEBUG
	// Register it and adjust the pointer.
	__MCFCRT_HeapDebugRegister(&pBlockNew, uSizeNew, pStorageNew, pRetAddrOuter, __builtin_return_address(0));
	if(!bFillsWithZero && (uSizeNew > uSizeOld)){
		// If the block has been extended, poison bytes that are considered uninitialized.
		_MCFCRT_inline_mempset_fwd((unsigned char *)pBlockNew + uSizeOld, 0xCC, uSizeNew - uSizeOld);
	}
#else
	pBlockNew = pStorageNew;
#endif

	// Invoke the heap callback in the end, if any.
	InvokeHeapCallback(pBlockNew, uSizeNew, pBlockOld, pRetAddrOuter, __builtin_return_address(0));
	return pBlockNew;
}
void __MCFCRT_HeapFree(void *pBlockOld, const void *pRetAddrOuter){
	size_t uSizeOld;
	void *pStorageOld;

#ifdef __MCFCRT_HEAP_DEBUG
	// Clobber the per-thread error code unconditionally in debug mode.
	SetLastError(0xDEADBEEF);
	// Make sure the old block is not corrupted.
	if(!__MCFCRT_HeapDebugValidateAndUnregister(&uSizeOld, &pStorageOld, pBlockOld)){
		_MCFCRT_Bail(L"__MCFCRT_HeapFree() 检测到堆损坏，这通常是错误的内存写入操作导致的。");
	}
	if(uSizeOld > 0){
		// If any bytes are to be freed, poison those that are to be discarded.
		_MCFCRT_inline_mempset_fwd(pBlockOld, 0xDD, uSizeOld);
	}
#else
	(void)uSizeOld;
	pStorageOld = pBlockOld;
#endif
	// Perform the deallocation.
	Underlying_free(pStorageOld);

	// Invoke the heap callback in the end, if any.
	InvokeHeapCallback(_MCFCRT_NULLPTR, 0, pBlockOld, pRetAddrOuter, __builtin_return_address(0));
}

static volatile _MCFCRT_HeapCallback g_pfnHeapCallback = _MCFCRT_NULLPTR;

_MCFCRT_HeapCallback _MCFCRT_GetHeapCallback(void){
	return __atomic_load_n(&g_pfnHeapCallback, __ATOMIC_ACQUIRE);
}
_MCFCRT_HeapCallback _MCFCRT_SetHeapCallback(_MCFCRT_HeapCallback pfnNewCallback){
	return __atomic_exchange_n(&g_pfnHeapCallback, pfnNewCallback, __ATOMIC_RELEASE);
}
