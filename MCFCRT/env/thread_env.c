// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "thread_env.h"
#include "mcfwin.h"
#include "avl_tree.h"
#include "mutex.h"
#include "../ext/assert.h"
#include <stdlib.h>

typedef struct tagTlsObject {
	_MCFCRT_AvlNodeHeader avlhNodeByKey;

	_MCFCRT_TlsDestructor pfnDestructor;
	intptr_t nContext;

	struct tagTlsThread *pThread;
	struct tagTlsObject *pPrevByThread;
	struct tagTlsObject *pNextByThread;

	struct tagTlsKey *pKey;
	struct tagTlsObject *pPrevByKey;
	struct tagTlsObject *pNextByKey;

	alignas(max_align_t) unsigned char abyStorage[];
} TlsObject;

static inline int CompareTlsKeys(const struct tagTlsKey *pKey1, const struct tagTlsKey *pKey2){
	const uintptr_t u1 = (uintptr_t)pKey1, u2 = (uintptr_t)pKey2;
	if(u1 < u2){
		return -1;
	} else if(u1 > u2){
		return 1;
	}
	return 0;
}

static inline int ObjectComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pObj1, intptr_t nKey2){
	return CompareTlsKeys(((const TlsObject *)pObj1)->pKey,
                          (struct tagTlsKey *)nKey2);
}
static inline int ObjectComparatorNodes(const _MCFCRT_AvlNodeHeader *pObj1, const _MCFCRT_AvlNodeHeader *pObj2){
	return CompareTlsKeys(((const TlsObject *)pObj1)->pKey,
	                      ((const TlsObject *)pObj2)->pKey);
}

typedef struct tagTlsKey {
	size_t uSize;
	_MCFCRT_TlsConstructor pfnConstructor;
	_MCFCRT_TlsDestructor pfnDestructor;
	intptr_t nContext;

	_MCFCRT_Mutex vMutex;
	struct tagTlsObject *pFirstByKey;
	struct tagTlsObject *pLastByKey;
} TlsKey;

typedef struct tagTlsThread {
	_MCFCRT_AvlRoot avlObjects;
	struct tagTlsObject *pFirstByThread;
	struct tagTlsObject *pLastByThread;
} TlsThread;

static DWORD g_dwTlsIndex = TLS_OUT_OF_INDEXES;

static TlsThread *GetTlsForCurrentThread(void){
	_MCFCRT_ASSERT(g_dwTlsIndex != TLS_OUT_OF_INDEXES);

	TlsThread *pThread = TlsGetValue(g_dwTlsIndex);
	return pThread;
}
static TlsThread *RequireTlsForCurrentThread(void){
	_MCFCRT_ASSERT(g_dwTlsIndex != TLS_OUT_OF_INDEXES);

	TlsThread *pThread = TlsGetValue(g_dwTlsIndex);
	if(!pThread){
		pThread = malloc(sizeof(TlsThread));
		if(!pThread){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return nullptr;
		}
		pThread->avlObjects     = nullptr;
		pThread->pFirstByThread = nullptr;
		pThread->pLastByThread  = nullptr;

		TlsSetValue(g_dwTlsIndex, pThread);
	}
	return pThread;
}

bool __MCFCRT_ThreadEnvInit(void){
	const DWORD dwTlsIndex = TlsAlloc();
	if(dwTlsIndex == TLS_OUT_OF_INDEXES){
		return false;
	}

	g_dwTlsIndex = dwTlsIndex;
	return true;
}
void __MCFCRT_ThreadEnvUninit(void){
	const DWORD dwTlsIndex = g_dwTlsIndex;
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;

	TlsFree(dwTlsIndex);
}

void __MCFCRT_TlsCleanup(void){
	TlsThread *const pThread = GetTlsForCurrentThread();
	if(!pThread){
		return;
	}

	TlsObject *pObject = pThread->pLastByThread;
	while(pObject){
		TlsKey *const pKey = pObject->pKey;
		if(pKey){
			_MCFCRT_WaitForMutexForever(&(pKey->vMutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
			{
				TlsObject *const pPrev = pObject->pPrevByKey;
				TlsObject *const pNext = pObject->pNextByKey;
				if(pPrev){
					pPrev->pNextByKey = pNext;
				} else {
					pKey->pFirstByKey = pNext;
				}
				if(pNext){
					pNext->pPrevByKey = pPrev;
				} else {
					pKey->pLastByKey = pPrev;
				}
			}
			_MCFCRT_SignalMutex(&(pKey->vMutex));
		}

		if(pObject->pfnDestructor){
			(*(pObject->pfnDestructor))(pObject->nContext, pObject->abyStorage);
		}

		TlsObject *const pPrevByThread = pObject->pPrevByThread;
		free(pObject);
		pObject = pPrevByThread;
	}

	TlsSetValue(g_dwTlsIndex, nullptr);
	free(pThread);
}

_MCFCRT_TlsKeyHandle _MCFCRT_TlsAllocKey(size_t uSize, _MCFCRT_TlsConstructor pfnConstructor, _MCFCRT_TlsDestructor pfnDestructor, intptr_t nContext){
	TlsKey *const pKey = malloc(sizeof(TlsKey));
	if(!pKey){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
	pKey->uSize          = uSize;
	pKey->pfnConstructor = pfnConstructor;
	pKey->pfnDestructor  = pfnDestructor;
	pKey->nContext       = nContext;
	_MCFCRT_InitializeMutex(&(pKey->vMutex));
	pKey->pFirstByKey    = nullptr;
	pKey->pLastByKey     = nullptr;

	return (_MCFCRT_TlsKeyHandle)pKey;
}
bool _MCFCRT_TlsFreeKey(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	_MCFCRT_ASSERT_MSG(pKey->pLastByKey == nullptr, L"TLS 键仍在被使用。");

	free(pKey);
	return true;
}

size_t _MCFCRT_TlsGetSize(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);
	return pKey->uSize;
}
_MCFCRT_TlsConstructor _MCFCRT_TlsGetConstructor(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);
	return pKey->pfnConstructor;
}
_MCFCRT_TlsDestructor _MCFCRT_TlsGetDestructor(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);
	return pKey->pfnDestructor;
}
intptr_t _MCFCRT_TlsGetContext(_MCFCRT_TlsKeyHandle hTlsKey){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);
	return pKey->nContext;
}

