// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "thread.h"
#include "mcfwin.h"
#include "fenv.h"
#include "avl_tree.h"
#include "bail_out.h"
#include "mingw_hacks.h"
#include "_eh_top.h"
#include "../ext/assert.h"
#include "../ext/unref_param.h"
#include <stdlib.h>

// 结构定义。
// 全局的用于描述已分配的 Tls 键的结构。
typedef struct tagTlsKey {
	MCF_AvlNodeHeader vHeader;

	void (__cdecl *pfnCallback)(intptr_t);
	struct tagTlsObject *pLastByKey;
} TlsKey;

static bool KeyComparatorNodes(
	const MCF_AvlNodeHeader *pObj1,
	const MCF_AvlNodeHeader *pObj2
){
	return (uintptr_t)(void *)pObj1 < (uintptr_t)(void *)pObj2;
}
static __attribute__((__unused__)) bool KeyComparatorNodeKey(
	const MCF_AvlNodeHeader *pObj1,
	intptr_t nKey2
){
	return (uintptr_t)(void *)pObj1 < (uintptr_t)(void *)nKey2;
}
static __attribute__((__unused__)) bool KeyComparatorKeyNode(
	intptr_t nKey1,
	const MCF_AvlNodeHeader *pObj2
){
	return (uintptr_t)(void *)nKey1 < (uintptr_t)(void *)pObj2;
}

// 用于描述每个线程内所有 Tls 对象的结构。
typedef struct tagTlsMap {
	SRWLOCK srwLock;
	MCF_AvlRoot pavlObjects;
	struct tagTlsObject *pLastInThread;
	struct tagTlsMap *pPrevMap;
	struct tagTlsMap *pNextMap;
} TlsMap;

// 用于描述每个线程内的每个 Tls 对象的结构。
typedef struct tagTlsObject {
	MCF_AvlNodeHeader vHeader;

	struct tagTlsObject *pPrevByKey;
	struct tagTlsObject *pNextByKey;
	struct tagTlsObject *pPrevInThread;
	struct tagTlsObject *pNextInThread;

	struct tagTlsMap *pMap;
	TlsKey *pKey;
	intptr_t nValue;
} TlsObject;

static bool ObjectComparatorNodes(
	const MCF_AvlNodeHeader *pObj1,
	const MCF_AvlNodeHeader *pObj2
){
	return (uintptr_t)(void *)((const TlsObject *)pObj1)->pKey <
		(uintptr_t)(void *)((const TlsObject *)pObj2)->pKey;
}
static bool ObjectComparatorNodeKey(
	const MCF_AvlNodeHeader *pObj1,
	intptr_t nKey2
){
	return (uintptr_t)(void *)((const TlsObject *)pObj1)->pKey <
		(uintptr_t)(void *)nKey2;
}
static bool ObjectComparatorKeyNode(
	intptr_t nKey1,
	const MCF_AvlNodeHeader *pObj2
){
	return (uintptr_t)(void *)nKey1 <
		(uintptr_t)(void *)((const TlsObject *)pObj2)->pKey;
}

// 全局变量。
static DWORD		g_dwTlsIndex	= TLS_OUT_OF_INDEXES;

static SRWLOCK		g_srwLock		= SRWLOCK_INIT;
static TlsMap *	g_pLastMap		= NULL;
static MCF_AvlRoot	g_pavlKeys		= NULL;

bool __MCF_CRT_TlsEnvInit(){
	g_dwTlsIndex = TlsAlloc();
	if(g_dwTlsIndex == TLS_OUT_OF_INDEXES){
		return false;
	}
	return true;
}
void __MCF_CRT_TlsEnvUninit(){
	TlsFree(g_dwTlsIndex);
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;

	while(g_pLastMap){
		TlsObject *pObject = g_pLastMap->pLastInThread;
		while(pObject){
			if(pObject->pKey->pfnCallback){
				(*(pObject->pKey->pfnCallback))(pObject->nValue);
			}

			TlsObject *const pPrevInThread = pObject->pPrevInThread;
			free(pObject);
			pObject = pPrevInThread;
		}

		TlsMap *const pPrevMap = g_pLastMap->pPrevMap;
		free(g_pLastMap);
		g_pLastMap = pPrevMap;
	}
	if(g_pavlKeys){
		TlsKey *pCur = (TlsKey *)MCF_AvlPrev(g_pavlKeys);
		while(pCur){
			TlsKey *const pPrev = (TlsKey *)MCF_AvlPrev((MCF_AvlNodeHeader *)pCur);
			free(pCur);
			pCur = pPrev;
		}

		pCur = (TlsKey *)MCF_AvlNext(g_pavlKeys);
		while(pCur){
			TlsKey *const pNext = (TlsKey *)MCF_AvlNext((MCF_AvlNodeHeader *)pCur);
			free(pCur);
			pCur = pNext;
		}

		free(g_pavlKeys);
		g_pavlKeys = NULL;
	}
}

