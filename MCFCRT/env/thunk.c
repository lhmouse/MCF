// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "thunk.h"
#include "mcfwin.h"
#include "avl_tree.h"
#include "bail.h"
#include "../ext/assert.h"
#include <stdlib.h>

typedef struct tagThunkInfo {
	// 内存是以 64KiB 的粒度分配的，每一块称为一个 chunk。
	void *pChunk;
	size_t uChunkSize;

	MCF_AvlNodeHeader vThunkIndex;
	void *pThunk;
	size_t uThunkSize;

	MCF_AvlNodeHeader vFreeSizeIndex;
	size_t uFreeSize;
} ThunkInfo;

static inline ThunkInfo *GetInfoFromThunkIndex(const MCF_AvlNodeHeader *pThunkIndex){
	ASSERT(pThunkIndex);

	return (ThunkInfo *)((char *)pThunkIndex - offsetof(ThunkInfo, vThunkIndex));
}
static inline ThunkInfo *GetInfoFromFreeSizeIndex(const MCF_AvlNodeHeader *pFreeSizeIndex){
	ASSERT(pFreeSizeIndex);

	return (ThunkInfo *)((char *)pFreeSizeIndex - offsetof(ThunkInfo, vFreeSizeIndex));
}

static int ThunkComparatorNodeKey(const MCF_AvlNodeHeader *pIndex1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(GetInfoFromThunkIndex(pIndex1)->pThunk);
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int ThunkComparatorNodes(const MCF_AvlNodeHeader *pIndex1, const MCF_AvlNodeHeader *pIndex2){
	return ThunkComparatorNodeKey(pIndex1, (intptr_t)(GetInfoFromThunkIndex(pIndex2)->pThunk));
}

_Static_assert(sizeof(size_t) <= sizeof(uintptr_t), "This platform is not supported.");

static int FreeSizeComparatorNodeKey(const MCF_AvlNodeHeader *pIndex1, intptr_t nKey2){
	const uintptr_t uKey1 = GetInfoFromFreeSizeIndex(pIndex1)->uFreeSize;
	const uintptr_t uKey2 = (uintptr_t)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int FreeSizeComparatorNodes(const MCF_AvlNodeHeader *pIndex1, const MCF_AvlNodeHeader *pIndex2){
	return FreeSizeComparatorNodeKey(pIndex1, (intptr_t)GetInfoFromFreeSizeIndex(pIndex2)->uFreeSize);
}

static SRWLOCK     g_srwlMutex            = SRWLOCK_INIT;

static uintptr_t   g_uPageMask            = 0;
static ThunkInfo * g_pSpare               = nullptr;
static bool        g_bCleanupRegistered   = false;

static MCF_AvlRoot g_pavlThunksByThunk    = nullptr;
static MCF_AvlRoot g_pavlThunksByFreeSize = nullptr;

static void BackupCleanup(){
	free(g_pSpare);
	g_pSpare = nullptr;
}

void *MCF_CRT_AllocateThunk(const void *pInit, size_t uSize){
	ASSERT(pInit);

	unsigned char *pRet = nullptr;

	AcquireSRWLockExclusive(&g_srwlMutex);
	{
		if(g_uPageMask == 0){
			SYSTEM_INFO vSystemInfo;
			GetSystemInfo(&vSystemInfo);
			g_uPageMask = vSystemInfo.dwPageSize - 1;
		}
		if(!g_bCleanupRegistered){
			if(atexit(&BackupCleanup)){
				goto jDone;
			}
			g_bCleanupRegistered = true;
		}

		if(!g_pSpare){
			g_pSpare = malloc(sizeof(ThunkInfo));
			if(!g_pSpare){
				goto jDone;
			}
		}

		size_t uThunkSize = uSize + 8;
		uThunkSize = (uThunkSize + 0x0F) & (size_t)-0x10;
		if(uThunkSize < uSize){
			goto jDone;
		}

		MCF_AvlNodeHeader *pFreeSizeIndex = MCF_AvlLowerBound(&g_pavlThunksByFreeSize,
			(intptr_t)uThunkSize, &FreeSizeComparatorNodeKey);
		ThunkInfo *pInfo;
		if(pFreeSizeIndex){
			pInfo = GetInfoFromFreeSizeIndex(pFreeSizeIndex);
		} else {
			// 如果没有足够大的 thunk，我们先分配一个新的 chunk。
			pInfo = malloc(sizeof(ThunkInfo));
			if(!pInfo){
				goto jDone;
			}
			pInfo->uChunkSize = (uThunkSize + 0xFFFF) & (size_t)-0x10000;
			pInfo->pChunk = VirtualAlloc(0, pInfo->uChunkSize, MEM_COMMIT | MEM_RESERVE, PAGE_READONLY);
			if(!pInfo->pChunk){
				free(pInfo);
				goto jDone;
			}
			pInfo->pThunk = pInfo->pChunk;
			pInfo->uThunkSize = pInfo->uChunkSize;
			pInfo->uFreeSize = pInfo->uChunkSize;

			MCF_AvlAttach(&g_pavlThunksByThunk, &(pInfo->vThunkIndex), &ThunkComparatorNodes);
			MCF_AvlAttach(&g_pavlThunksByFreeSize, &(pInfo->vFreeSizeIndex), &FreeSizeComparatorNodes);

			pFreeSizeIndex = &(pInfo->vFreeSizeIndex);
		}
		ASSERT(pInfo->uFreeSize >= uThunkSize);

		pRet = pInfo->pThunk;
		const size_t uRemaining = pInfo->uFreeSize - uThunkSize;
		if(uRemaining >= 0x20){
			// 如果剩下的空间还很大，保存成一个新的空闲 thunk。
			g_pSpare->pChunk     = pInfo->pChunk;
			g_pSpare->uChunkSize = pInfo->uChunkSize;
			g_pSpare->pThunk     = pRet + uThunkSize;
			g_pSpare->uThunkSize = uRemaining;
			g_pSpare->uFreeSize  = uRemaining;

			MCF_AvlAttach(&g_pavlThunksByThunk, &(g_pSpare->vThunkIndex), &ThunkComparatorNodes);
			MCF_AvlAttach(&g_pavlThunksByFreeSize, &(g_pSpare->vFreeSizeIndex), &FreeSizeComparatorNodes);

			g_pSpare = nullptr;

			pInfo->uThunkSize = uThunkSize;
		} else {
			uThunkSize = pInfo->uThunkSize;
		}

		MCF_AvlDetach(&(pInfo->vFreeSizeIndex));
		pInfo->uFreeSize = 0;
		MCF_AvlAttach(&g_pavlThunksByFreeSize, &(pInfo->vFreeSizeIndex), &FreeSizeComparatorNodes);

		// 由于其他 thunk 可能共享了当前内存页，所以不能设置为 PAGE_READWRITE。
		DWORD dwOldProtect;
		VirtualProtect(pRet, uThunkSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		memcpy(pRet, pInit, uSize);
		memset(pRet + uSize, 0xCC, uThunkSize - uSize);
		VirtualProtect(pRet, uThunkSize, PAGE_EXECUTE_READ, &dwOldProtect);
	}
jDone:
	ReleaseSRWLockExclusive(&g_srwlMutex);

	if(!pRet){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	}
	return pRet;
}
void MCF_CRT_DeallocateThunk(void *pThunk, bool bToPoison){
	void *pPageToRelease;

	AcquireSRWLockExclusive(&g_srwlMutex);
	{
		MCF_AvlNodeHeader *pThunkIndex = MCF_AvlFind(&g_pavlThunksByThunk, (intptr_t)pThunk, &ThunkComparatorNodeKey);
		ThunkInfo *pInfo;
		if(!pThunkIndex || ((pInfo = GetInfoFromThunkIndex(pThunkIndex))->uFreeSize != 0)){
			MCF_CRT_Bail(L"MCF_CRT_DeallocateThunk() 失败：传入的指针无效。");
		}

#ifndef NDEBUG
		bToPoison = true;
#endif
		if(bToPoison){
			// 由于其他 thunk 可能共享了当前内存页，所以不能设置为 PAGE_READWRITE。
			DWORD dwOldProtect;
			VirtualProtect(pThunk, pInfo->uThunkSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			memset(pThunk, 0xCC, pInfo->uThunkSize);
			VirtualProtect(pThunk, pInfo->uThunkSize, PAGE_EXECUTE_READ, &dwOldProtect);
		}

		MCF_AvlNodeHeader *const pNextThunkIndex = MCF_AvlNext(pThunkIndex);
		if(pNextThunkIndex){
			// 如果连续的下一个 thunk 也处于空闲状态，把它合并到当前 thunk。
			ThunkInfo *const pNextInfo = GetInfoFromThunkIndex(pNextThunkIndex);
			if((pNextInfo->pChunk == pInfo->pChunk) && (pNextInfo->uFreeSize != 0)){
				pInfo->uThunkSize += pNextInfo->uThunkSize;

				MCF_AvlDetach(&(pNextInfo->vThunkIndex));
				MCF_AvlDetach(&(pNextInfo->vFreeSizeIndex));
				free(pNextInfo);
			}
		}

		MCF_AvlNodeHeader *const pPrevThunkIndex = MCF_AvlPrev(pThunkIndex);
		if(pPrevThunkIndex){
			// 如果连续的上一个 thunk 也处于空闲状态，把它合并到当前 thunk。
			ThunkInfo *const pPrevInfo = GetInfoFromThunkIndex(pPrevThunkIndex);
			if((pPrevInfo->pChunk == pInfo->pChunk) && (pPrevInfo->uFreeSize != 0)){
				pPrevInfo->uThunkSize += pInfo->uThunkSize;

				MCF_AvlDetach(&(pInfo->vThunkIndex));
				MCF_AvlDetach(&(pInfo->vFreeSizeIndex));
				free(pInfo);

				pThunkIndex = pPrevThunkIndex;
				pInfo = pPrevInfo;
			}
		}

		if((pInfo->pThunk == pInfo->pChunk) && (pInfo->uThunkSize == pInfo->uChunkSize)){
			pPageToRelease = pInfo->pChunk;

			MCF_AvlDetach(&(pInfo->vThunkIndex));
			MCF_AvlDetach(&(pInfo->vFreeSizeIndex));
			free(pInfo);
		} else {
			pPageToRelease = nullptr;

			MCF_AvlDetach(&(pInfo->vFreeSizeIndex));
			pInfo->uFreeSize = pInfo->uThunkSize;
			MCF_AvlAttach(&g_pavlThunksByFreeSize, &(pInfo->vFreeSizeIndex), &FreeSizeComparatorNodes);
		}
	}
	ReleaseSRWLockExclusive(&g_srwlMutex);

	if(pPageToRelease){
		VirtualFree(pPageToRelease, 0, MEM_RELEASE);
	}
}
