// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "heap_dbg.h"
#include "heap.h"
#include "../ext/stpcpy.h"
#include "bail.h"
#include "mcfwin.h"
#include <stdio.h>

#define GUARD_BAND_SIZE     0x20u

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)

static HANDLE          g_hMapAllocator = nullptr;
static MCFCRT_AvlRoot  g_avlBlocks     = nullptr;

static int BlockInfoComparatorNodeKey(const MCFCRT_AvlNodeHeader *pInfo1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(((const __MCFCRT_HeapDbgBlockInfo *)pInfo1)->__pContents);
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int BlockInfoComparatorNodes(const MCFCRT_AvlNodeHeader *pInfo1, const MCFCRT_AvlNodeHeader *pInfo2){
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
			MCFCRT_Bail(L"__MCFCRT_HeapDbgUninit() 失败：侦测到内存泄漏。无法打开标准错误流，没有生成内存泄漏信息。");
		}

		const __MCFCRT_HeapDbgBlockInfo *pBlockInfo = (const __MCFCRT_HeapDbgBlockInfo *)MCFCRT_AvlFront(&g_avlBlocks);
		do {
			const unsigned char *pbyDump = pBlockInfo->__pContents;

			char achTemp[1024];
			char *pchWrite = achTemp + sprintf(achTemp,
				"Memory leak: address = %p, size = %p, return address = %p, leading bytes =",
				(void *)pbyDump, (void *)pBlockInfo->__uSize, (void *)pBlockInfo->__pRetAddr);
			for(size_t i = 0; i < 16; ++i){
				*(pchWrite++) = ' ';
				if(IsBadReadPtr(pbyDump, 1)){
					*(pchWrite++) = '?';
					*(pchWrite++) = '?';
				} else {
					static const char kHexTable[16] = "0123456789ABCDEF";
					*(pchWrite++) = kHexTable[(*pbyDump) >> 4];
					*(pchWrite++) = kHexTable[(*pbyDump) & 0x0F];
				}
				++pbyDump;
			}
			*(pchWrite++) = '\n';

			const char *pchRead = achTemp;
			for(;;){
				const DWORD dwBytesToWrite = (DWORD)(pchWrite - pchRead);
				if(dwBytesToWrite == 0){
					break;
				}
				DWORD dwBytesWritten;
				if(!WriteFile(hStdErr, pchRead, dwBytesToWrite, &dwBytesWritten, nullptr)){
					break;
				}
				pchRead += dwBytesWritten;
			}

			pBlockInfo = (const __MCFCRT_HeapDbgBlockInfo *)MCFCRT_AvlNext((const MCFCRT_AvlNodeHeader *)pBlockInfo);
		} while(pBlockInfo);

		MCFCRT_Bail(L"__MCFCRT_HeapDbgUninit() 失败：侦测到内存泄漏。内存泄漏的详细信息已经输出至标准错误流中。");
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

unsigned char *__MCFCRT_HeapDbgRegisterBlockInfo(__MCFCRT_HeapDbgBlockInfo *pBlockInfo, unsigned char *pRaw, MCFCRT_STD size_t uContentSize, const void *pRetAddr){
	unsigned char *const pContents = pRaw + GUARD_BAND_SIZE;
	const size_t uSize = uContentSize;

	pBlockInfo->__pContents = pContents;
	pBlockInfo->__uSize     = uContentSize;
	pBlockInfo->__pRetAddr  = pRetAddr;
	MCFCRT_AvlAttach(&g_avlBlocks, (MCFCRT_AvlNodeHeader *)pBlockInfo, &BlockInfoComparatorNodes);

	void **ppGuard1 = (void **)pContents;
	void **ppGuard2 = (void **)(pContents + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *const pTemp1 = EncodePointer(ppGuard2), *const pTemp2 = EncodePointer(ppGuard1);
		__builtin_memcpy(ppGuard1, &pTemp1, sizeof(void *));
		__builtin_memcpy(ppGuard2, &pTemp2, sizeof(void *));

		++ppGuard2;
	}

	return pContents;
}
__MCFCRT_HeapDbgBlockInfo *__MCFCRT_HeapDbgValidateBlock(unsigned char **ppRaw, unsigned char *pContents, const void *pRetAddr){
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	__MCFCRT_HeapDbgBlockInfo *const pBlockInfo = (__MCFCRT_HeapDbgBlockInfo *)MCFCRT_AvlFind(&g_avlBlocks, (intptr_t)pContents, &BlockInfoComparatorNodeKey);
	if(!pBlockInfo){
		MCFCRT_BailF(L"__MCFCRT_HeapDbgValidate() 失败：传入的指针无效。\n调用返回地址：%p", pRetAddr);
	}
	const size_t uSize = pBlockInfo->__uSize;

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *pTemp1, *pTemp2;
		__builtin_memcpy(&pTemp1, ppGuard1, sizeof(void *));
		__builtin_memcpy(&pTemp2, ppGuard2, sizeof(void *));
		if((DecodePointer(pTemp1) != ppGuard2) || (DecodePointer(pTemp2) != ppGuard1)){
			MCFCRT_BailF(L"__MCFCRT_HeapDbgValidate() 失败：侦测到堆损坏。\n调用返回地址：%p", pRetAddr);
		}

		++ppGuard2;
	}

	return pBlockInfo;
}
void __MCFCRT_HeapDbgUnregisterBlockInfo(__MCFCRT_HeapDbgBlockInfo *pBlockInfo){
	MCFCRT_AvlDetach((const MCFCRT_AvlNodeHeader *)pBlockInfo);
}

#	else

unsigned char *__MCFCRT_HeapDbgAddBlockGuardsBasic(unsigned char *pRaw){
	unsigned char *const pContents = pRaw + GUARD_BAND_SIZE;
	const size_t uSize = __MCFCRT_ReallyGetUsableSize(pRaw) - GUARD_BAND_SIZE * 2;

	void **ppGuard1 = (void **)pContents;
	void **ppGuard2 = (void **)(pContents + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *const pTemp1 = EncodePointer(ppGuard2), *const pTemp2 = EncodePointer(ppGuard1);
		__builtin_memcpy(ppGuard1, &pTemp1, sizeof(void *));
		__builtin_memcpy(ppGuard2, &pTemp2, sizeof(void *));

		++ppGuard2;
	}

	return pContents;
}
void __MCFCRT_HeapDbgValidateBlockBasic(unsigned char **ppRaw, unsigned char *pContents, const void *pRetAddr){
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	const size_t uSize = __MCFCRT_ReallyGetUsableSize(pRaw) - GUARD_BAND_SIZE * 2;

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + uSize);
	for(size_t i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *pTemp1, *pTemp2;
		__builtin_memcpy(&pTemp1, ppGuard1, sizeof(void *));
		__builtin_memcpy(&pTemp2, ppGuard2, sizeof(void *));
		if((DecodePointer(pTemp1) != ppGuard2) || (DecodePointer(pTemp2) != ppGuard1)){
			MCFCRT_BailF(L"__MCFCRT_HeapDbgValidate() 失败：侦测到堆损坏。\n调用返回地址：%p", pRetAddr);
		}

		++ppGuard2;
	}
}

#	endif

#endif
