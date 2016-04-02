// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "heap_dbg.h"
#include "../ext/stpcpy.h"
#include "../ext/wcpcpy.h"
#include "../ext/itoa.h"
#include "../ext/itow.h"
#include "bail.h"
#include "mcfwin.h"

#define GUARD_BAND_SIZE     0x20u

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)

static HANDLE          g_hMapAllocator = nullptr;
static _MCFCRT_AvlRoot g_avlBlocks     = nullptr;

static int BlockInfoComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pInfo1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(((const __MCFCRT_HeapDbgBlockInfo *)pInfo1)->__pContents);
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int BlockInfoComparatorNodes(const _MCFCRT_AvlNodeHeader *pInfo1, const _MCFCRT_AvlNodeHeader *pInfo2){
	return BlockInfoComparatorNodeKey(pInfo1, (intptr_t)(((const __MCFCRT_HeapDbgBlockInfo *)pInfo2)->__pContents));
}

#endif

bool __MCFCRT_HeapDbgInit(){
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
void __MCFCRT_HeapDbgUninit(){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)
	if(g_avlBlocks){
		const HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
		if(hStdErr == INVALID_HANDLE_VALUE){
			_MCFCRT_Bail(L"__MCFCRT_HeapDbgUninit() 失败：侦测到内存泄漏。无法打开标准错误流，没有生成内存泄漏信息。");
		}
		if(hStdErr){
			const __MCFCRT_HeapDbgBlockInfo *pBlockInfo = (const __MCFCRT_HeapDbgBlockInfo *)_MCFCRT_AvlFront(&g_avlBlocks);
			do {
				const unsigned char *pbyDump = pBlockInfo->__pContents;

				char achTemp[1024];
				char *pchWrite = achTemp;
				pchWrite = _MCFCRT_stpcpy(pchWrite, "Memory leak: address = ");
				pchWrite = _MCFCRT_itoa0X(pchWrite, (uintptr_t)pbyDump, sizeof(pbyDump) * 2);
				pchWrite = _MCFCRT_stpcpy(pchWrite, ", size = ");
				pchWrite = _MCFCRT_itoa0X(pchWrite, (uintptr_t)pBlockInfo->__uSize, sizeof(pBlockInfo->__uSize) * 2);
				pchWrite = _MCFCRT_stpcpy(pchWrite, ", return address = ");
				pchWrite = _MCFCRT_itoa0X(pchWrite, (uintptr_t)pBlockInfo->__pRetAddr, sizeof(pBlockInfo->__pRetAddr) * 2);
				pchWrite = _MCFCRT_stpcpy(pchWrite, ", leading bytes =");
				for(size_t i = 0; i < 16; ++i){
					*(pchWrite++) = ' ';
					if(IsBadReadPtr(pbyDump, 1)){
						pchWrite = _MCFCRT_stpcpy(pchWrite, "??");
					} else {
						pchWrite = _MCFCRT_itoa0X(pchWrite, *pbyDump, 2);
					}
					++pbyDump;
				}
				*(pchWrite++) = '\n';

				DWORD dwBytesWritten;
				WriteFile(hStdErr, achTemp, (DWORD)(pchWrite - achTemp), &dwBytesWritten, nullptr);

				pBlockInfo = (const __MCFCRT_HeapDbgBlockInfo *)_MCFCRT_AvlNext((const _MCFCRT_AvlNodeHeader *)pBlockInfo);
			} while(pBlockInfo);
		}

		_MCFCRT_Bail(L"__MCFCRT_HeapDbgUninit() 失败：侦测到内存泄漏。内存泄漏的详细信息已经输出至标准错误流中。");
	}

	g_avlBlocks = nullptr;
	HeapDestroy(g_hMapAllocator);
	g_hMapAllocator = nullptr;
#endif
}

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)

size_t __MCFCRT_HeapDbgGetRawSize(size_t uContentSize){
	return uContentSize + GUARD_BAND_SIZE * 2;
}

#	if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)

__MCFCRT_HeapDbgBlockInfo *__MCFCRT_HeapDbgAllocateBlockInfo(){
	return HeapAlloc(g_hMapAllocator, 0, sizeof(__MCFCRT_HeapDbgBlockInfo));
}
void __MCFCRT_HeapDbgDeallocateBlockInfo(__MCFCRT_HeapDbgBlockInfo *pBlockInfo){
	HeapFree(g_hMapAllocator, 0, pBlockInfo);
}

