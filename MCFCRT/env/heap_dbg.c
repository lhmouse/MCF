// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "heap_dbg.h"
#include "bail.h"

#ifdef __MCF_CRT_HEAPDBG_ON

#include <wchar.h>
#include <windows.h>

#define GUARD_BAND_SIZE		0x20ul

static HANDLE				g_hMapAllocator;
static __MCF_AVL_PROOT		g_pMapRoot;

unsigned long __MCF_CRT_HeapDbgInitContext(){
	g_hMapAllocator = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	if(g_hMapAllocator == NULL){
		return GetLastError();
	}

	g_pMapRoot = NULL;

	return ERROR_SUCCESS;
}
void __MCF_CRT_HeapDbgUninitContext(){
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo = (const __MCF_HEAPDBG_BLOCK_INFO *)__MCF_AVLBegin(g_pMapRoot);
	if(pBlockInfo != NULL){
		__MCF_Bail(
			L"__MCF_CRT_HeapDbgUninitContext() 失败：侦测到内存泄漏。\n\n"
			"如果您选择调试应用程序，MCF CRT 将尝试使用 OutputDebugString() 导出内存泄漏的详细信息。"
		);

		do {
			wchar_t awchBuffer[256];

			const BYTE *pbyDump = __MCF_CRT_HeapDbgGetContents(pBlockInfo);
			wchar_t *pWrite = awchBuffer + wsprintfW(
				awchBuffer,
				L"地址 0x%p  大小 0x%p  调用返回地址 0x%p  首字节",
				(void *)pbyDump,
				(void *)pBlockInfo->uSize,
				pBlockInfo->pRetAddr
			);
			for(size_t i = 0; i < 16; ++i){
				*(pWrite++) = L' ';
				if(IsBadReadPtr(pbyDump, 1)){
					*(pWrite++) = L'?';
					*(pWrite++) = L'?';
				} else {
					const unsigned int uHi = (*pbyDump) >> 4;
					const unsigned int uLo = (*pbyDump) & 0x0F;

					*(pWrite++) = L'0' + uHi + ((uHi >= 0x0A) ? (L'A' - L'0' - 0x0A) : 0);
					*(pWrite++) = L'0' + uLo + ((uLo >= 0x0A) ? (L'A' - L'0' - 0x0A) : 0);
				}
				++pbyDump;
			}
			*pWrite = 0;

			OutputDebugStringW(awchBuffer);
		} while((pBlockInfo = (const __MCF_HEAPDBG_BLOCK_INFO *)__MCF_AVLNext((const __MCF_AVL_NODE_HEADER *)pBlockInfo)) != NULL);
		__asm__ __volatile__("int 3 \n");
	}

	HeapDestroy(g_hMapAllocator);
}

size_t __MCF_CRT_HeapDbgGetRawSize(
	size_t uContentSize
){
	return uContentSize + GUARD_BAND_SIZE * 2;
}
void __MCF_CRT_HeapDbgAddGuardsAndRegister(
	unsigned char **ppContents,
	unsigned char *pRaw,
	size_t uContentSize,
	const void *pRetAddr
){
	unsigned char *const pContents = pRaw + GUARD_BAND_SIZE;
	*ppContents = pContents;

	void **ppGuard1 = (void **)pContents;
	void **ppGuard2 = (void **)(pContents + uContentSize);
	for(unsigned int i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		*ppGuard1 = EncodePointer(ppGuard2);
		*ppGuard2 = EncodePointer(ppGuard1);

		++ppGuard2;
	}

	__MCF_HEAPDBG_BLOCK_INFO *const pBlockInfo = (__MCF_HEAPDBG_BLOCK_INFO *)HeapAlloc(g_hMapAllocator, 0, sizeof(__MCF_HEAPDBG_BLOCK_INFO));
	if(pBlockInfo == NULL){
		__MCF_BailF(L"__MCF_CRT_HeapDbgAddGuardsAndRegister() 失败：内存不足。\n调用返回地址：%p", pRetAddr);
	}
	if(__MCF_AVLAttach(&g_pMapRoot, (intptr_t)pContents, (__MCF_AVL_NODE_HEADER *)pBlockInfo) != NULL){
		__MCF_BailF(L"__MCF_CRT_HeapDbgAddGuardsAndRegister() 失败：传入的指针的记录已存在。\n调用返回地址：%p", pRetAddr);
	}

	pBlockInfo->uSize = uContentSize;
	pBlockInfo->pRetAddr = pRetAddr;
}
const __MCF_HEAPDBG_BLOCK_INFO *__MCF_CRT_HeapDbgValidate(
	unsigned char **ppRaw,
	unsigned char *pContents,
	const void *pRetAddr
){
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo = (const __MCF_HEAPDBG_BLOCK_INFO *)__MCF_AVLFind(g_pMapRoot, (intptr_t)pContents);
	if(pBlockInfo == NULL){
		__MCF_BailF(L"__MCF_CRT_HeapDbgValidate() 失败：传入的指针无效。\n调用返回地址：%p", pRetAddr);
	}

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + pBlockInfo->uSize);
	for(unsigned int i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		if((DecodePointer(*ppGuard1) != ppGuard2) || (DecodePointer(*ppGuard2) != ppGuard1)){
			__MCF_BailF(L"__MCF_CRT_HeapDbgValidate() 失败：侦测到堆损坏。\n调用返回地址：%p", pRetAddr);
		}

		++ppGuard2;
	}

	return pBlockInfo;
}
const unsigned char *__MCF_CRT_HeapDbgGetContents(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
){
	return (const unsigned char *)pBlockInfo->__MCF_AVLNodeHeader.nKey;
}
void __MCF_CRT_HeapDbgUnregister(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
){
	__MCF_AVLDetach((const __MCF_AVL_NODE_HEADER *)pBlockInfo);

	HeapFree(g_hMapAllocator, 0, (void *)pBlockInfo);
}

#endif
