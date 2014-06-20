// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "thread.h"
#include "mcfwin.h"
#include "fenv.h"
#include "avl_tree.h"
#include "bail.h"
#include "mingw_hacks.h"
#include "_eh_top.h"
#include "../ext/assert.h"
#include "../ext/unref_param.h"
#include <stdlib.h>

// 结构定义。
// 全局的用于描述已分配的 Tls 键的结构。
typedef struct tagTlsKey {
	MCF_AVL_NODE_HEADER vHeader;

	void (*pfnCallback)(intptr_t);
	struct tagTlsObject *pLastByKey;
} TLS_KEY;

static int KeyComparatorNodes(
	const MCF_AVL_NODE_HEADER *pObj1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return (uintptr_t)pObj1 < (uintptr_t)pObj2;
}
static int KeyComparatorNodeKey(
	const MCF_AVL_NODE_HEADER *pObj1,
	intptr_t nKey2
){
	return (uintptr_t)pObj1 < (uintptr_t)nKey2;
}
static int KeyComparatorKeyNode(
	intptr_t nKey1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return (uintptr_t)nKey1 < (uintptr_t)pObj2;
}

// 用于描述每个线程内所有 Tls 对象的结构。
typedef struct tagTlsMap {
	SRWLOCK srwLock;
	MCF_AVL_ROOT pavlObjects;
	struct tagTlsObject *pLastInThread;
	struct tagTlsMap *pPrevMap;
	struct tagTlsMap *pNextMap;
} TLS_MAP;

// 用于描述每个线程内的每个 Tls 对象的结构。
typedef struct tagTlsObject {
	MCF_AVL_NODE_HEADER vHeader;

	struct tagTlsObject *pPrevByKey;
	struct tagTlsObject *pNextByKey;
	struct tagTlsObject *pPrevInThread;
	struct tagTlsObject *pNextInThread;

	struct tagTlsMap *pMap;
	TLS_KEY *pKey;
	intptr_t nValue;
} TLS_OBJECT;

static int ObjectComparatorNodes(
	const MCF_AVL_NODE_HEADER *pObj1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return (uintptr_t)((const TLS_OBJECT *)pObj1)->pKey < (uintptr_t)((const TLS_OBJECT *)pObj2)->pKey;
}
static int ObjectComparatorNodeKey(
	const MCF_AVL_NODE_HEADER *pObj1,
	intptr_t nKey2
){
	return (uintptr_t)((const TLS_OBJECT *)pObj1)->pKey < (uintptr_t)nKey2;
}
static int ObjectComparatorKeyNode(
	intptr_t nKey1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return (uintptr_t)nKey1 < (uintptr_t)((const TLS_OBJECT *)pObj2)->pKey;
}

// 全局变量。
static DWORD		g_dwTlsIndex	= TLS_OUT_OF_INDEXES;

static SRWLOCK		g_srwLock		= SRWLOCK_INIT;
static TLS_MAP *	g_pLastMap		= NULL;
static MCF_AVL_ROOT	g_pavlKeys		= NULL;

bool __MCF_CRT_TlsEnvInit(){
	g_dwTlsIndex = TlsAlloc();
	if(g_dwTlsIndex == TLS_OUT_OF_INDEXES){
		return false;
	}
	return true;
}
void __MCF_CRT_TlsEnvUninit(){
	while(g_pLastMap){
		TLS_OBJECT *pObject = g_pLastMap->pLastInThread;
		while(pObject){
			if(pObject->pKey->pfnCallback){
				(*(pObject->pKey->pfnCallback))(pObject->nValue);
			}

			TLS_OBJECT *const pPrevInThread = pObject->pPrevInThread;
			free(pObject);
			pObject = pPrevInThread;
		}

		TLS_MAP *const pPrevMap = g_pLastMap->pPrevMap;
		free(g_pLastMap);
		g_pLastMap = pPrevMap;
	}
	if(g_pavlKeys){
		TLS_KEY *pCur = (TLS_KEY *)MCF_AvlPrev((MCF_AVL_NODE_HEADER *)g_pavlKeys);
		while(pCur){
			TLS_KEY *const pPrev = (TLS_KEY *)MCF_AvlPrev((MCF_AVL_NODE_HEADER *)pCur);
			free(pCur);
			pCur = pPrev;
		}

		pCur = (TLS_KEY *)MCF_AvlNext((MCF_AVL_NODE_HEADER *)g_pavlKeys);
		while(pCur){
			TLS_KEY *const pNext = (TLS_KEY *)MCF_AvlNext((MCF_AVL_NODE_HEADER *)pCur);
			free(pCur);
			pCur = pNext;
		}

		free(g_pavlKeys);
		g_pavlKeys = NULL;
	}

	TlsFree(g_dwTlsIndex);
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;
}

