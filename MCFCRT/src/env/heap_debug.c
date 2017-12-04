// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "heap_debug.h"
#include "avl_tree.h"
#include "mutex.h"
#include "inline_mem.h"
#include "standard_streams.h"
#include "bail.h"
#include "clocks.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"

typedef struct tagBlockHeader {
	_MCFCRT_AvlNodeHeader avlhBlockIndex;
	size_t uSize;
	const void *pRetAddrOuter;
	const void *pRetAddrInner;
	uintptr_t uCookie;
	uintptr_t uReserved;
	unsigned char abySentry[16];
} BlockHeader;

static_assert(sizeof(BlockHeader) % alignof(max_align_t) == 0, "??");

static inline int BlockHeaderComparatorNodeHeader(const _MCFCRT_AvlNodeHeader *pInfoSelf, intptr_t nHeaderOther){
	const uintptr_t uHeaderSelf = (uintptr_t)(BlockHeader *)pInfoSelf;
	const uintptr_t uHeaderOther = (uintptr_t)nHeaderOther;
	if(uHeaderSelf != uHeaderOther){
		return (uHeaderSelf < uHeaderOther) ? -1 : 1;
	}
	return 0;
}
static inline int BlockHeaderComparatorNodes(const _MCFCRT_AvlNodeHeader *pInfoSelf, const _MCFCRT_AvlNodeHeader *pInfoOther){
	return BlockHeaderComparatorNodeHeader(pInfoSelf, (intptr_t)(uintptr_t)(BlockHeader *)pInfoOther);
}

typedef struct tagBlockTrailer {
	unsigned char abySentry[64];
} BlockTrailer;

static_assert(sizeof(BlockTrailer) % alignof(max_align_t) == 0, "??");

static inline unsigned char LcgGetByte(unsigned long *pulSeed){
	unsigned long ulSeed = *pulSeed;
	ulSeed = ulSeed * 1664525u + 1013904223u;
	*pulSeed = ulSeed;
	return (unsigned char)(ulSeed >> 24);
}

__attribute__((__noinline__, __noclone__))
static void MakeSentry(unsigned char *pbyData, size_t uSize, uintptr_t uCookie){
	unsigned long ulSeed = (unsigned long)uCookie;
	for(size_t uIndex = 0; uIndex < uSize; ++uIndex){
		pbyData[uIndex] = LcgGetByte(&ulSeed);
	}
}
__attribute__((__noinline__, __noclone__))
static bool CheckSentry(uintptr_t uCookie, const unsigned char *pbyData, size_t uSize){
	unsigned long ulSeed = (unsigned long)uCookie;
	for(size_t uIndex = 0; uIndex < uSize; ++uIndex){
		if(pbyData[uIndex] != LcgGetByte(&ulSeed)){
			return false;
		}
	}
	return true;
}

static _MCFCRT_Mutex g_vMutex = { 0 };
static _MCFCRT_AvlRoot g_avlBlocks = _MCFCRT_NULLPTR;

static void CheckForMemoryLeaksUnlocked(void){
	wchar_t awcLine[1024];
	uintptr_t uCount = 0;
	const BlockHeader *pHeader = (BlockHeader *)_MCFCRT_AvlFront(&g_avlBlocks);
	while(pHeader){
		++uCount;
		if(uCount <= 9999){
			wchar_t *pwcWrite = awcLine;
			pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"*** Memory leak ");
			pwcWrite = _MCFCRT_itow0u(pwcWrite, uCount, 4);
			pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L": address = 0x");
			pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)((char *)pHeader + sizeof(BlockHeader)), sizeof(void *) * 2);
			pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L", size = 0x");
			pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)(pHeader->uSize), sizeof(size_t) * 2);
			pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L", allocated from 0x");
			pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)(pHeader->pRetAddrInner), sizeof(void *) * 2);
			pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L" inside 0x");
			pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)(pHeader->pRetAddrOuter), sizeof(void *) * 2);
			pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L" ***");
			_MCFCRT_WriteStandardErrorText(awcLine, (size_t)(pwcWrite - awcLine), true);
		}
		pHeader = (BlockHeader *)_MCFCRT_AvlNext((_MCFCRT_AvlNodeHeader *)pHeader);
	}
	if(uCount > 9999){
		wchar_t *pwcWrite = awcLine;
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"*** ... ");
		const uintptr_t uCountRemaining = uCount - 9999;
		pwcWrite = _MCFCRT_itow_u(pwcWrite, uCountRemaining);
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, (uCountRemaining == 1) ? L" more memory leak follows. ***"
		                                                           : L" more memory leaks follow. ***");
		_MCFCRT_WriteStandardErrorText(awcLine, (size_t)(pwcWrite - awcLine), true);
	}
	if(uCount != 0){
		_MCFCRT_Bail(L"检测到内存泄漏。内存泄漏的详细信息已经输出至标准错误流中。");
	}
}

