// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "heap_dbg.h"
#include "../ext/stpcpy.h"
#include "../ext/unref_param.h"
#include "bail.h"
#include "mcfwin.h"
#include <wchar.h>

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)

#define GUARD_BAND_SIZE	0x20u

static HANDLE		g_hMapAllocator;
static MCF_AvlRoot	g_pavlBlocks;

static int BlockInfoComparatorNodeKey(const MCF_AvlNodeHeader *pInfo1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(((const __MCF_HeapDbgBlockInfo *)pInfo1)->pContents);
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int BlockInfoComparatorNodes(const MCF_AvlNodeHeader *pInfo1, const MCF_AvlNodeHeader *pInfo2){
	return BlockInfoComparatorNodeKey(pInfo1, (intptr_t)(((const __MCF_HeapDbgBlockInfo *)pInfo2)->pContents));
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
		MCF_CRT_Bail(L"__MCF_CRT_HeapDbgUninit() 失败：侦测到内存泄漏。\n\n"
			"如果您选择调试应用程序，MCF CRT 将尝试使用 OutputDebugString() 导出内存泄漏的详细信息。");

		const __MCF_HeapDbgBlockInfo *pBlockInfo = (const __MCF_HeapDbgBlockInfo *)MCF_AvlFront(&g_pavlBlocks);
		wchar_t awcBuffer[256];
		do {
			const unsigned char *pbyDump = pBlockInfo->pContents;
			wchar_t *pwcWrite = awcBuffer + __mingw_snwprintf(
				awcBuffer, sizeof(awcBuffer) / sizeof(wchar_t),
				L"地址 %0*zX  大小 %0*zX  调用返回地址 %0*zX  首字节 ",
				(int)(sizeof(size_t) * 2), (size_t)pbyDump,
				(int)(sizeof(size_t) * 2), (pBlockInfo->uSize),
				(int)(sizeof(size_t) * 2), (size_t)(pBlockInfo->pRetAddr));
			for(size_t i = 0; i < 16; ++i){
				*(pwcWrite++) = L' ';
				if(IsBadReadPtr(pbyDump, 1)){
					*(pwcWrite++) = L'?';
					*(pwcWrite++) = L'?';
				} else {
					static const wchar_t HEX_TABLE[16] = L"0123456789ABCDEF";
					*(pwcWrite++) = HEX_TABLE[*pbyDump >> 4];
					*(pwcWrite++) = HEX_TABLE[*pbyDump & 0x0F];
				}
				++pbyDump;
			}
			*pwcWrite = 0;

			OutputDebugStringW(awcBuffer);

			pBlockInfo = (const __MCF_HeapDbgBlockInfo *)MCF_AvlNext((const MCF_AvlNodeHeader *)pBlockInfo);
		} while(pBlockInfo);

		__asm__ __volatile__("int3 \n");
	}

	g_pavlBlocks = nullptr;
	HeapDestroy(g_hMapAllocator);
	g_hMapAllocator = nullptr;
#endif
}

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)

size_t __MCF_CRT_HeapDbgGetRawSize(size_t uContentSize){
	return uContentSize + GUARD_BAND_SIZE * 2;
}
unsigned char *__MCF_CRT_HeapDbgAddGuardsAndRegister(
	unsigned char *pRaw, size_t uContentSize, const void *pRetAddr)
{
	unsigned char *const pContents = pRaw + GUARD_BAND_SIZE;

	void **ppGuard1 = (void **)pContents;
	void **ppGuard2 = (void **)(pContents + uContentSize);
	for(unsigned i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *const pTemp1 = EncodePointer(ppGuard2), *const pTemp2 = EncodePointer(ppGuard1);
		__builtin_memcpy(ppGuard1, &pTemp1, sizeof(void *));
		__builtin_memcpy(ppGuard2, &pTemp2, sizeof(void *));

		++ppGuard2;
	}

	__MCF_HeapDbgBlockInfo *const pBlockInfo = HeapAlloc(g_hMapAllocator, 0, sizeof(__MCF_HeapDbgBlockInfo));
	if(!pBlockInfo){
		MCF_CRT_BailF(L"__MCF_CRT_HeapDbgAddGuardsAndRegister() 失败：内存不足。\n调用返回地址：%0*zX",
			(int)(sizeof(size_t) * 2), (size_t)pRetAddr);
	}
	pBlockInfo->pContents	= pContents;
	pBlockInfo->uSize		= uContentSize;
	pBlockInfo->pRetAddr	= pRetAddr;
	MCF_AvlAttach(&g_pavlBlocks, (MCF_AvlNodeHeader *)pBlockInfo, &BlockInfoComparatorNodes);

	return pContents;
}
const __MCF_HeapDbgBlockInfo *__MCF_CRT_HeapDbgValidate(
	unsigned char **ppRaw, unsigned char *pContents, const void *pRetAddr)
{
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	const __MCF_HeapDbgBlockInfo *const pBlockInfo = (const __MCF_HeapDbgBlockInfo *)MCF_AvlFind(
		&g_pavlBlocks, (intptr_t)pContents, &BlockInfoComparatorNodeKey);
	if(!pBlockInfo){
		MCF_CRT_BailF(L"__MCF_CRT_HeapDbgValidate() 失败：传入的指针无效。\n调用返回地址：%0*zX",
			(int)(sizeof(size_t) * 2), (size_t)pRetAddr);
	}

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + pBlockInfo->uSize);
	for(unsigned i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		void *pTemp1, *pTemp2;
		__builtin_memcpy(&pTemp1, ppGuard1, sizeof(void *));
		__builtin_memcpy(&pTemp2, ppGuard2, sizeof(void *));
		if((DecodePointer(pTemp1) != ppGuard2) || (DecodePointer(pTemp2) != ppGuard1)){
			MCF_CRT_BailF(L"__MCF_CRT_HeapDbgValidate() 失败：侦测到堆损坏。\n调用返回地址：%0*zX",
				(int)(sizeof(size_t) * 2), (size_t)pRetAddr);
		}

		++ppGuard2;
	}

	return pBlockInfo;
}
void __MCF_CRT_HeapDbgUnregister(const __MCF_HeapDbgBlockInfo *pBlockInfo){
	MCF_AvlDetach((const MCF_AvlNodeHeader *)pBlockInfo);
	HeapFree(g_hMapAllocator, 0, (void *)pBlockInfo);
}

#endif