__MCF_C_STDCALL
void __MCF_CRT_TlsCallback(void *hModule, unsigned long ulReason, void *pReserved){
	UNREF_PARAM(hModule);
	UNREF_PARAM(pReserved);

	if(ulReason == DLL_THREAD_DETACH){
		MCF_CRT_TlsClearAll();
	}
}

void *MCF_CRT_AtThreadExit(void (__cdecl *pfnProc)(intptr_t), intptr_t nContext){
	void *const pKey = MCF_CRT_TlsAllocKey(pfnProc);
	if(!pKey){
		return NULL;
	}
	if(!MCF_CRT_TlsReset(pKey, nContext)){
		const DWORD dwLastError = GetLastError();
		MCF_CRT_TlsFreeKey(pKey);
		SetLastError(dwLastError);
		return NULL;
	}
	return pKey;
}
bool MCF_CRT_RemoveAtThreadExit(void *pTlsKey){
	return MCF_CRT_TlsFreeKey(pTlsKey);
}

void *MCF_CRT_TlsAllocKey(void (__cdecl *pfnCallback)(intptr_t)){
	TlsKey *const pKey = malloc(sizeof(TlsKey));
	if(!pKey){
		return NULL;
	}
	pKey->pfnCallback	= pfnCallback;
	pKey->pLastByKey	= NULL;

	AcquireSRWLockExclusive(&g_srwLock);
	{
		MCF_AvlAttach(
			&g_pavlKeys,
			(MCF_AvlNodeHeader *)pKey,
			&KeyComparatorNodes
		);
	}
	ReleaseSRWLockExclusive(&g_srwLock);

	return pKey;
}
bool MCF_CRT_TlsFreeKey(void *pTlsKey){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		return false;
	}

	AcquireSRWLockExclusive(&g_srwLock);
	{
		ASSERT((TlsKey *)MCF_AvlFind(
			&g_pavlKeys,
			(intptr_t)pKey,
			&KeyComparatorNodeKey,
			&KeyComparatorKeyNode
		) == pKey);

		MCF_AvlDetach((MCF_AvlNodeHeader *)pKey);
	}
	ReleaseSRWLockExclusive(&g_srwLock);

	TlsObject *pObject = pKey->pLastByKey;
	while(pObject){
		AcquireSRWLockExclusive(&(pObject->pMap->srwLock));
		{
			TlsObject *const pPrevInThread = pObject->pPrevInThread;
			TlsObject *const pNextInThread = pObject->pNextInThread;
			if(pPrevInThread){
				pPrevInThread->pNextInThread = pNextInThread;
			}
			if(pNextInThread){
				pNextInThread->pPrevInThread = pPrevInThread;
			}

			if(pObject->pMap->pLastInThread == pObject){
				pObject->pMap->pLastInThread = pPrevInThread;
			}
		}
		ReleaseSRWLockExclusive(&(pObject->pMap->srwLock));

		if(pObject->pKey->pfnCallback){
			(*(pObject->pKey->pfnCallback))(pObject->nValue);
		}

		TlsObject *const pPrevByKey = pObject->pPrevByKey;
		free(pObject);
		pObject = pPrevByKey;
	}
	free(pKey);

	return true;
}

bool MCF_CRT_TlsGet(void *pTlsKey, intptr_t *pnValue){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		return false;
	}

	bool bRet = false;
	TlsMap *const pMap = TlsGetValue(g_dwTlsIndex);
	if(pMap){
		AcquireSRWLockShared(&(pMap->srwLock));
		{
			TlsObject *const pObject = (TlsObject *)MCF_AvlFind(
				&(pMap->pavlObjects),
				(intptr_t)pTlsKey,
				&ObjectComparatorNodeKey,
				&ObjectComparatorKeyNode
			);
			if(pObject){
				*pnValue = pObject->nValue;
				bRet = true;
			}
		}
		ReleaseSRWLockShared(&(pMap->srwLock));
	}
	return bRet;
}

