// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "thread.h"
#include "mcfwin.h"
#include "fenv.h"
#include "avl_tree.h"
#include "bail.h"
#include "mingw_hacks.h"
#include "eh_top.h"
#include "../ext/assert.h"
#include "../ext/unref_param.h"
#include <stdlib.h>

typedef struct tagTlsObject {
	MCF_AvlNodeHeader vHeader;

	intptr_t nValue;

	struct tagThreadMap *pMap;
	struct tagTlsObject *pPrevByThread;
	struct tagTlsObject *pNextByThread;

	struct tagTlsKey *pKey;
	struct tagTlsObject *pPrevByKey;
	struct tagTlsObject *pNextByKey;
} TlsObject;

static int ObjectComparatorNodeKey(const MCF_AvlNodeHeader *pObj1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(((const TlsObject *)pObj1)->pKey);
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int ObjectComparatorNodes(const MCF_AvlNodeHeader *pObj1, const MCF_AvlNodeHeader *pObj2){
	return ObjectComparatorNodeKey(pObj1, (intptr_t)(void *)(((const TlsObject *)pObj2)->pKey));
}

typedef struct tagThreadMap {
	CRITICAL_SECTION csMutex;
	MCF_AvlRoot pavlObjects;
	struct tagTlsObject *pLastByThread;
} ThreadMap;

typedef struct tagTlsKey {
	MCF_AvlNodeHeader vHeader;

	CRITICAL_SECTION csMutex;
	void (__cdecl *pfnCallback)(intptr_t);
	struct tagTlsObject *pLastByKey;
} TlsKey;

static int KeyComparatorNodeKey(const MCF_AvlNodeHeader *pObj1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(void *)pObj1;
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int KeyComparatorNodes(const MCF_AvlNodeHeader *pObj1, const MCF_AvlNodeHeader *pObj2){
	return KeyComparatorNodeKey(pObj1, (intptr_t)(void *)pObj2);
}

static CRITICAL_SECTION	g_csKeyMutex;
static DWORD			g_dwTlsIndex	= TLS_OUT_OF_INDEXES;
static MCF_AvlRoot		g_pavlKeys		= nullptr;

bool __MCF_CRT_TlsEnvInit(){
	if(!InitializeCriticalSectionEx(&g_csKeyMutex, 0x400u,
#ifdef NDEBUG
		CRITICAL_SECTION_NO_DEBUG_INFO
#else
		0
#endif
		))
	{
		return false;
	}
	g_dwTlsIndex = TlsAlloc();
	if(g_dwTlsIndex == TLS_OUT_OF_INDEXES){
		const DWORD dwError = GetLastError();
		DeleteCriticalSection(&g_csKeyMutex);
		SetLastError(dwError);
		return false;
	}
	return true;
}
void __MCF_CRT_TlsEnvUninit(){
	if(g_pavlKeys){
		MCF_AvlNodeHeader *const pRoot = g_pavlKeys;
		g_pavlKeys = nullptr;

		TlsKey *pKey;
		MCF_AvlNodeHeader *pCur = MCF_AvlPrev(pRoot);
		while(pCur){
			pKey = (TlsKey *)pCur;
			pCur = MCF_AvlPrev(pCur);
			DeleteCriticalSection(&(pKey->csMutex));
			free(pKey);
		}
		pCur = MCF_AvlNext(pRoot);
		while(pCur){
			pKey = (TlsKey *)pCur;
			pCur = MCF_AvlNext(pCur);
			DeleteCriticalSection(&(pKey->csMutex));
			free(pKey);
		}
		pKey = (TlsKey *)pRoot;
		DeleteCriticalSection(&(pKey->csMutex));
		free(pKey);
	}

	TlsFree(g_dwTlsIndex);
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;

	DeleteCriticalSection(&g_csKeyMutex);
}

void __stdcall __MCF_CRT_TlsCallback(void *hModule, unsigned long ulReason, void *pReserved){
	UNREF_PARAM(hModule);
	UNREF_PARAM(pReserved);

	if(ulReason == DLL_THREAD_DETACH){
		MCF_CRT_TlsClearAll();
	}
}

void *MCF_CRT_TlsAllocKey(void (__cdecl *pfnCallback)(intptr_t)){
	TlsKey *const pKey = malloc(sizeof(TlsKey));
	if(!pKey){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
	if(!InitializeCriticalSectionEx(&(pKey->csMutex), 0x400u,
#ifdef NDEBUG
		CRITICAL_SECTION_NO_DEBUG_INFO
#else
		0
#endif
		))
	{
		const DWORD dwError = GetLastError();
		free(pKey);
		SetLastError(dwError);
		return nullptr;
	}
	pKey->pfnCallback = pfnCallback;
	pKey->pLastByKey = nullptr;

	EnterCriticalSection(&g_csKeyMutex);
	{
		MCF_AvlAttach(&g_pavlKeys, (MCF_AvlNodeHeader *)pKey, &KeyComparatorNodes);
	}
	LeaveCriticalSection(&g_csKeyMutex);

	return pKey;
}
bool MCF_CRT_TlsFreeKey(void *pTlsKey){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	EnterCriticalSection(&g_csKeyMutex);
	{
		MCF_AvlDetach((MCF_AvlNodeHeader *)pKey);
	}
	LeaveCriticalSection(&g_csKeyMutex);

	TlsObject *pObject = pKey->pLastByKey;
	while(pObject){
		ThreadMap *const pMap = pObject->pMap;

		EnterCriticalSection(&(pMap->csMutex));
		{
			TlsObject *const pPrev = pObject->pPrevByThread;
			TlsObject *const pNext = pObject->pNextByThread;
			if(pPrev){
				pPrev->pNextByThread = pNext;
			}
			if(pNext){
				pNext->pPrevByThread = pPrev;
			}

			if(pMap->pLastByThread == pObject){
				pMap->pLastByThread = pObject->pPrevByThread;
			}
		}
		LeaveCriticalSection(&(pMap->csMutex));

		if(pKey->pfnCallback){
			(*pKey->pfnCallback)(pObject->nValue);
		}

		TlsObject *const pTemp = pObject->pPrevByKey;
		free(pObject);
		pObject = pTemp;
	}
	DeleteCriticalSection(&(pKey->csMutex));
	free(pKey);

	return true;
}

void (__cdecl *MCF_CRT_TlsGetCallback(void *pTlsKey))(intptr_t){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return nullptr;
	}
	SetLastError(ERROR_SUCCESS);
	return pKey->pfnCallback;
}
bool MCF_CRT_TlsGet(void *pTlsKey, bool *pbHasValue, intptr_t *pnValue){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	*pbHasValue = false;

	ThreadMap *const pMap = TlsGetValue(g_dwTlsIndex);
	if(!pMap){
		return true;
	}

	EnterCriticalSection(&(pMap->csMutex));
	{
		TlsObject *const pObject = (TlsObject *)MCF_AvlFind(
			&(pMap->pavlObjects), (intptr_t)pKey, &ObjectComparatorNodeKey);
		if(pObject){
			*pbHasValue = true;
			*pnValue = pObject->nValue;
		}
	}
	LeaveCriticalSection(&(pMap->csMutex));

	return true;
}
bool MCF_CRT_TlsReset(void *pTlsKey, intptr_t nNewValue){
	bool bHasOldValue;
	intptr_t nOldValue;
	if(!MCF_CRT_TlsExchange(pTlsKey, &bHasOldValue, &nOldValue, nNewValue)){
		return false;
	}
	if(bHasOldValue){
		TlsKey *const pKey = pTlsKey;
		if(pKey->pfnCallback){
			(*pKey->pfnCallback)(nOldValue);
		}
	}
	return true;
}
bool MCF_CRT_TlsExchange(void *pTlsKey, bool *pbHasOldValue, intptr_t *pnOldValue, intptr_t nNewValue){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	*pbHasOldValue = false;

	ThreadMap *pMap = TlsGetValue(g_dwTlsIndex);
	if(!pMap){
		pMap = malloc(sizeof(ThreadMap));
		if(!pMap){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		if(!InitializeCriticalSectionEx(&(pMap->csMutex), 0x400u,
#ifdef NDEBUG
			CRITICAL_SECTION_NO_DEBUG_INFO
#else
			0
#endif
			))
		{
			const DWORD dwError = GetLastError();
			free(pMap);
			SetLastError(dwError);
			return false;
		}
		pMap->pavlObjects = nullptr;
		pMap->pLastByThread = nullptr;

		TlsSetValue(g_dwTlsIndex, pMap);
	}

	EnterCriticalSection(&(pMap->csMutex));
	{
		TlsObject *const pObject = (TlsObject *)MCF_AvlFind(
			&(pMap->pavlObjects), (intptr_t)pKey, &ObjectComparatorNodeKey);
		if(pObject){
			*pbHasOldValue = true;
			*pnOldValue = pObject->nValue;
			pObject->nValue = nNewValue;
		}
	}
	LeaveCriticalSection(&(pMap->csMutex));

	if(!*pbHasOldValue){
		TlsObject *const pObject = malloc(sizeof(TlsObject));
		if(!pObject){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pObject->nValue = nNewValue;
		pObject->pMap = pMap;
		pObject->pKey = pKey;

		EnterCriticalSection(&(pMap->csMutex));
		{
			TlsObject *const pPrev = pMap->pLastByThread;
			pMap->pLastByThread = pObject;

			pObject->pPrevByThread = pPrev;
			pObject->pNextByThread = nullptr;
			if(pPrev){
				pPrev->pNextByThread = pObject;
			}
			MCF_AvlAttach(&(pMap->pavlObjects), (MCF_AvlNodeHeader *)pObject, &ObjectComparatorNodes);
		}
		LeaveCriticalSection(&(pMap->csMutex));

		EnterCriticalSection(&(pKey->csMutex));
		{
			TlsObject *const pPrev = pKey->pLastByKey;
			pKey->pLastByKey = pObject;

			pObject->pPrevByKey = pPrev;
			pObject->pNextByKey = nullptr;
			if(pPrev){
				pPrev->pNextByKey = pObject;
			}
		}
		LeaveCriticalSection(&(pKey->csMutex));
	}

	return true;
}
void MCF_CRT_TlsClearAll(){
	ThreadMap *const pMap = TlsGetValue(g_dwTlsIndex);
	if(pMap){
		TlsObject *pObject = pMap->pLastByThread;
		while(pObject){
			TlsKey *const pKey = pObject->pKey;

			EnterCriticalSection(&(pKey->csMutex));
			{
				if(pKey->pLastByKey == pObject){
					pKey->pLastByKey = pObject->pPrevByKey;
				}
			}
			LeaveCriticalSection(&(pKey->csMutex));

			if(pKey->pfnCallback){
				(*pKey->pfnCallback)(pObject->nValue);
			}

			TlsObject *const pTemp = pObject->pPrevByThread;
			free(pObject);
			pObject = pTemp;
		}
		DeleteCriticalSection(&(pMap->csMutex));
		free(pMap);
		TlsSetValue(g_dwTlsIndex, nullptr);
	}

	__MCF_CRT_RunEmutlsDtors();
}

int MCF_CRT_AtEndThread(void (__cdecl *pfnProc)(intptr_t), intptr_t nContext){
	void *const pKey = MCF_CRT_TlsAllocKey(pfnProc);
	if(!pKey){
		return -1;
	}
	if(!MCF_CRT_TlsReset(pKey, nContext)){
		const DWORD dwLastError = GetLastError();
		MCF_CRT_TlsFreeKey(pKey);
		SetLastError(dwLastError);
		return -1;
	}
	return 0;
}

typedef struct tagThreadInitInfo {
	unsigned (*pfnProc)(intptr_t);
	intptr_t nParam;
} ThreadInitInfo;

static __MCF_C_STDCALL __MCF_HAS_EH_TOP
DWORD CRTThreadProc(LPVOID pParam){
	DWORD dwExitCode;
	__MCF_EH_TOP_BEGIN
	{
		const ThreadInitInfo vInitInfo = *(ThreadInitInfo *)pParam;
		free(pParam);

		__MCF_CRT_FEnvInit();

		dwExitCode = (*vInitInfo.pfnProc)(vInitInfo.nParam);
	}
	__MCF_EH_TOP_END
	return dwExitCode;
}

void *MCF_CRT_CreateThread(unsigned (*pfnThreadProc)(intptr_t), intptr_t nParam, bool bSuspended, unsigned long *pulThreadId){
	ThreadInitInfo *const pInitInfo = malloc(sizeof(ThreadInitInfo));
	if(!pInitInfo){
		return nullptr;
	}
	pInitInfo->pfnProc	= pfnThreadProc;
	pInitInfo->nParam	= nParam;

	const HANDLE hThread = CreateThread(nullptr, 0, &CRTThreadProc, pInitInfo, CREATE_SUSPENDED, pulThreadId);
	if(!hThread){
		const DWORD dwErrorCode = GetLastError();
		free(pInitInfo);
		SetLastError(dwErrorCode);
		return nullptr;
	}

	if(!bSuspended){
		ResumeThread(hThread);
	}
	return (void *)hThread;
}