unsigned char *__MCFCRT_HeapDbgRegisterBlockInfo(__MCFCRT_HeapDbgBlockInfo *pBlockInfo, unsigned char *pRaw, _MCFCRT_STD size_t uContentSize, const void *pRetAddr){
	unsigned char *const pContents = pRaw + GUARD_BAND_SIZE;
	const size_t uSize = uContentSize;

	pBlockInfo->__pContents = pContents;
	pBlockInfo->__uSize     = uContentSize;
	pBlockInfo->__pRetAddr  = pRetAddr;
	_MCFCRT_AvlAttach(&g_avlBlocks, (_MCFCRT_AvlNodeHeader *)pBlockInfo, &BlockInfoComparatorNodes);

	void **ppGuard1 = (void **)pContents;
	void **ppGuard2 = (void **)(pContents + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *const pTemp1 = EncodePointer(ppGuard2), *const pTemp2 = EncodePointer(ppGuard1);
		memcpy(ppGuard1, &pTemp1, sizeof(void *));
		memcpy(ppGuard2, &pTemp2, sizeof(void *));

		++ppGuard2;
	}

	return pContents;
}
__MCFCRT_HeapDbgBlockInfo *__MCFCRT_HeapDbgValidateBlock(unsigned char **ppRaw, unsigned char *pContents, const void *pRetAddr){
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	__MCFCRT_HeapDbgBlockInfo *const pBlockInfo = (__MCFCRT_HeapDbgBlockInfo *)_MCFCRT_AvlFind(&g_avlBlocks, (intptr_t)pContents, &BlockInfoComparatorNodeKey);
	if(!pBlockInfo){
		wchar_t awchTemp[256];
		wchar_t *pwcWrite = awchTemp;
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"__MCFCRT_HeapDbgValidate() 失败：传入的指针无效。\n返回地址：");
		pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)pRetAddr, sizeof(pRetAddr) * 2);
		*pwcWrite = 0;
		_MCFCRT_Bail(awchTemp);
	}
	const size_t uSize = pBlockInfo->__uSize;

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *pTemp1, *pTemp2;
		memcpy(&pTemp1, ppGuard1, sizeof(void *));
		memcpy(&pTemp2, ppGuard2, sizeof(void *));
		if((DecodePointer(pTemp1) != ppGuard2) || (DecodePointer(pTemp2) != ppGuard1)){
			wchar_t awchTemp[256];
			wchar_t *pwcWrite = awchTemp;
			pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"__MCFCRT_HeapDbgValidate() 失败：侦测到堆损坏。\n返回地址：");
			pwcWrite = _MCFCRT_itow0X(pwcWrite, (uintptr_t)pRetAddr, sizeof(pRetAddr) * 2);
			*pwcWrite = 0;
			_MCFCRT_Bail(awchTemp);
		}

		++ppGuard2;
	}

	return pBlockInfo;
}
void __MCFCRT_HeapDbgUnregisterBlockInfo(__MCFCRT_HeapDbgBlockInfo *pBlockInfo){
	_MCFCRT_AvlDetach((const _MCFCRT_AvlNodeHeader *)pBlockInfo);
}

#	else

unsigned char *__MCFCRT_HeapDbgAddBlockGuardsBasic(unsigned char *pRaw){
	unsigned char *const pContents = pRaw + GUARD_BAND_SIZE;
	const size_t uSize = HeapSize(GetProcessHeap(), 0, pRaw) - GUARD_BAND_SIZE * 2;

	void **ppGuard1 = (void **)pContents;
	void **ppGuard2 = (void **)(pContents + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *const pTemp1 = EncodePointer(ppGuard2), *const pTemp2 = EncodePointer(ppGuard1);
		memcpy(ppGuard1, &pTemp1, sizeof(void *));
		memcpy(ppGuard2, &pTemp2, sizeof(void *));

		++ppGuard2;
	}

	return pContents;
}
void __MCFCRT_HeapDbgValidateBlockBasic(unsigned char **ppRaw, unsigned char *pContents, const void *pRetAddr){
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	const size_t uSize = HeapSize(GetProcessHeap(), 0, pRaw) - GUARD_BAND_SIZE * 2;

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *pTemp1, *pTemp2;
		memcpy(&pTemp1, ppGuard1, sizeof(void *));
		memcpy(&pTemp2, ppGuard2, sizeof(void *));
		if((DecodePointer(pTemp1) != ppGuard2) || (DecodePointer(pTemp2) != ppGuard1)){
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