static MCF_TlsExchangeResult TlsExchange(
	void *pTlsKey,
	void (__cdecl **ppfnCallback)(intptr_t),
	intptr_t *pnOldValue,
	intptr_t nNewValue
){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		return MCF_TLSXCH_FAILED;
	}

#ifndef NDEBUG
	AcquireSRWLockShared(&g_srwLock);
	{
		ASSERT((TlsKey *)MCF_AvlFind(
			&g_pavlKeys,
			(intptr_t)pKey,
			&KeyComparatorNodeKey,
			&KeyComparatorKeyNode
		) == pKey);
	}
	ReleaseSRWLockShared(&g_srwLock);
#endif

	TlsMap *pMap = TlsGetValue(g_dwTlsIndex);
	if(!pMap){
		pMap = malloc(sizeof(TlsMap));
		if(!pMap){
			return MCF_TLSXCH_FAILED;
		}
		const SRWLOCK vLockInit = SRWLOCK_INIT;
		pMap->srwLock		= vLockInit;
		pMap->pavlObjects	= NULL;
		pMap->pLastInThread	= NULL;
		pMap->pNextMap		= NULL;

		AcquireSRWLockExclusive(&g_srwLock);
		{
			TlsMap *const pPrevMap = g_pLastMap;
			g_pLastMap = pMap;
			pMap->pPrevMap = pPrevMap;
			if(pPrevMap){
				pPrevMap->pNextMap = pMap;
			}
		}
		ReleaseSRWLockExclusive(&g_srwLock);

		TlsSetValue(g_dwTlsIndex, pMap);
	}

	MCF_TlsExchangeResult eRet;

	AcquireSRWLockExclusive(&(pMap->srwLock));
	{
		TlsObject *pObject = (TlsObject *)MCF_AvlLowerBound(
			&(pMap->pavlObjects),
			(intptr_t)pKey,
			&ObjectComparatorNodeKey
		);
		if(!pObject || (pObject->pKey != pKey)){
			ReleaseSRWLockExclusive(&(pMap->srwLock));
			{
				const TlsObject *const pHint = pObject;
				pObject = malloc(sizeof(TlsObject));
				if(!pObject){
					return false;
				}
				pObject->pMap	= pMap;
				pObject->pKey	= pKey;
				pObject->nValue	= nNewValue;

				MCF_AvlAttachHint(
					&(pMap->pavlObjects),
					(MCF_AvlNodeHeader *)pHint,
					(MCF_AvlNodeHeader *)pObject,
					&ObjectComparatorNodes
				);
			}
			AcquireSRWLockExclusive(&(pMap->srwLock));

			eRet = MCF_TLSXCH_NEW_VAL_SET;
		} else {
			ASSERT(pObject->pMap == pMap);
			ASSERT(pObject->pKey == pKey);

			if(ppfnCallback){
				*ppfnCallback = pObject->pKey->pfnCallback;
			}
			*pnOldValue = pObject->nValue;

			TlsObject *const pPrevByKey = pObject->pPrevByKey;
			TlsObject *const pNextByKey = pObject->pNextByKey;
			if(pPrevByKey){
				pPrevByKey->pNextByKey = pNextByKey;
			}
			if(pNextByKey){
				pNextByKey->pPrevByKey = pPrevByKey;
			}
			if(pKey->pLastByKey == pObject){
				pKey->pLastByKey = pPrevByKey;
			}

			TlsObject *const pPrevInThread = pObject->pPrevInThread;
			TlsObject *const pNextInThread = pObject->pNextInThread;
			if(pPrevInThread){
				pPrevInThread->pNextInThread = pNextInThread;
			}
			if(pNextInThread){
				pNextInThread->pPrevInThread = pPrevInThread;
			}
			if(pMap->pLastInThread == pObject){
				pMap->pLastInThread = pPrevInThread;
			}

			pObject->nValue	= nNewValue;

			eRet = MCF_TLSXCH_OLD_VAL_RETURNED;
		}

		AcquireSRWLockExclusive(&g_srwLock);
		{
			TlsObject *const pPrevByKey = pKey->pLastByKey;
			pKey->pLastByKey = pObject;
			if(pPrevByKey){
				pPrevByKey->pNextByKey = pObject;
			}
			pObject->pPrevByKey	= pPrevByKey;
			pObject->pNextByKey	= NULL;
		}
		ReleaseSRWLockExclusive(&g_srwLock);

		TlsObject *const pPrevInThread = pMap->pLastInThread;
		pMap->pLastInThread = pObject;
		if(pPrevInThread){
			pPrevInThread->pNextInThread = pObject;
		}
		pObject->pPrevInThread	= pPrevInThread;
		pObject->pNextInThread	= NULL;
	}
	ReleaseSRWLockExclusive(&(pMap->srwLock));

	return eRet;
}

