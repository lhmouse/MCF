// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "_tls_common.h"
#include "mutex.h"
#include "avl_tree.h"
#include "heap.h"
#include "inline_mem.h"
#include <winerror.h>

typedef struct tagTlsKey {
	uintptr_t uCounter;

	size_t uSize;
	_MCFCRT_TlsConstructor pfnConstructor;
	_MCFCRT_TlsDestructor pfnDestructor;
	intptr_t nContext;
} TlsKey;

_MCFCRT_TlsKeyHandle _MCFCRT_TlsAllocKey(size_t uSize, _MCFCRT_TlsConstructor pfnConstructor, _MCFCRT_TlsDestructor pfnDestructor, intptr_t nContext){
	static volatile size_t s_uKeyCounter;

	TlsKey *const pKey = _MCFCRT_malloc(sizeof(TlsKey));
	if(!pKey){
		return _MCFCRT_NULLPTR;
	}
	pKey->uCounter       = __atomic_add_fetch(&s_uKeyCounter, 1, __ATOMIC_RELAXED);
	pKey->uSize          = uSize;
	pKey->pfnConstructor = pfnConstructor;
	pKey->pfnDestructor  = pfnDestructor;
	pKey->nContext       = nContext;

	return (_MCFCRT_TlsKeyHandle)pKey;
}
void _MCFCRT_TlsFreeKey(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		return;
	}

	_MCFCRT_free(pKey);
}

size_t _MCFCRT_TlsGetSize(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	return pKey->uSize;
}
_MCFCRT_TlsConstructor _MCFCRT_TlsGetConstructor(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	return pKey->pfnConstructor;
}
_MCFCRT_TlsDestructor _MCFCRT_TlsGetDestructor(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	return pKey->pfnDestructor;
}
intptr_t _MCFCRT_TlsGetContext(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	return pKey->nContext;
}

typedef struct tagTlsObjectKey {
	struct tagTlsKey *pKey;
	uintptr_t uCounter;
} TlsObjectKey;

typedef struct tagTlsObject {
	_MCFCRT_AvlNodeHeader avlhNodeByKey;

	_MCFCRT_TlsDestructor pfnDestructor;
	intptr_t nContext;

	struct tagTlsObject *pPrev; // By thread
	struct tagTlsObject *pNext; // By thread
	struct tagTlsObjectKey vObjectKey;

	alignas(max_align_t) unsigned char abyStorage[];
} TlsObject;

static inline int TlsObjectComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pObjSelf, intptr_t nKeyOther){
	const TlsObjectKey *const pIndexSelf = &((const TlsObject *)pObjSelf)->vObjectKey;
	const TlsObjectKey *const pIndexOther = (const TlsObjectKey *)nKeyOther;
	if(pIndexSelf->pKey != pIndexOther->pKey){
		return (pIndexSelf->pKey < pIndexOther->pKey) ? -1 : 1;
	}
	if(pIndexSelf->uCounter != pIndexOther->uCounter){
		return (pIndexSelf->uCounter < pIndexOther->uCounter) ? -1 : 1;
	}
	return 0;
}
static inline int TlsObjectComparatorNodes(const _MCFCRT_AvlNodeHeader *pObjSelf, const _MCFCRT_AvlNodeHeader *pObjOther){
	return TlsObjectComparatorNodeKey(pObjSelf, (intptr_t)&((const TlsObject *)pObjOther)->vObjectKey);
}

typedef struct tagTlsThreadMap {
	_MCFCRT_AvlRoot avlObjects;
	struct tagTlsObject *pLast; // By thread
	struct tagTlsObject *pFirst; // By thread
} TlsThreadMap;

_MCFCRT_TlsThreadMapHandle __MCFCRT_InternalTlsCreateThreadMap(void){
	TlsThreadMap *const pThreadMap = _MCFCRT_malloc(sizeof(TlsThreadMap));
	if(!pThreadMap){
		return _MCFCRT_NULLPTR;
	}
	pThreadMap->avlObjects = _MCFCRT_NULLPTR;
	pThreadMap->pLast      = _MCFCRT_NULLPTR;
	pThreadMap->pFirst     = _MCFCRT_NULLPTR;

	return (_MCFCRT_TlsThreadMapHandle)pThreadMap;
}
void __MCFCRT_InternalTlsDestroyThreadMap(_MCFCRT_TlsThreadMapHandle hThreadMap){
	TlsThreadMap *const pThreadMap = (TlsThreadMap *)hThreadMap;
	if(!pThreadMap){
		return;
	}

	for(;;){
		TlsObject *const pObject = pThreadMap->pLast;
		if(!pObject){
			break;
		}

		TlsObject *const pPrev = pObject->pPrev;
		TlsObject *const pNext = pObject->pNext;
		_MCFCRT_ASSERT(!pNext);
		if(pPrev){
			pPrev->pNext = pNext;
		} else {
			pThreadMap->pFirst = pNext;
		}
		if(pNext){
			pNext->pPrev = pPrev;
		} else {
			pThreadMap->pLast = pPrev;
		}

		const _MCFCRT_TlsDestructor pfnDestructor = pObject->pfnDestructor;
		if(pfnDestructor){
			(*pfnDestructor)(pObject->nContext, pObject->abyStorage);
		}
		_MCFCRT_free(pObject);
	}

	_MCFCRT_free(pThreadMap);
}