bool _MCFCRT_TlsGet(_MCFCRT_TlsKeyHandle hTlsKey, void **restrict ppStorage){
	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	TlsThread *const pThread = GetTlsForCurrentThread();
	if(!pThread){
		*ppStorage = nullptr;
		return true;
	}

	TlsObject *pObject = (TlsObject *)_MCFCRT_AvlFind(&(pThread->avlObjects), (intptr_t)pKey, &ObjectComparatorNodeKey);
	if(!pObject){
		*ppStorage = nullptr;
		return true;
	}

	*ppStorage = pObject->abyStorage;
	return true;
}
bool _MCFCRT_TlsRequire(_MCFCRT_TlsKeyHandle hTlsKey, void **restrict ppStorage){
	*ppStorage = nullptr;

	TlsKey *const pKey = (TlsKey *)hTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	TlsThread *const pThread = RequireTlsForCurrentThread();
	if(!pThread){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}

	TlsObject *pObject = (TlsObject *)_MCFCRT_AvlFind(&(pThread->avlObjects), (intptr_t)pKey, &ObjectComparatorNodeKey);
	if(!pObject){
		const size_t uSizeToAlloc = sizeof(TlsObject) + pKey->uSize;
		if(uSizeToAlloc < sizeof(TlsObject)){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pObject = malloc(uSizeToAlloc);
		if(!pObject){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}

		memset(pObject->abyStorage, 0, pKey->uSize);
		if(pKey->pfnConstructor){
			const DWORD dwErrorCode = (*(pKey->pfnConstructor))(pKey->nContext, pObject->abyStorage);
			if(dwErrorCode != 0){
				free(pObject);
				SetLastError(dwErrorCode);
				return false;
			}
		}

		pObject->pfnDestructor = pKey->pfnDestructor;
		pObject->nContext = pKey->nContext;

		_MCFCRT_AvlAttach(&(pThread->avlObjects), (_MCFCRT_AvlNodeHeader *)pObject, &ObjectComparatorNodes);

		{
			TlsObject *const pPrev = pThread->pLastByThread;
			TlsObject *const pNext = nullptr;
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
		}

		_MCFCRT_WaitForMutexForever(&(pKey->vMutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
		{
			TlsObject *const pPrev = pKey->pLastByKey;
			TlsObject *const pNext = nullptr;
			if(pPrev){
				pPrev->pNextByKey = pObject;
			} else {
				pKey->pFirstByKey = pObject;
			}
			if(pNext){
				pNext->pPrevByKey = pObject;
			} else {
				pKey->pLastByKey = pObject;
			}

			pObject->pKey = pKey;
			pObject->pPrevByKey = pPrev;
			pObject->pNextByKey = pNext;
		}
		_MCFCRT_SignalMutex(&(pKey->vMutex));
	}

	*ppStorage = pObject->abyStorage;
	return true;
}

static void CrtAtExitThreadProc(intptr_t nContext, void *pStorage){
	const _MCFCRT_AtThreadExitCallback pfnProc = *(_MCFCRT_AtThreadExitCallback *)pStorage;
	(*pfnProc)(nContext);
}

bool _MCFCRT_AtThreadExit(_MCFCRT_AtThreadExitCallback pfnProc, intptr_t nContext){
	TlsThread *const pThread = RequireTlsForCurrentThread();
	if(!pThread){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}

	const size_t uSizeToAlloc = sizeof(TlsObject) + sizeof(pfnProc);
	TlsObject *const pObject = malloc(uSizeToAlloc);
	if(!pObject){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
#ifndef NDEBUG
	memset(pObject, 0xAA, uSizeToAlloc);
#endif

	memcpy(pObject->abyStorage, &pfnProc, sizeof(pfnProc));
	pObject->pfnDestructor = &CrtAtExitThreadProc;
	pObject->nContext = nContext;

	TlsObject *const pPrev = pThread->pLastByThread;
	TlsObject *const pNext = nullptr;
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

	pObject->pKey = nullptr;

	return true;
}
