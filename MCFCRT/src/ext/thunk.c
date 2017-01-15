// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "thunk.h"
#include "../env/mcfwin.h"
#include "../env/avl_tree.h"
#include "../env/mutex.h"
#include "../env/bail.h"
#include "../env/heap.h"
#include "assert.h"

typedef struct tagThunkInfo {
	// 内存是以 64KiB 的粒度分配的，每一块称为一个 chunk。
	void *pChunk;
	size_t uChunkSize;

	_MCFCRT_AvlNodeHeader vThunkIndex;
	void *pThunk;
	size_t uThunkSize;

	_MCFCRT_AvlNodeHeader vFreeSizeIndex;
	size_t uFreeSize;
} ThunkInfo;

static inline ThunkInfo *GetInfoFromThunkIndex(const _MCFCRT_AvlNodeHeader *pThunkIndex){
	_MCFCRT_ASSERT(pThunkIndex);

	return (ThunkInfo *)((char *)pThunkIndex - offsetof(ThunkInfo, vThunkIndex));
}
static inline ThunkInfo *GetInfoFromFreeSizeIndex(const _MCFCRT_AvlNodeHeader *pFreeSizeIndex){
	_MCFCRT_ASSERT(pFreeSizeIndex);

	return (ThunkInfo *)((char *)pFreeSizeIndex - offsetof(ThunkInfo, vFreeSizeIndex));
}

static int ThunkComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pIndex1, intptr_t nKey2){
	const uintptr_t u1 = (uintptr_t)(GetInfoFromThunkIndex(pIndex1)->pThunk);
	const uintptr_t u2 = (uintptr_t)(void *)nKey2;
	if(u1 != u2){
		return (u1 < u2) ? -1 : 1;
	}
	return 0;
}
static int ThunkComparatorNodes(const _MCFCRT_AvlNodeHeader *pIndex1, const _MCFCRT_AvlNodeHeader *pIndex2){
	return ThunkComparatorNodeKey(pIndex1, (intptr_t)(GetInfoFromThunkIndex(pIndex2)->pThunk));
}

static_assert(sizeof(size_t) <= sizeof(uintptr_t), "This platform is not supported.");

static int FreeSizeComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pIndex1, intptr_t nKey2){
	const uintptr_t u1 = GetInfoFromFreeSizeIndex(pIndex1)->uFreeSize;
	const uintptr_t u2 = (uintptr_t)nKey2;
	if(u1 != u2){
		return (u1 < u2) ? -1 : 1;
	}
	return 0;
}
static int FreeSizeComparatorNodes(const _MCFCRT_AvlNodeHeader *pIndex1, const _MCFCRT_AvlNodeHeader *pIndex2){
	return FreeSizeComparatorNodeKey(pIndex1, (intptr_t)GetInfoFromFreeSizeIndex(pIndex2)->uFreeSize);
}

static _MCFCRT_Mutex    g_vThunkMutex         = { 0 };
static _MCFCRT_AvlRoot  g_avlThunksByThunk    = _MCFCRT_NULLPTR;
static _MCFCRT_AvlRoot  g_avlThunksByFreeSize = _MCFCRT_NULLPTR;

