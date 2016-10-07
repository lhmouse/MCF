// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "heap_dbg.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include "bail.h"
#include "standard_streams.h"
#include "mcfwin.h"

#define GUARD_BAND_SIZE     0x20u

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)

static inline int BlockInfoComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pInfo1, intptr_t nKey2){
	const uintptr_t u1 = (uintptr_t)(((const __MCFCRT_HeapDbgBlockInfo *)pInfo1)->__pAddress);
	const uintptr_t u2 = (uintptr_t)(void *)nKey2;
	if(u1 != u2){
		return (u1 < u2) ? -1 : 1;
	}
	return 0;
}
static inline int BlockInfoComparatorNodes(const _MCFCRT_AvlNodeHeader *pInfo1, const _MCFCRT_AvlNodeHeader *pInfo2){
	return BlockInfoComparatorNodeKey(pInfo1, (intptr_t)(((const __MCFCRT_HeapDbgBlockInfo *)pInfo2)->__pAddress));
}

static void DumpMemoryLeaks(_MCFCRT_AvlRoot avlBlocks){
	if(!avlBlocks){
		return;
	}

	for(const __MCFCRT_HeapDbgBlockInfo *pInfo = (const __MCFCRT_HeapDbgBlockInfo *)_MCFCRT_AvlFront(&avlBlocks);
		pInfo; pInfo = (const __MCFCRT_HeapDbgBlockInfo *)_MCFCRT_AvlNext((const _MCFCRT_AvlNodeHeader *)pInfo))
	{
		wchar_t awcBuffer[1024];
		wchar_t *pwcWrite = awcBuffer;
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"内存块泄漏：首字节地址 = ");
		pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)pInfo->__pAddress, sizeof(pInfo->__pAddress) * 2);
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"，大小 = ");
		pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)pInfo->__uSize, sizeof(pInfo->__uSize) * 2);
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"，返回地址 = ");
		pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)pInfo->__pRetAddr, sizeof(pInfo->__pRetAddr) * 2);
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"，首字节 =");
		for(size_t i = 0; i < 16; ++i){
			const unsigned char *const p = (const unsigned char *)pInfo->__pAddress + i;
			*(pwcWrite++) = ' ';
			if(IsBadReadPtr(p, 1)){
				pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"??");
			} else {
				pwcWrite = _MCFCRT_itow0X(pwcWrite, *p, 2);
			}
		}
		// *pwcWrite = 0;

		_MCFCRT_WriteStandardErrorText(awcBuffer, (size_t)(pwcWrite - awcBuffer), true);
	}
	_MCFCRT_Bail(L"__MCFCRT_HeapDbgUninit() 失败：侦测到内存泄漏。内存泄漏的详细信息已经输出至标准错误流中。");
}

static HANDLE          g_hMapAllocator = nullptr;
static _MCFCRT_AvlRoot g_avlBlocks     = nullptr;

#endif

bool __MCFCRT_HeapDbgInit(void){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
	const HANDLE hAllocator = HeapCreate(0, 0, 0);
	if(!hAllocator){
		return false;
	}

	// 启用 FLH，但是忽略任何错误。
	ULONG ulMagic = 2;
	HeapSetInformation(hAllocator, HeapCompatibilityInformation, &ulMagic, sizeof(ulMagic));

	g_hMapAllocator = hAllocator;
#endif
	return true;
}
void __MCFCRT_HeapDbgUninit(void){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
	const HANDLE hAllocator = g_hMapAllocator;
	g_hMapAllocator = nullptr;

	const _MCFCRT_AvlRoot avlBlocks = g_avlBlocks;
	g_avlBlocks = nullptr;

	DumpMemoryLeaks(avlBlocks);
	HeapDestroy(hAllocator);
#endif
}

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)

size_t __MCFCRT_HeapDbgGetRawSize(size_t uContentSize){
	return uContentSize + GUARD_BAND_SIZE * 2;
}

#	if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)

__MCFCRT_HeapDbgBlockInfo *__MCFCRT_HeapDbgAllocateBlockInfo(void){
	return HeapAlloc(g_hMapAllocator, 0, sizeof(__MCFCRT_HeapDbgBlockInfo));
}
void __MCFCRT_HeapDbgDeallocateBlockInfo(__MCFCRT_HeapDbgBlockInfo *pInfo){
	HeapFree(g_hMapAllocator, 0, pInfo);
}