bool MCF_CRT_TlsReset(void *pTlsKey, intptr_t nNewValue){
	void (__cdecl *pfnCallback)(intptr_t) = NULL;
	intptr_t nOldValue = 0;
	switch(TlsExchange(pTlsKey, &pfnCallback, &nOldValue, nNewValue)){
	case MCF_TLSXCH_OLD_VAL_RETURNED:
		if(pfnCallback){
			(*pfnCallback)(nOldValue);
		}
	case MCF_TLSXCH_NEW_VAL_SET:
		return true;

	default:
		return false;
	}
}
MCF_TlsExchangeResult MCF_CRT_TlsExchange(void *pTlsKey, intptr_t *pnOldValue, intptr_t nNewValue){
	return TlsExchange(pTlsKey, NULL, pnOldValue, nNewValue);
}

void MCF_CRT_TlsClearAll(){
	TlsMap *const pMap = TlsGetValue(g_dwTlsIndex);
	if(pMap){
		TlsObject *pObject = pMap->pLastInThread;
		while(pObject){
			AcquireSRWLockExclusive(&g_srwLock);
			{
				TlsObject *const pPrevByKey = pObject->pPrevByKey;
				TlsObject *const pNextByKey = pObject->pNextByKey;
				if(pPrevByKey){
					pPrevByKey->pNextByKey = pNextByKey;
				}
				if(pNextByKey){
					pNextByKey->pPrevByKey = pPrevByKey;
				}

				if(pObject->pKey->pLastByKey == pObject){
					pObject->pKey->pLastByKey = pPrevByKey;
				}
			}
			ReleaseSRWLockExclusive(&g_srwLock);

			if(pObject->pKey->pfnCallback){
				(*(pObject->pKey->pfnCallback))(pObject->nValue);
			}

			TlsObject *const pPrevInThread = pObject->pPrevInThread;
			free(pObject);
			pObject = pPrevInThread;
		}

		AcquireSRWLockExclusive(&g_srwLock);
		{
			TlsMap *const pPrevMap = pMap->pPrevMap;
			TlsMap *const pNextMap = pMap->pNextMap;
			if(pPrevMap){
				pPrevMap->pNextMap = pNextMap;
			}
			if(pNextMap){
				pNextMap->pPrevMap = pPrevMap;
			}
			if(g_pLastMap == pMap){
				g_pLastMap = pPrevMap;
			}
		}
		ReleaseSRWLockExclusive(&g_srwLock);

		free(pMap);
		TlsSetValue(g_dwTlsIndex, NULL);
	}

	__MCF_CRT_RunEmutlsDtors();
}

typedef struct tagThreadInitInfo {
	unsigned (*pfnProc)(intptr_t);
	intptr_t nParam;
} ThreadInitInfo;

_Noreturn static
__MCF_C_STDCALL __MCF_HAS_EH_TOP
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
	ExitThread(dwExitCode);
	__builtin_trap();
}

void *MCF_CRT_CreateThread(
	unsigned (*pfnThreadProc)(intptr_t),
	intptr_t nParam,
	bool bSuspended,
	unsigned long *pulThreadId
){
	ThreadInitInfo *const pInitInfo = malloc(sizeof(ThreadInitInfo));
	if(!pInitInfo){
		return NULL;
	}
	pInitInfo->pfnProc	= pfnThreadProc;
	pInitInfo->nParam	= nParam;

	const HANDLE hThread = CreateThread(
		NULL, 0,
		&CRTThreadProc, pInitInfo,
		CREATE_SUSPENDED,
		pulThreadId
	);
	if(!hThread){
		const DWORD dwErrorCode = GetLastError();
		free(pInitInfo);
		SetLastError(dwErrorCode);
		return NULL;
	}
	if(!bSuspended){
		ResumeThread(hThread);
	}
	return hThread;
}