const void *_MCFCRT_AllocateThunk(const void *pInit, size_t uSize){
	_MCFCRT_ASSERT(pInit);

	size_t uThunkSize = uSize + 8;
	uThunkSize = (uThunkSize + 0x0F) & (size_t)-0x10;
	if(uThunkSize < uSize){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return _MCFCRT_NULLPTR;
	}

	unsigned char *pbyRaw;

	_MCFCRT_WaitForMutexForever(&g_vThunkMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		_MCFCRT_AvlNodeHeader *pFreeSizeIndex = _MCFCRT_AvlGetLowerBound(&g_avlThunksByFreeSize, (intptr_t)uThunkSize, &FreeSizeComparatorNodeKey);
		ThunkInfo *pInfo;
		bool bInfoAllocated;
		if(pFreeSizeIndex){
			pInfo = GetInfoFromFreeSizeIndex(pFreeSizeIndex);
			bInfoAllocated = false;
		} else {
			// 如果没有足够大的 thunk，我们先分配一个新的 chunk。
			pInfo = _MCFCRT_malloc(sizeof(ThunkInfo));
			if(!pInfo){
				_MCFCRT_SignalMutex(&g_vThunkMutex);
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return _MCFCRT_NULLPTR;
			}
			size_t uChunkSize = (uThunkSize + 0xFFFF) & (size_t)-0x10000;
			if(uChunkSize < uThunkSize){
				uChunkSize = uThunkSize;
			}
			void *const pChunk = VirtualAlloc(0, uChunkSize, MEM_COMMIT | MEM_RESERVE, PAGE_READONLY);
			if(!pChunk){
				_MCFCRT_free(pInfo);
				_MCFCRT_SignalMutex(&g_vThunkMutex);
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return _MCFCRT_NULLPTR;
			}
			pInfo->pChunk     = pChunk;
			pInfo->uChunkSize = uChunkSize;

			pInfo->pThunk     = pChunk;
			pInfo->uThunkSize = uChunkSize;
			pInfo->uFreeSize  = uChunkSize;

			_MCFCRT_AvlAttach(&g_avlThunksByThunk, &(pInfo->vThunkIndex), &ThunkComparatorNodes);
			_MCFCRT_AvlAttach(&g_avlThunksByFreeSize, &(pInfo->vFreeSizeIndex), &FreeSizeComparatorNodes);

			pFreeSizeIndex = &(pInfo->vFreeSizeIndex);
			bInfoAllocated = true;
		}
		_MCFCRT_ASSERT(pInfo->uFreeSize >= uThunkSize);

		const size_t uRemaining = pInfo->uFreeSize - uThunkSize;
		if(uRemaining >= 0x20){
			// 如果剩下的空间还很大，保存成一个新的空闲 thunk。
			ThunkInfo *const pSpare = _MCFCRT_malloc(sizeof(ThunkInfo));
			if(!pSpare){
				if(bInfoAllocated){
					_MCFCRT_AvlDetach(&(pInfo->vThunkIndex));
					_MCFCRT_AvlDetach(&(pInfo->vFreeSizeIndex));
					VirtualFree(pInfo->pChunk, 0, MEM_RELEASE);
					_MCFCRT_free(pInfo);
				}
				_MCFCRT_SignalMutex(&g_vThunkMutex);
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return _MCFCRT_NULLPTR;
			}
			pSpare->pChunk     = pInfo->pChunk;
			pSpare->uChunkSize = pInfo->uChunkSize;
			pSpare->pThunk     = (unsigned char *)pInfo->pThunk + uThunkSize;
			pSpare->uThunkSize = uRemaining;
			pSpare->uFreeSize  = uRemaining;

			_MCFCRT_AvlAttach(&g_avlThunksByThunk, &(pSpare->vThunkIndex), &ThunkComparatorNodes);
			_MCFCRT_AvlAttach(&g_avlThunksByFreeSize, &(pSpare->vFreeSizeIndex), &FreeSizeComparatorNodes);

			pInfo->uThunkSize = uThunkSize;
		} else {
			uThunkSize = pInfo->uThunkSize;
		}

		_MCFCRT_AvlDetach(&(pInfo->vFreeSizeIndex));
		pInfo->uFreeSize = 0;
		_MCFCRT_AvlAttach(&g_avlThunksByFreeSize, &(pInfo->vFreeSizeIndex), &FreeSizeComparatorNodes);

		pbyRaw = pInfo->pThunk;

		// 由于其他 thunk 可能共享了当前内存页，所以不能设置为 PAGE_READWRITE。
		DWORD dwOldProtect;
		VirtualProtect(pbyRaw, uThunkSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		memcpy(pbyRaw, pInit, uSize);
		memset(pbyRaw + uSize, 0xCC, uThunkSize - uSize);
		VirtualProtect(pbyRaw, uThunkSize, PAGE_EXECUTE_READ, &dwOldProtect);
	}
	_MCFCRT_SignalMutex(&g_vThunkMutex);

	return pbyRaw;
}
void _MCFCRT_DeallocateThunk(const void *pThunk, bool bToPoison){
	unsigned char *const pbyRaw = (void *)pThunk;
	void *pPageToRelease;
	void *pInfoToFree;

	_MCFCRT_WaitForMutexForever(&g_vThunkMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		_MCFCRT_AvlNodeHeader *pThunkIndex = _MCFCRT_AvlFind(&g_avlThunksByThunk, (intptr_t)pThunk, &ThunkComparatorNodeKey);
		ThunkInfo *pInfo;
		if(!pThunkIndex || ((pInfo = GetInfoFromThunkIndex(pThunkIndex))->uFreeSize != 0)){
			_MCFCRT_Bail(L"_MCFCRT_DeallocateThunk() 失败：传入的指针无效。");
		}

#ifndef NDEBUG
		bToPoison = true;
#endif
		if(bToPoison){
			// 由于其他 thunk 可能共享了当前内存页，所以不能设置为 PAGE_READWRITE。
			DWORD dwOldProtect;
			VirtualProtect(pbyRaw, pInfo->uThunkSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			memset(pbyRaw, 0xCC, pInfo->uThunkSize);
			VirtualProtect(pbyRaw, pInfo->uThunkSize, PAGE_EXECUTE_READ, &dwOldProtect);
		}

		_MCFCRT_AvlNodeHeader *const pNextThunkIndex = _MCFCRT_AvlNext(pThunkIndex);
		if(pNextThunkIndex){
			// 如果连续的下一个 thunk 也处于空闲状态，把它合并到当前 thunk。
			ThunkInfo *const pNextInfo = GetInfoFromThunkIndex(pNextThunkIndex);
			if((pNextInfo->pChunk == pInfo->pChunk) && (pNextInfo->uFreeSize != 0)){
				pInfo->uThunkSize += pNextInfo->uThunkSize;

				_MCFCRT_AvlDetach(&(pNextInfo->vThunkIndex));
				_MCFCRT_AvlDetach(&(pNextInfo->vFreeSizeIndex));
				_MCFCRT_free(pNextInfo);
			}
		}

		_MCFCRT_AvlNodeHeader *const pPrevThunkIndex = _MCFCRT_AvlPrev(pThunkIndex);
		if(pPrevThunkIndex){
			// 如果连续的上一个 thunk 也处于空闲状态，把它合并到当前 thunk。
			ThunkInfo *const pPrevInfo = GetInfoFromThunkIndex(pPrevThunkIndex);
			if((pPrevInfo->pChunk == pInfo->pChunk) && (pPrevInfo->uFreeSize != 0)){
				pPrevInfo->uThunkSize += pInfo->uThunkSize;

				_MCFCRT_AvlDetach(&(pInfo->vThunkIndex));
				_MCFCRT_AvlDetach(&(pInfo->vFreeSizeIndex));
				_MCFCRT_free(pInfo);

				pThunkIndex = pPrevThunkIndex;
				pInfo = pPrevInfo;
			}
		}

		if((pInfo->pThunk == pInfo->pChunk) && (pInfo->uThunkSize == pInfo->uChunkSize)){
			pPageToRelease = pInfo->pChunk;
			pInfoToFree = pInfo;

			_MCFCRT_AvlDetach(&(pInfo->vThunkIndex));
			_MCFCRT_AvlDetach(&(pInfo->vFreeSizeIndex));
		} else {
			pPageToRelease = _MCFCRT_NULLPTR;
			pInfoToFree = _MCFCRT_NULLPTR;

			_MCFCRT_AvlDetach(&(pInfo->vFreeSizeIndex));
			pInfo->uFreeSize = pInfo->uThunkSize;
			_MCFCRT_AvlAttach(&g_avlThunksByFreeSize, &(pInfo->vFreeSizeIndex), &FreeSizeComparatorNodes);
		}
	}
	_MCFCRT_SignalMutex(&g_vThunkMutex);

	if(pPageToRelease){
		VirtualFree(pPageToRelease, 0, MEM_RELEASE);
	}
	if(pInfoToFree){
		_MCFCRT_free(pInfoToFree);
	}
}
