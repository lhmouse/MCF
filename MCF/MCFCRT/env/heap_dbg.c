// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "heap_dbg.h"

#ifdef __MCF_CRT_HEAPDBG_ON

#include "daemon.h"
#include <windows.h>

#define GUARD_BAND_SIZE		0x10ul

static HANDLE		g_hAllocator;
static AVL_ROOT		g_pRoot;

__MCF_CRT_EXTERN void __MCF_CRTHeapDbgInitContext(){
	g_hAllocator = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	if(g_hAllocator == NULL){
		__MCF_Bail(L"__MCF_CRTHeapDbgInitContext() 失败：CreateHeap() 失败。");
	}

	g_pRoot = NULL;
}
__MCF_CRT_EXTERN void __MCF_CRTHeapDbgUninitContext(){
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo = (const __MCF_HEAPDBG_BLOCK_INFO *)AVLBegin(g_pRoot);
	if(pBlockInfo != NULL){
		__MCF_Bail(
			L"__MCF_CRTHeapDbgUninitContext() 失败：侦测到内存泄漏。\n\n"
			"如果您选择调试应用程序，MCF CRT 将尝试使用 OutputDebugString() 导出内存泄漏的详细信息。"
		);

		do {
			wchar_t awchBuffer[1025];

			const BYTE *pbyDump = __MCF_CRTHeapDbgGetContents(pBlockInfo);
			wchar_t *pWrite = awchBuffer + wsprintfW(
				awchBuffer,
				L"地址 0x%p  大小 0x%08X  首字节 ",
				(const void *)pbyDump,
				pBlockInfo->uSize
			);
			for(size_t i = 0; i < 16; ++i){
				if(IsBadReadPtr(pbyDump, 1)){
					*(pWrite++) = L'?';
					*(pWrite++) = L'?';
					*(pWrite++) = L' ';
				} else {
					const unsigned int uHi = (*pbyDump) >> 4;
					const unsigned int uLo = (*pbyDump) & 0x0F;

					*(pWrite++) = L'0' + uHi + ((uHi >= 0x0A) ? (L'A' - L'0' - 0x0A) : 0);
					*(pWrite++) = L'0' + uLo + ((uLo >= 0x0A) ? (L'A' - L'0' - 0x0A) : 0);
					*(pWrite++) = L' ';
				}
				++pbyDump;
			}
			*pWrite = 0;

			OutputDebugStringW(awchBuffer);
		} while((pBlockInfo = (const __MCF_HEAPDBG_BLOCK_INFO *)AVLNext((const AVL_NODE_HEADER *)pBlockInfo)) != NULL);
		__asm__ __volatile__("int 3 \n");
	}

	HeapDestroy(g_hAllocator);
}

__MCF_CRT_EXTERN size_t __MCF_CRTHeapDbgGetRawSize(
	size_t uContentSize
){
	return uContentSize + GUARD_BAND_SIZE * 2;
}
__MCF_CRT_EXTERN void __MCF_CRTHeapDbgAddGuardsAndRegister(
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

	__MCF_HEAPDBG_BLOCK_INFO *const pBlockInfo = (__MCF_HEAPDBG_BLOCK_INFO *)HeapAlloc(g_hAllocator, 0, sizeof(__MCF_HEAPDBG_BLOCK_INFO));
	if(pBlockInfo == NULL){
		__MCF_BailF(L"__MCF_CRTHeapDbgAddGuardsAndRegister() 失败：内存不足。\n调用返回地址：%p", pRetAddr);
	}
	if(!AVLAttach(&g_pRoot, (intptr_t)pContents, (AVL_NODE_HEADER *)pBlockInfo)){
		__MCF_BailF(L"__MCF_CRTHeapDbgAddGuardsAndRegister() 失败：传入的指针的记录已存在。\n调用返回地址：%p", pRetAddr);
	}

	pBlockInfo->uSize = uContentSize;
	pBlockInfo->pRetAddr = pRetAddr;
}
__MCF_CRT_EXTERN const __MCF_HEAPDBG_BLOCK_INFO *__MCF_CRTHeapDbgValidate(
	unsigned char **ppRaw,
	unsigned char *pContents,
	const void *pRetAddr
){
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo = (const __MCF_HEAPDBG_BLOCK_INFO *)AVLFind(g_pRoot, (intptr_t)pContents);
	if(pBlockInfo == NULL){
		__MCF_BailF(L"__MCF_CRTHeapDbgValidate() 失败：传入的指针无效。\n调用返回地址：%p", pRetAddr);
	}

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + pBlockInfo->uSize);
	for(unsigned int i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		if((DecodePointer(*ppGuard1) != ppGuard2) || (DecodePointer(*ppGuard2) != ppGuard1)){
			__MCF_BailF(L"__MCF_CRTHeapDbgValidate() 失败：侦测到堆损坏。\n调用返回地址：%p", pRetAddr);
		}

		++ppGuard2;
	}

	return pBlockInfo;
}
__MCF_CRT_EXTERN const unsigned char *__MCF_CRTHeapDbgGetContents(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
){
	return (const unsigned char *)pBlockInfo->AVLNodeHeader.nKey;
}
__MCF_CRT_EXTERN void __MCF_CRTHeapDbgUnregister(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
){
	AVLDetach((const AVL_NODE_HEADER *)pBlockInfo);

	HeapFree(g_hAllocator, 0, (void *)pBlockInfo);
}

#endif