bool __MCFCRT_HeapDebugInit(void){
	return true;
}
void __MCFCRT_HeapDebugUninit(void){
	CheckForMemoryLeaksUnlocked();
}

size_t __MCFCRT_HeapDebugCalculateSizeToAlloc(size_t uSize){
	size_t uSizeToAlloc;
	if(__builtin_add_overflow(uSize, sizeof(BlockHeader) + sizeof(BlockTrailer), &uSizeToAlloc)){
		return SIZE_MAX;
	}
	return uSizeToAlloc;
}
void __MCFCRT_HeapDebugRegister(void **restrict ppBlock, size_t uSize, void *pStorage, const void *pRetAddrOuter, const void *pRetAddrInner){
	BlockHeader *const pHeader = pStorage;
	void *const pBlock = (char *)pHeader + sizeof(BlockHeader);
	BlockTrailer *const pTrailer = (void *)((char *)pHeader + sizeof(BlockHeader) + uSize);

	// Initialize the header.
	pHeader->uSize = uSize;
	pHeader->pRetAddrOuter = pRetAddrOuter;
	pHeader->pRetAddrInner = pRetAddrInner;
	pHeader->uCookie = (uintptr_t)_MCFCRT_GetFastMonoClock();
	pHeader->uReserved = 0;
	MakeSentry(pHeader->abySentry, sizeof(pHeader->abySentry), pHeader->uCookie);
	// Initialize the trailer.
	MakeSentry(pTrailer->abySentry, sizeof(pTrailer->abySentry), pHeader->uCookie);

	// Register it.
	_MCFCRT_WaitForMutexForever(&g_vMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	_MCFCRT_AvlAttach(&g_avlBlocks, (_MCFCRT_AvlNodeHeader *)pStorage, &BlockHeaderComparatorNodes);
	_MCFCRT_SignalMutex(&g_vMutex);

	*ppBlock = pBlock;
}
bool __MCFCRT_HeapDebugValidateAndUnregister(size_t *restrict puSize, void **restrict ppStorage, void *pBlock){
	void *const pStorage = (char *)pBlock - sizeof(BlockHeader);
	BlockHeader *const pHeader = pStorage;
	const size_t uSize = pHeader->uSize;
	BlockTrailer *const pTrailer = (void *)((char *)pHeader + sizeof(BlockHeader) + uSize);

	// Check the header.
	if(!CheckSentry(pHeader->uCookie, pHeader->abySentry, sizeof(pHeader->abySentry))){
		return false;
	}
	// Check the trailer.
	if(!CheckSentry(pHeader->uCookie, pTrailer->abySentry, sizeof(pTrailer->abySentry))){
		return false;
	}

	// Search for it in all registered blocks. Detach it if one is found.
	_MCFCRT_WaitForMutexForever(&g_vMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	BlockHeader *const pHeaderFound = (BlockHeader *)_MCFCRT_AvlFind(&g_avlBlocks, (intptr_t)pHeader, &BlockHeaderComparatorNodeHeader);
	if(pHeaderFound != pHeader){
		_MCFCRT_SignalMutex(&g_vMutex);
		return false;
	}
	_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)pStorage);
	_MCFCRT_SignalMutex(&g_vMutex);

	// Leave the header alone in order to enable the unregistration to be reverted.
	// Zero out the trailer so the storage can be passed to `HeapReAlloc()` with the `HEAP_ZERO_MEMORY` option without causing confusion.
	_MCFCRT_inline_mempset_fwd(pTrailer, 0, sizeof(*pTrailer));

	*ppStorage = pStorage;
	*puSize = uSize;
	return true;
}
void __MCFCRT_HeapDebugUndoUnregister(void *pStorage){
	BlockHeader *const pHeader = pStorage;
	const size_t uSize = pHeader->uSize;

	// Generate a new cookie and update the header sentry.
	pHeader->uCookie = (uintptr_t)_MCFCRT_GetFastMonoClock();
	pHeader->uReserved = 0;
	MakeSentry(pHeader->abySentry, sizeof(pHeader->abySentry), pHeader->uCookie);
	// Reinitialize the trailer.
	BlockTrailer *const pTrailer = (void *)((char *)pHeader + sizeof(BlockHeader) + uSize);
	MakeSentry(pTrailer->abySentry, sizeof(pTrailer->abySentry), pHeader->uCookie);

	// Re-register it.
	_MCFCRT_WaitForMutexForever(&g_vMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	_MCFCRT_AvlAttach(&g_avlBlocks, (_MCFCRT_AvlNodeHeader *)pStorage, &BlockHeaderComparatorNodes);
	_MCFCRT_SignalMutex(&g_vMutex);
}
