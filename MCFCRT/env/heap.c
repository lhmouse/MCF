// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "heap.h"
#include "heap_dbg.h"
#include <string.h>
#include <windows.h>

static CRITICAL_SECTION			g_csHeapLock;
static __MCF_BAD_ALLOC_HANDLER	g_vBadAllocHandler;

unsigned long __MCF_CRT_HeapInitialize(){
	InitializeCriticalSectionAndSpinCount(&g_csHeapLock, 0x400);
	g_vBadAllocHandler.pfnProc = NULL;

#ifdef __MCF_CRT_HEAPDBG_ON
	__MCF_CRT_HeapDbgInit();
#endif

	const HANDLE hProcessHeap = GetProcessHeap();

	ULONG ulLFHFlag = 2;
	HeapSetInformation(hProcessHeap, HeapCompatibilityInformation, &ulLFHFlag, sizeof(ulLFHFlag));

#if WINVER >= 0x0600
	if(LOBYTE(LOWORD(GetVersion())) >= 6){
		HeapSetInformation(hProcessHeap, HeapEnableTerminationOnCorruption, NULL, 0);
	}
#endif

	return ERROR_SUCCESS;
}
void __MCF_CRT_HeapUninitialize(){
#ifdef __MCF_CRT_HEAPDBG_ON
	__MCF_CRT_HeapDbgUninit();
#endif

	DeleteCriticalSection(&g_csHeapLock);
}

unsigned char *__MCF_CRT_HeapAlloc(size_t uSize, const void *pRetAddr __attribute__((unused))){
#ifdef __MCF_CRT_HEAPDBG_ON
	const size_t uRawSize = __MCF_CRT_HeapDbgGetRawSize(uSize);
	if(uSize & ~uRawSize & ((size_t)1 << (sizeof(size_t) * 8 - 1))){
		return NULL;
	}
#else
	const size_t uRawSize = uSize;
#endif

	unsigned char *pRet = NULL;
	EnterCriticalSection(&g_csHeapLock);
		do {
			unsigned char *const pRaw = (unsigned char *)HeapAlloc(GetProcessHeap(), 0, uRawSize);
			if(pRaw){
#ifdef __MCF_CRT_HEAPDBG_ON
				__MCF_CRT_HeapDbgAddGuardsAndRegister(&pRet, pRaw, uSize, pRetAddr);
				memset(pRet, 0xCD, uSize);
#else
				pRet = pRaw;
#endif
				break;
			}
		} while((g_vBadAllocHandler.pfnProc) && (*g_vBadAllocHandler.pfnProc)(g_vBadAllocHandler.nContext));
	LeaveCriticalSection(&g_csHeapLock);
	return pRet;
}
unsigned char *__MCF_CRT_HeapReAlloc(unsigned char *pBlock /* NON-NULL */, size_t uSize, const void *pRetAddr __attribute__((unused))){
#ifdef __MCF_CRT_HEAPDBG_ON
	const size_t uRawSize = __MCF_CRT_HeapDbgGetRawSize(uSize);
	if(uSize & ~uRawSize & ((size_t)1 << (sizeof(size_t) * 8 - 1))){
		return NULL;
	}
#else
	const size_t uRawSize = uSize;
#endif

	unsigned char *pRet = NULL;
	EnterCriticalSection(&g_csHeapLock);
#ifdef __MCF_CRT_HEAPDBG_ON
		unsigned char *pRawOriginal;
		const __MCF_HEAPDBG_BLOCK_INFO *const pBlockInfo = __MCF_CRT_HeapDbgValidate(&pRawOriginal, pBlock, pRetAddr);
#else
		unsigned char *const pRawOriginal = pBlock;
#endif

		do {
			unsigned char *const pRaw = (unsigned char *)HeapReAlloc(GetProcessHeap(), 0, pRawOriginal, uRawSize);
			if(pRaw){
#ifdef __MCF_CRT_HEAPDBG_ON
				const size_t uOriginalSize = pBlockInfo->uSize;
				__MCF_CRT_HeapDbgUnregister(pBlockInfo);

				__MCF_CRT_HeapDbgAddGuardsAndRegister(&pRet, pRaw, uSize, pRetAddr);
				if(uOriginalSize < uSize){
					memset(pRet + uOriginalSize, 0xCD, uSize - uOriginalSize);
				}
#else
				pRet = pRaw;
#endif
				break;
			}
		} while((g_vBadAllocHandler.pfnProc) && (*g_vBadAllocHandler.pfnProc)(g_vBadAllocHandler.nContext));
	LeaveCriticalSection(&g_csHeapLock);
	return pRet;
}
void __MCF_CRT_HeapFree(unsigned char *pBlock /* NON-NULL */, const void *pRetAddr __attribute__((unused))){
	EnterCriticalSection(&g_csHeapLock);
#ifdef __MCF_CRT_HEAPDBG_ON
		unsigned char *pRaw;
		const __MCF_HEAPDBG_BLOCK_INFO *const pBlockInfo = __MCF_CRT_HeapDbgValidate(&pRaw, pBlock, pRetAddr);

		memset(pBlock, 0xFE, pBlockInfo->uSize);

		__MCF_CRT_HeapDbgUnregister(pBlockInfo);
#else
		unsigned char *const pRaw = pBlock;
#endif
		HeapFree(GetProcessHeap(), 0, pRaw);
	LeaveCriticalSection(&g_csHeapLock);
}

// Q: 这里为什么不用原子操作？
// A: 如果使用原子操作，线程 A 就可能在线程 B 正在执行内存分配/去配操作的时候更改响应函数。
//    如果线程 B 在线程 A 更改响应函数之后调用了旧的响应函数，结果将是不可预测的。
__MCF_BAD_ALLOC_HANDLER __MCF_GetBadAllocHandler(){
	EnterCriticalSection(&g_csHeapLock);
		const __MCF_BAD_ALLOC_HANDLER Handler = g_vBadAllocHandler;
	LeaveCriticalSection(&g_csHeapLock);
	return Handler;
}
__MCF_BAD_ALLOC_HANDLER __MCF_SetBadAllocHandler(__MCF_BAD_ALLOC_HANDLER NewHandler){
	EnterCriticalSection(&g_csHeapLock);
		const __MCF_BAD_ALLOC_HANDLER OldHandler = g_vBadAllocHandler;
		g_vBadAllocHandler = NewHandler;
	LeaveCriticalSection(&g_csHeapLock);
	return OldHandler;
}
