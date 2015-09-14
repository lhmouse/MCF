// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "heap_dbg.h"
#include "heap.h"
#include "../ext/stpcpy.h"
#include "../ext/unref_param.h"
#include "bail.h"
#include "mcfwin.h"
#include <stdio.h>

#define GUARD_BAND_SIZE     0x20u

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)

static HANDLE       g_hMapAllocator;
static MCF_AvlRoot  g_pavlBlocks;

static int BlockInfoComparatorNodeKey(const MCF_AvlNodeHeader *pInfo1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(((const __MCF_HeapDbgBlockInfo *)pInfo1)->__pContents);
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int BlockInfoComparatorNodes(const MCF_AvlNodeHeader *pInfo1, const MCF_AvlNodeHeader *pInfo2){
	return BlockInfoComparatorNodeKey(pInfo1, (intptr_t)(((const __MCF_HeapDbgBlockInfo *)pInfo2)->__pContents));
}

#endif

bool __MCF_CRT_HeapDbgInit(){
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
	g_hMapAllocator = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	if(!g_hMapAllocator){
		return false;
	}
#endif
	return true;
}
void __MCF_CRT_HeapDbgUninit(){
#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)
	if(g_pavlBlocks){
		const HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
		if(hStdErr == INVALID_HANDLE_VALUE){
			MCF_CRT_Bail(L"__MCF_CRT_HeapDbgUninit() 失败：侦测到内存泄漏。无法打开标准错误流，没有生成内存泄漏信息。");
		}

		const __MCF_HeapDbgBlockInfo *pBlockInfo = (const __MCF_HeapDbgBlockInfo *)MCF_AvlFront(&g_pavlBlocks);
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

			pBlockInfo = (const __MCF_HeapDbgBlockInfo *)MCF_AvlNext((const MCF_AvlNodeHeader *)pBlockInfo);
		} while(pBlockInfo);

		MCF_CRT_Bail(L"__MCF_CRT_HeapDbgUninit() 失败：侦测到内存泄漏。内存泄漏的详细信息已经输出至标准错误流中。");
	}

	g_pavlBlocks = nullptr;
	HeapDestroy(g_hMapAllocator);
	g_hMapAllocator = nullptr;
#endif
}

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)

size_t __MCF_CRT_HeapDbgGetRawSize(size_t uContentSize){
	return uContentSize + GUARD_BAND_SIZE * 2;
}

#	if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)

__MCF_HeapDbgBlockInfo *__MCF_CRT_HeapDbgAllocateBlockInfo(){
	return HeapAlloc(g_hMapAllocator, 0, sizeof(__MCF_HeapDbgBlockInfo));
}
void __MCF_CRT_HeapDbgDeallocateBlockInfo(__MCF_HeapDbgBlockInfo *pBlockInfo){
	HeapFree(g_hMapAllocator, 0, pBlockInfo);
}

unsigned char *__MCF_CRT_HeapDbgRegisterBlockInfo(__MCF_HeapDbgBlockInfo *pBlockInfo, unsigned char *pRaw, MCF_STD size_t uContentSize, const void *pRetAddr){
	unsigned char *const pContents = pRaw + GUARD_BAND_SIZE;
	const size_t uSize = uContentSize;

	pBlockInfo->__pContents = pContents;
	pBlockInfo->__uSize     = uContentSize;
	pBlockInfo->__pRetAddr  = pRetAddr;
	MCF_AvlAttach(&g_pavlBlocks, (MCF_AvlNodeHeader *)pBlockInfo, &BlockInfoComparatorNodes);

	void **ppGuard1 = (void **)pContents;
	void **ppGuard2 = (void **)(pContents + uSize);
	for(unsigned i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *const pTemp1 = EncodePointer(ppGuard2), *const pTemp2 = EncodePointer(ppGuard1);
		__builtin_memcpy(ppGuard1, &pTemp1, sizeof(void *));
		__builtin_memcpy(ppGuard2, &pTemp2, sizeof(void *));

		++ppGuard2;
	}

	return pContents;
}
__MCF_HeapDbgBlockInfo *__MCF_CRT_HeapDbgValidateBlock(unsigned char **ppRaw, unsigned char *pContents, const void *pRetAddr){
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	__MCF_HeapDbgBlockInfo *const pBlockInfo = (__MCF_HeapDbgBlockInfo *)MCF_AvlFind(&g_pavlBlocks, (intptr_t)pContents, &BlockInfoComparatorNodeKey);
	if(!pBlockInfo){
		MCF_CRT_BailF(L"__MCF_CRT_HeapDbgValidate() 失败：传入的指针无效。\n调用返回地址：%p", pRetAddr);
	}
	const size_t uSize = pBlockInfo->__uSize;

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + uSize);
	for(unsigned i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *pTemp1, *pTemp2;
		__builtin_memcpy(&pTemp1, ppGuard1, sizeof(void *));
		__builtin_memcpy(&pTemp2, ppGuard2, sizeof(void *));
		if((DecodePointer(pTemp1) != ppGuard2) || (DecodePointer(pTemp2) != ppGuard1)){
			MCF_CRT_BailF(L"__MCF_CRT_HeapDbgValidate() 失败：侦测到堆损坏。\n调用返回地址：%p", pRetAddr);
		}

		++ppGuard2;
	}

	return pBlockInfo;
}
void __MCF_CRT_HeapDbgUnregisterBlockInfo(__MCF_HeapDbgBlockInfo *pBlockInfo){
	MCF_AvlDetach((const MCF_AvlNodeHeader *)pBlockInfo);
}

#	else

unsigned char *__MCF_CRT_HeapDbgAddBlockGuardsBasic(unsigned char *pRaw){
	unsigned char *const pContents = pRaw + GUARD_BAND_SIZE;
	const size_t uSize = __MCF_CRT_ReallyGetUsableSize(pRaw) - GUARD_BAND_SIZE * 2;

	void **ppGuard1 = (void **)pContents;
	void **ppGuard2 = (void **)(pContents + uSize);
	for(unsigned i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *const pTemp1 = EncodePointer(ppGuard2), *const pTemp2 = EncodePointer(ppGuard1);
		__builtin_memcpy(ppGuard1, &pTemp1, sizeof(void *));
		__builtin_memcpy(ppGuard2, &pTemp2, sizeof(void *));

		++ppGuard2;
	}

	return pContents;
}
void __MCF_CRT_HeapDbgValidateBlockBasic(unsigned char **ppRaw, unsigned char *pContents, const void *pRetAddr){
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	const size_t uSize = __MCF_CRT_ReallyGetUsableSize(pRaw) - GUARD_BAND_SIZE * 2;

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + uSize);
	for(unsigned i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *pTemp1, *pTemp2;
		__builtin_memcpy(&pTemp1, ppGuard1, sizeof(void *));
		__builtin_memcpy(&pTemp2, ppGuard2, sizeof(void *));
		if((DecodePointer(pTemp1) != ppGuard2) || (DecodePointer(pTemp2) != ppGuard1)){
			MCF_CRT_BailF(L"__MCF_CRT_HeapDbgValidate() 失败：侦测到堆损坏。\n调用返回地址：%p", pRetAddr);
		}

		++ppGuard2;
	}
}

#	endif

#endif