void __MCFCRT_InternalTlsGet(_MCFCRT_TlsThreadMapHandle hThreadMap, _MCFCRT_TlsKeyHandle hTlsKey, void **restrict ppStorage){
	TlsThreadMap *const pThreadMap = (TlsThreadMap *)hThreadMap;
	_MCFCRT_ASSERT(pThreadMap);
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	_MCFCRT_ASSERT(pKey);

	const TlsObjectKey vObjectKey = { pKey, pKey->uCounter };
	TlsObject *pObject = (TlsObject *)_MCFCRT_AvlFind(&(pThreadMap->avlObjects), (intptr_t)&vObjectKey, &TlsObjectComparatorNodeKey);
	if(!pObject){
		*ppStorage = _MCFCRT_NULLPTR;
		return;
	}
	*ppStorage = pObject->abyStorage;
}
unsigned long __MCFCRT_InternalTlsRequire(_MCFCRT_TlsThreadMapHandle hThreadMap, _MCFCRT_TlsKeyHandle hTlsKey, void **restrict ppStorage){
	TlsThreadMap *const pThreadMap = (TlsThreadMap *)hThreadMap;
	_MCFCRT_ASSERT(pThreadMap);
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	_MCFCRT_ASSERT(pKey);

#ifndef NDEBUG
	*ppStorage = (void *)0xDEADBEEF;
#endif

	const TlsObjectKey vObjectKey = { pKey, pKey->uCounter };
	TlsObject *pObject = (TlsObject *)_MCFCRT_AvlFind(&(pThreadMap->avlObjects), (intptr_t)&vObjectKey, &TlsObjectComparatorNodeKey);
	if(!pObject){
		const size_t uSizeToAlloc = sizeof(TlsObject) + pKey->uSize;
		if(uSizeToAlloc < sizeof(TlsObject)){
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		pObject = _MCFCRT_malloc(uSizeToAlloc);
		if(!pObject){
			return ERROR_NOT_ENOUGH_MEMORY;
		}
#ifndef NDEBUG
		_MCFCRT_inline_mempset_fwd(pObject, 0xAA, sizeof(TlsObject));
#endif
		_MCFCRT_inline_mempset_fwd(pObject->abyStorage, 0, pKey->uSize);
		if(pKey->pfnConstructor){
			const unsigned long ulErrorCode = (*(pKey->pfnConstructor))(pKey->nContext, pObject->abyStorage);
			if(ulErrorCode != 0){
				_MCFCRT_free(pObject);
				return ulErrorCode;
			}
		}
		pObject->pfnDestructor = pKey->pfnDestructor;
		pObject->nContext      = pKey->nContext;

		TlsObject *const pPrev = pThreadMap->pLast;
		TlsObject *const pNext = _MCFCRT_NULLPTR;
		if(pPrev){
			pPrev->pNext = pObject;
		} else {
			pThreadMap->pFirst = pObject;
		}
		if(pNext){
			pNext->pPrev = pObject;
		} else {
			pThreadMap->pLast = pObject;
		}
		pObject->pPrev = pPrev;
		pObject->pNext = pNext;

		pObject->vObjectKey = vObjectKey;
		_MCFCRT_AvlAttach(&(pThreadMap->avlObjects), (_MCFCRT_AvlNodeHeader *)pObject, &TlsObjectComparatorNodes);
	}
	*ppStorage = pObject->abyStorage;
	return 0;
}

#define CALLBACKS_PER_BLOCK   64u

typedef struct tagAtExitCallbackBlock {
	size_t uSize;
	struct {
		_MCFCRT_AtThreadExitCallback pfnProc;
		intptr_t nContext;
	} aCallbacks[CALLBACKS_PER_BLOCK];
} AtExitCallbackBlock;

static void CrtAtThreadExitDestructor(intptr_t nUnused, void *pStorage){
	(void)nUnused;

	AtExitCallbackBlock *const pBlock = pStorage;
	for(size_t uIndex = pBlock->uSize; uIndex != 0; --uIndex){
		(*(pBlock->aCallbacks[uIndex - 1].pfnProc))(pBlock->aCallbacks[uIndex - 1].nContext);
	}
}

unsigned long __MCFCRT_InternalAtThreadExit(_MCFCRT_TlsThreadMapHandle hThreadMap, _MCFCRT_AtThreadExitCallback pfnProc, intptr_t nContext){
	TlsThreadMap *const pThreadMap = (TlsThreadMap *)hThreadMap;
	_MCFCRT_ASSERT(pThreadMap);

	AtExitCallbackBlock *pBlock = _MCFCRT_NULLPTR;
	TlsObject *pObject = pThreadMap->pLast;
	if(pObject && (pObject->pfnDestructor == &CrtAtThreadExitDestructor)){
		pBlock = (void *)pObject->abyStorage;
	}
	if(!pBlock || (pBlock->uSize >= CALLBACKS_PER_BLOCK)){
		pObject = _MCFCRT_malloc(sizeof(TlsObject) + sizeof(AtExitCallbackBlock));
		if(!pObject){
			return ERROR_NOT_ENOUGH_MEMORY;
		}
#ifndef NDEBUG
		_MCFCRT_inline_mempset_fwd(pObject, 0xAA, sizeof(TlsObject));
#endif
		pBlock = (void *)pObject->abyStorage;
		pBlock->uSize = 0;

		TlsObject *const pPrev = pThreadMap->pLast;
		TlsObject *const pNext = _MCFCRT_NULLPTR;
		if(pPrev){
			pPrev->pNext = pObject;
		} else {
			pThreadMap->pFirst = pObject;
		}
		if(pNext){
			pNext->pPrev = pObject;
		} else {
			pThreadMap->pLast = pObject;
		}
		pObject->pPrev = pPrev;
		pObject->pNext = pNext;
	}
	const size_t uIndex = (pBlock->uSize)++;
	pBlock->aCallbacks[uIndex].pfnProc  = pfnProc;
	pBlock->aCallbacks[uIndex].nContext = nContext;
	return 0;
}