unsigned char *__MCFCRT_HeapDbgRegisterBlockInfo(__MCFCRT_HeapDbgBlockInfo *pInfo, unsigned char *pRaw, _MCFCRT_STD size_t uContentSize, const void *pRetAddr){
	unsigned char *const pAddress = pRaw + GUARD_BAND_SIZE;
	const size_t uSize = uContentSize;

	pInfo->__pAddress = pAddress;
	pInfo->__uSize     = uContentSize;
	pInfo->__pRetAddr  = pRetAddr;
	_MCFCRT_AvlAttach(&g_avlBlocks, (_MCFCRT_AvlNodeHeader *)pInfo, &BlockInfoComparatorNodes);

	void **ppGuard1 = (void **)pAddress;
	void **ppGuard2 = (void **)(pAddress + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *const pTemp1 = EncodeSystemPointer(ppGuard2), *const pTemp2 = EncodeSystemPointer(ppGuard1);
		memcpy(ppGuard1, &pTemp1, sizeof(void *));
		memcpy(ppGuard2, &pTemp2, sizeof(void *));

		++ppGuard2;
	}

	return pAddress;
}
__MCFCRT_HeapDbgBlockInfo *__MCFCRT_HeapDbgValidateBlock(unsigned char **ppRaw, unsigned char *pAddress, const void *pRetAddr){
	unsigned char *const pRaw = pAddress - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	__MCFCRT_HeapDbgBlockInfo *const pInfo = (__MCFCRT_HeapDbgBlockInfo *)_MCFCRT_AvlFind(&g_avlBlocks, (intptr_t)pAddress, &BlockInfoComparatorNodeKey);
	if(!pInfo){
		wchar_t awchTemp[256];
		wchar_t *pwcWrite = awchTemp;
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"__MCFCRT_HeapDbgValidate() 失败：传入的指针无效。\n返回地址：");
		pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)pRetAddr, sizeof(pRetAddr) * 2);
		*pwcWrite = 0;
		_MCFCRT_Bail(awchTemp);
	}
	const size_t uSize = pInfo->__uSize;

	void *const *ppGuard1 = (void *const *)pAddress;
	void *const *ppGuard2 = (void *const *)(pAddress + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *pTemp1, *pTemp2;
		memcpy(&pTemp1, ppGuard1, sizeof(void *));
		memcpy(&pTemp2, ppGuard2, sizeof(void *));
		if((DecodeSystemPointer(pTemp1) != ppGuard2) || (DecodeSystemPointer(pTemp2) != ppGuard1)){
			wchar_t awchTemp[256];
			wchar_t *pwcWrite = awchTemp;
			pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"__MCFCRT_HeapDbgValidate() 失败：侦测到堆损坏。\n返回地址：");
			pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)pRetAddr, sizeof(pRetAddr) * 2);
			*pwcWrite = 0;
			_MCFCRT_Bail(awchTemp);
		}

		++ppGuard2;
	}

	return pInfo;
}
void __MCFCRT_HeapDbgUnregisterBlockInfo(__MCFCRT_HeapDbgBlockInfo *pInfo){
	_MCFCRT_AvlDetach((const _MCFCRT_AvlNodeHeader *)pInfo);
}

#	else

unsigned char *__MCFCRT_HeapDbgAddBlockGuardsBasic(unsigned char *pRaw){
	unsigned char *const pAddress = pRaw + GUARD_BAND_SIZE;
	const size_t uSize = HeapSize(GetProcessHeap(), 0, pRaw) - GUARD_BAND_SIZE * 2;

	void **ppGuard1 = (void **)pAddress;
	void **ppGuard2 = (void **)(pAddress + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *const pTemp1 = EncodeSystemPointer(ppGuard2), *const pTemp2 = EncodeSystemPointer(ppGuard1);
		memcpy(ppGuard1, &pTemp1, sizeof(void *));
		memcpy(ppGuard2, &pTemp2, sizeof(void *));

		++ppGuard2;
	}

	return pAddress;
}
void __MCFCRT_HeapDbgValidateBlockBasic(unsigned char **ppRaw, unsigned char *pAddress, const void *pRetAddr){
	unsigned char *const pRaw = pAddress - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	const size_t uSize = HeapSize(GetProcessHeap(), 0, pRaw) - GUARD_BAND_SIZE * 2;

	void *const *ppGuard1 = (void *const *)pAddress;
	void *const *ppGuard2 = (void *const *)(pAddress + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *pTemp1, *pTemp2;
		memcpy(&pTemp1, ppGuard1, sizeof(void *));
		memcpy(&pTemp2, ppGuard2, sizeof(void *));
		if((DecodeSystemPointer(pTemp1) != ppGuard2) || (DecodeSystemPointer(pTemp2) != ppGuard1)){
			wchar_t awchTemp[256];
			wchar_t *pwcWrite = awchTemp;
			pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"__MCFCRT_HeapDbgValidate() 失败：侦测到堆损坏。\n返回地址：");
			pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)pRetAddr, sizeof(pRetAddr) * 2);
			*pwcWrite = 0;
			_MCFCRT_Bail(awchTemp);
		}

		++ppGuard2;
	}
}

#	endif

#endif
