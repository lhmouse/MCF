// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "tls.h"
#include "../env/mutex.h"
#include "../env/avl_tree.h"
#include "../env/mcfwin.h"
#include "../env/heap.h"
#include "../env/inline_mem.h"

static DWORD g_dwTlsIndex = TLS_OUT_OF_INDEXES;

typedef struct tagTlsObjectKey {
	struct tagTlsKey *pKey;
	uintptr_t uCounter;
} TlsObjectKey;

typedef struct tagTlsObject {
	_MCFCRT_AvlNodeHeader avlhNodeByKey;
	struct tagTlsObjectKey vObjectKey;

	_MCFCRT_TlsDestructor pfnDestructor;
	intptr_t nContext;

	struct tagTlsThread *pThread;
	struct tagTlsObject *pPrevByThread;
	struct tagTlsObject *pNextByThread;

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

typedef struct tagTlsThread {
	_MCFCRT_AvlRoot avlObjects;
	struct tagTlsObject *pFirstByThread;
	struct tagTlsObject *pLastByThread;
} TlsThread;

static void DestroyTls(TlsThread *pThread){
	if(!pThread){
		return;
	}

	for(;;){
		TlsObject *const pObject = pThread->pLastByThread;
		if(!pObject){
			break;
		}

		TlsObject *const pPrev = pObject->pPrevByThread;
		if(pPrev){
			pPrev->pNextByThread = _MCFCRT_NULLPTR;
		}
		pThread->pLastByThread = pPrev;

		const _MCFCRT_TlsDestructor pfnDestructor = pObject->pfnDestructor;
		if(pfnDestructor){
			(*pfnDestructor)(pObject->nContext, pObject->abyStorage);
		}
		_MCFCRT_free(pObject);
	}
	_MCFCRT_free(pThread);
}

bool __MCFCRT_TlsInit(void){
	const DWORD dwTlsIndex = TlsAlloc();
	if(dwTlsIndex == TLS_OUT_OF_INDEXES){
		return false;
	}

	g_dwTlsIndex = dwTlsIndex;
	return true;
}
void __MCFCRT_TlsUninit(void){
	const DWORD dwTlsIndex = g_dwTlsIndex;
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;

	TlsThread *const pThread = TlsGetValue(dwTlsIndex);
	DestroyTls(pThread);

	const bool bSucceeded = TlsFree(dwTlsIndex);
	_MCFCRT_ASSERT(bSucceeded);
}

static TlsThread *GetTlsForCurrentThread(void){
	const DWORD dwTlsIndex = g_dwTlsIndex;
	if(dwTlsIndex == TLS_OUT_OF_INDEXES){
		return _MCFCRT_NULLPTR;
	}
	TlsThread *const pThread = TlsGetValue(dwTlsIndex);
	return pThread;
}
static TlsThread *RequireTlsForCurrentThread(void){
	const DWORD dwTlsIndex = g_dwTlsIndex;
	if(dwTlsIndex == TLS_OUT_OF_INDEXES){
		SetLastError(ERROR_ACCESS_DENIED); // XXX: Pick a better error code?
		return _MCFCRT_NULLPTR;
	}
	TlsThread *pThread = TlsGetValue(dwTlsIndex);
	if(!pThread){
		pThread = _MCFCRT_malloc(sizeof(TlsThread));
		if(!pThread){
			return _MCFCRT_NULLPTR;
		}
		pThread->avlObjects     = _MCFCRT_NULLPTR;
		pThread->pFirstByThread = _MCFCRT_NULLPTR;
		pThread->pLastByThread  = _MCFCRT_NULLPTR;

		if(!TlsSetValue(dwTlsIndex, pThread)){
			const DWORD dwErrorCode = GetLastError();
			_MCFCRT_free(pThread);
			SetLastError(dwErrorCode);
			return _MCFCRT_NULLPTR;
		}
	}
	return pThread;
}

void __MCFCRT_TlsCleanup(void){
	const DWORD dwTlsIndex = g_dwTlsIndex;
	if(dwTlsIndex == TLS_OUT_OF_INDEXES){
		return;
	}
	TlsThread *const pThread = TlsGetValue(dwTlsIndex);
	if(!pThread){
		return;
	}
	TlsSetValue(dwTlsIndex, _MCFCRT_NULLPTR);

	DestroyTls(pThread);
}

typedef struct tagTlsKey {
	uintptr_t uCounter;

	size_t uSize;
	_MCFCRT_TlsConstructor pfnConstructor;
	_MCFCRT_TlsDestructor pfnDestructor;
	intptr_t nContext;
} TlsKey;

static TlsObject *GetTlsObject(TlsThread *pThread, TlsKey *pKey){
	_MCFCRT_ASSERT(pThread);

	if(!pKey){
		return _MCFCRT_NULLPTR;
	}

	const TlsObjectKey vObjectKey = { pKey, pKey->uCounter };
	TlsObject *const pObject = (TlsObject *)_MCFCRT_AvlFind(&(pThread->avlObjects), (intptr_t)&vObjectKey, &TlsObjectComparatorNodeKey);
	return pObject;
}
static TlsObject *RequireTlsObject(TlsThread *pThread, TlsKey *pKey, size_t uSize, _MCFCRT_TlsConstructor pfnConstructor, _MCFCRT_TlsDestructor pfnDestructor, intptr_t nContext){
	TlsObject *pObject = GetTlsObject(pThread, pKey);
	if(!pObject){
		const size_t uSizeToAlloc = sizeof(TlsObject) + uSize;
		if(uSizeToAlloc < sizeof(TlsObject)){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return _MCFCRT_NULLPTR;
		}
		pObject = _MCFCRT_malloc(uSizeToAlloc);
		if(!pObject){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return _MCFCRT_NULLPTR;
		}
#ifndef NDEBUG
		_MCFCRT_inline_mempset_fwd(pObject, 0xAA, sizeof(TlsObject));
#endif
		_MCFCRT_inline_mempset_fwd(pObject->abyStorage, 0, uSize);

		if(pfnConstructor){
			const DWORD dwErrorCode = (*pfnConstructor)(nContext, pObject->abyStorage);
			if(dwErrorCode != 0){
				_MCFCRT_free(pObject);
				SetLastError(dwErrorCode);
				return _MCFCRT_NULLPTR;
			}
		}

		pObject->pfnDestructor = pfnDestructor;
		pObject->nContext      = nContext;

		TlsObject *const pPrev = pThread->pLastByThread;
		TlsObject *const pNext = _MCFCRT_NULLPTR;
		if(pPrev){
			pPrev->pNextByThread = pObject;
		} else {
			pThread->pFirstByThread = pObject;
		}
		if(pNext){
			pNext->pPrevByThread = pObject;
		} else {
			pThread->pLastByThread = pObject;
		}
		pObject->pThread = pThread;
		pObject->pPrevByThread = pPrev;
		pObject->pNextByThread = pNext;

		if(pKey){
			pObject->vObjectKey.pKey     = pKey;
			pObject->vObjectKey.uCounter = pKey->uCounter;
			_MCFCRT_AvlAttach(&(pThread->avlObjects), (_MCFCRT_AvlNodeHeader *)pObject, &TlsObjectComparatorNodes);
		}
	}
	return pObject;
}

_MCFCRT_TlsKeyHandle _MCFCRT_TlsAllocKey(size_t uSize, _MCFCRT_TlsConstructor pfnConstructor, _MCFCRT_TlsDestructor pfnDestructor, intptr_t nContext){
	static volatile size_t s_uTlsKeyCounter = 0;

	TlsKey *const pKey = _MCFCRT_malloc(sizeof(TlsKey));
	if(!pKey){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return _MCFCRT_NULLPTR;
	}
	pKey->uCounter       = __atomic_add_fetch(&s_uTlsKeyCounter, 1, __ATOMIC_RELAXED);
	pKey->uSize          = uSize;
	pKey->pfnConstructor = pfnConstructor;
	pKey->pfnDestructor  = pfnDestructor;
	pKey->nContext       = nContext;
	return (_MCFCRT_TlsKeyHandle)pKey;
}
void _MCFCRT_TlsFreeKey(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
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

bool _MCFCRT_TlsGet(_MCFCRT_TlsKeyHandle hTlsKey, void **restrict ppStorage){
#ifndef NDEBUG
	*ppStorage = (void *)0xDEADBEEF;
#endif

	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	TlsThread *const pThread = GetTlsForCurrentThread();
	if(!pThread){
		*ppStorage = _MCFCRT_NULLPTR;
		return true;
	}
	TlsObject *const pObject = GetTlsObject(pThread, pKey);
	if(!pObject){
		*ppStorage = _MCFCRT_NULLPTR;
		return true;
	}
	*ppStorage = pObject->abyStorage;
	return true;
}
bool _MCFCRT_TlsRequire(_MCFCRT_TlsKeyHandle hTlsKey, void **restrict ppStorage){
#ifndef NDEBUG
	*ppStorage = (void *)0xDEADBEEF;
#endif

	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	TlsThread *const pThread = RequireTlsForCurrentThread();
	if(!pThread){
		return false;
	}
	TlsObject *const pObject = RequireTlsObject(pThread, pKey, pKey->uSize, pKey->pfnConstructor, pKey->pfnDestructor, pKey->nContext);
	if(!pObject){
		return false;
	}
	*ppStorage = pObject->abyStorage;
	return true;
}

#define CALLBACKS_PER_BLOCK   64u

typedef struct tagAtExitCallbackBlock {
	size_t uSize;
	struct {
		_MCFCRT_AtThreadExitCallback pfnProc;
		intptr_t nContext;
	} aCallbacks[CALLBACKS_PER_BLOCK];
} AtExitCallbackBlock;

static unsigned long CrtAtThreadExitConstructor(intptr_t nUnused, void *pStorage){
	(void)nUnused;

	AtExitCallbackBlock *const pBlock = pStorage;
	pBlock->uSize = 0;
	return 0;
}
static void CrtAtThreadExitDestructor(intptr_t nUnused, void *pStorage){
	(void)nUnused;

	AtExitCallbackBlock *const pBlock = pStorage;
	for(size_t i = pBlock->uSize; i != 0; --i){
		(*(pBlock->aCallbacks[i - 1].pfnProc))(pBlock->aCallbacks[i - 1].nContext);
	}
}

bool _MCFCRT_AtThreadExit(_MCFCRT_AtThreadExitCallback pfnProc, intptr_t nContext){
	TlsThread *const pThread = RequireTlsForCurrentThread();
	if(!pThread){
		return false;
	}
	AtExitCallbackBlock *pBlock = _MCFCRT_NULLPTR;
	TlsObject *pObject = pThread->pLastByThread;
	if(pObject && (pObject->pfnDestructor == &CrtAtThreadExitDestructor)){
		pBlock = (void *)pObject->abyStorage;
	}
	if(!pBlock || (pBlock->uSize >= CALLBACKS_PER_BLOCK)){
		pObject = RequireTlsObject(pThread, _MCFCRT_NULLPTR, sizeof(AtExitCallbackBlock), &CrtAtThreadExitConstructor, &CrtAtThreadExitDestructor, 0);
		if(!pObject){
			return false;
		}
		pBlock = (void *)pObject->abyStorage;
	}
	const size_t uIndex = (pBlock->uSize)++;
	pBlock->aCallbacks[uIndex].pfnProc = pfnProc;
	pBlock->aCallbacks[uIndex].nContext = nContext;
	return true;
}