__attribute__((__stdcall__)) void __MCF_CRT_TlsCallback(void *hModule, unsigned long ulReason, void *pReserved){
	UNREF_PARAM(hModule);
	UNREF_PARAM(pReserved);

	if(ulReason == DLL_THREAD_DETACH){
		TLS_MAP *const pMap = TlsGetValue(g_dwTlsIndex);
		if(pMap){
			TLS_OBJECT *pObject = pMap->pLastInThread;
			while(pObject){
				AcquireSRWLockExclusive(&g_srwLock);
				{
					TLS_OBJECT *const pPrevByKey = pObject->pPrevByKey;
					TLS_OBJECT *const pNextByKey = pObject->pNextByKey;
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

				TLS_OBJECT *const pPrevInThread = pObject->pPrevInThread;
				free(pObject);
				pObject = pPrevInThread;
			}

			AcquireSRWLockExclusive(&g_srwLock);
			{
				TLS_MAP *const pPrevMap = pMap->pPrevMap;
				TLS_MAP *const pNextMap = pMap->pNextMap;
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
}

uintptr_t MCF_CRT_AtThreadExit(void (*pfnProc)(intptr_t), intptr_t nContext){
	const uintptr_t uKey = MCF_CRT_TlsAllocKey(pfnProc);
	if(!uKey){
		return 0;
	}
	if(!MCF_CRT_TlsReset(uKey, nContext)){
		const DWORD dwLastError = GetLastError();
		MCF_CRT_TlsFreeKey(uKey);
		SetLastError(dwLastError);
		return 0;
	}
	return uKey;
}
bool MCF_CRT_RemoveAtThreadExit(uintptr_t uKey){
	return MCF_CRT_TlsFreeKey(uKey);
}

uintptr_t MCF_CRT_TlsAllocKey(void (*pfnCallback)(intptr_t)){
	TLS_KEY *const pKey = malloc(sizeof(TLS_KEY));
	if(!pKey){
		return 0;
	}
	pKey->pfnCallback	= pfnCallback;
	pKey->pLastByKey	= NULL;

	AcquireSRWLockExclusive(&g_srwLock);
	{
		MCF_AvlAttach(
			&g_pavlKeys,
			(MCF_AVL_NODE_HEADER *)pKey,
			&KeyComparatorNodes
		);
	}
	ReleaseSRWLockExclusive(&g_srwLock);

	return (uintptr_t)pKey;
}
bool MCF_CRT_TlsFreeKey(uintptr_t uKey){
	TLS_KEY *pKey;

	AcquireSRWLockExclusive(&g_srwLock);
	{
		pKey = (TLS_KEY *)MCF_AvlFind(
			&g_pavlKeys,
			(intptr_t)uKey,
			&KeyComparatorNodeKey,
			&KeyComparatorKeyNode
		);
		if(pKey){
			MCF_AvlDetach((MCF_AVL_NODE_HEADER *)pKey);
		}
	}
	ReleaseSRWLockExclusive(&g_srwLock);

	if(!pKey){
		return false;
	}

	TLS_OBJECT *pObject = pKey->pLastByKey;
	while(pObject){
		AcquireSRWLockExclusive(&(pObject->pMap->srwLock));
		{
			TLS_OBJECT *const pPrevInThread = pObject->pPrevInThread;
			TLS_OBJECT *const pNextInThread = pObject->pNextInThread;
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

		TLS_OBJECT *const pPrevByKey = pObject->pPrevByKey;
		free(pObject);
		pObject = pPrevByKey;
	}
	free(pKey);

	return true;
}

bool MCF_CRT_TlsGet(uintptr_t uKey, intptr_t *pnValue){
	bool bRet = false;

	TLS_MAP *const pMap = TlsGetValue(g_dwTlsIndex);
	if(pMap){
		AcquireSRWLockShared(&(pMap->srwLock));
		{
			TLS_OBJECT *const pObject = (TLS_OBJECT *)MCF_AvlFind(
				&(pMap->pavlObjects),
				(intptr_t)uKey,
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

static int TlsExchange(uintptr_t uKey, void (**ppfnCallback)(intptr_t), intptr_t *pnOldValue, intptr_t nNewValue){
	TLS_KEY *const pKey = (TLS_KEY *)uKey;

#ifndef NDEBUG
	AcquireSRWLockShared(&g_srwLock);
	{
		TLS_KEY *const pTestKey = (TLS_KEY *)MCF_AvlFind(
			&g_pavlKeys,
			(intptr_t)pKey,
			&KeyComparatorNodeKey,
			&KeyComparatorKeyNode
		);
		ASSERT(pTestKey && (pTestKey == pKey));
	}
	ReleaseSRWLockShared(&g_srwLock);
#endif

	TLS_MAP *pMap = TlsGetValue(g_dwTlsIndex);
	if(!pMap){
		pMap = malloc(sizeof(TLS_MAP));
		if(!pMap){
			return 0;
		}
		const SRWLOCK vLockInit = SRWLOCK_INIT;
		pMap->srwLock		= vLockInit;
		pMap->pavlObjects	= NULL;
		pMap->pLastInThread	= NULL;
		pMap->pNextMap		= NULL;

		AcquireSRWLockExclusive(&g_srwLock);
		{
			TLS_MAP *const pPrevMap = g_pLastMap;
			g_pLastMap = pMap;
			pMap->pPrevMap = pPrevMap;
			if(pPrevMap){
				pPrevMap->pNextMap = pMap;
			}
		}
		ReleaseSRWLockExclusive(&g_srwLock);

		TlsSetValue(g_dwTlsIndex, pMap);
	}

	int nRet;

	AcquireSRWLockExclusive(&(pMap->srwLock));
	{
		TLS_OBJECT *pObject = (TLS_OBJECT *)MCF_AvlFind(
			&(pMap->pavlObjects),
			(intptr_t)pKey,
			&ObjectComparatorNodeKey,
			&ObjectComparatorKeyNode
		);
		if(pObject){
			ASSERT(pObject->pMap == pMap);
			ASSERT(pObject->pKey == pKey);

			if(ppfnCallback){
				*ppfnCallback = pObject->pKey->pfnCallback;
			}
			*pnOldValue = pObject->nValue;

			TLS_OBJECT *const pPrevByKey = pObject->pPrevByKey;
			TLS_OBJECT *const pNextByKey = pObject->pNextByKey;
			if(pPrevByKey){
				pPrevByKey->pNextByKey = pNextByKey;
			}
			if(pNextByKey){
				pNextByKey->pPrevByKey = pPrevByKey;
			}
			if(pKey->pLastByKey == pObject){
				pKey->pLastByKey = pPrevByKey;
			}

			TLS_OBJECT *const pPrevInThread = pObject->pPrevInThread;
			TLS_OBJECT *const pNextInThread = pObject->pNextInThread;
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

			nRet = 1;
		} else {
			ReleaseSRWLockExclusive(&(pMap->srwLock));
			{
				pObject = malloc(sizeof(TLS_OBJECT));
				if(!pObject){
					return false;
				}
				pObject->pMap	= pMap;
				pObject->pKey	= pKey;
				pObject->nValue	= nNewValue;

				MCF_AvlAttach(
					&(pMap->pavlObjects),
					(MCF_AVL_NODE_HEADER *)pObject,
					&ObjectComparatorNodes
				);
			}
			AcquireSRWLockExclusive(&(pMap->srwLock));

			nRet = 2;
		}

		AcquireSRWLockExclusive(&g_srwLock);
		{
			TLS_OBJECT *const pPrevByKey = pKey->pLastByKey;
			pKey->pLastByKey = pObject;
			if(pPrevByKey){
				pPrevByKey->pNextByKey = pObject;
			}
			pObject->pPrevByKey	= pPrevByKey;
			pObject->pNextByKey	= NULL;
		}
		ReleaseSRWLockExclusive(&g_srwLock);

		TLS_OBJECT *const pPrevInThread = pMap->pLastInThread;
		pMap->pLastInThread = pObject;
		if(pPrevInThread){
			pPrevInThread->pNextInThread = pObject;
		}
		pObject->pPrevInThread	= pPrevInThread;
		pObject->pNextInThread	= NULL;
	}
	ReleaseSRWLockExclusive(&(pMap->srwLock));

	return nRet;
}

bool MCF_CRT_TlsReset(uintptr_t uKey, intptr_t nNewValue){
	void (*pfnCallback)(intptr_t) = NULL;
	intptr_t nOldValue = 0;
	switch(TlsExchange(uKey, &pfnCallback, &nOldValue, nNewValue)){
	case 1:
		if(pfnCallback){
			(*pfnCallback)(nOldValue);
		}
	case 2:
		return true;

	default:
		return false;
	}
}
int MCF_CRT_TlsExchange(uintptr_t uKey, intptr_t *pnOldValue, intptr_t nNewValue){
	return TlsExchange(uKey, NULL, pnOldValue, nNewValue);
}

typedef struct tagThreadInitInfo {
	unsigned int (*pfnProc)(intptr_t);
	intptr_t nParam;
} THREAD_INIT_INFO;

extern __attribute__((__stdcall__, __noreturn__)) DWORD CRTThreadProc(LPVOID pParam)
	__asm__("CRTThreadProc");

void *MCF_CRT_CreateThread(
	unsigned int (*pfnThreadProc)(intptr_t),
	intptr_t nParam,
	bool bSuspended,
	unsigned long *pulThreadId
){
	THREAD_INIT_INFO *const pInitInfo = malloc(sizeof(THREAD_INIT_INFO));
	if(!pInitInfo){
		return NULL;
	}
	pInitInfo->pfnProc		= pfnThreadProc;
	pInitInfo->nParam		= nParam;

	const HANDLE hThread = CreateThread(NULL, 0, &CRTThreadProc, pInitInfo, CREATE_SUSPENDED, pulThreadId);
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

#pragma GCC optimize "-fno-function-sections"

static __attribute__((__cdecl__, __used__, __noreturn__)) DWORD AlignedCRTThreadProc(LPVOID pParam){
	DWORD dwExitCode;
	__MCF_EH_TOP_BEGIN
	{
		const THREAD_INIT_INFO vInitInfo = *(THREAD_INIT_INFO *)pParam;
		free(pParam);

		__MCF_CRT_FEnvInit();

		dwExitCode = (*vInitInfo.pfnProc)(vInitInfo.nParam);
	}
	__MCF_EH_TOP_END
	ExitThread(dwExitCode);
	__builtin_trap();
}

__asm__(
	"	.text \n"
	"	.align 16 \n"
	"CRTThreadProc: \n"
#ifdef _WIN64
	"	jmp AlignedCRTThreadProc \n"
#else
	"	mov eax, dword ptr[esp + 4] \n"
	"	and esp, -0x10 \n"
	"	sub esp, 0x10 \n"
	"	mov dword ptr[esp], eax \n"
	"	call _AlignedCRTThreadProc \n"
	"	int3 \n"
#endif
);
