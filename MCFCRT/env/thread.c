// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "thread.h"
#include "fenv.h"
#include "mingw_hacks.h"
#include "avl_tree.h"
#include "lockfree_list.h"
#include "bail.h"
#include <stdlib.h>
#include <windows.h>

typedef struct tagThreadInitInfo {
	unsigned int (*pfnProc)(intptr_t);
	intptr_t nParam;
} THREAD_INIT_INFO;

typedef struct tagAtExitNode {
	void (*pfnProc)(intptr_t);
	intptr_t nContext;

	struct tagAtExitNode *pNext;
} AT_EXIT_NODE;

typedef struct tagTlsObject {
	__MCF_AVL_NODE_HEADER AvlNodeHeader;

	void *pMem;
	void (*pfnDtor)(void *);

	struct tagTlsObject *pPrev;
	struct tagTlsObject *pNext;
#ifndef NDEBUG
	size_t uMemSize;
#endif
} TLS_OBJECT;

typedef struct tagThreadEnv {
	AT_EXIT_NODE *pAtExitHead;

	__MCF_AVL_PROOT pObjectMap;
	TLS_OBJECT *pLastObject;
} THREAD_ENV;

static DWORD g_dwTlsIndex = TLS_OUT_OF_INDEXES;

static DWORD WINAPI CRTThreadProc(LPVOID pParam){
	const THREAD_INIT_INFO ThreadInitInfo = *(const THREAD_INIT_INFO *)pParam;
	free(pParam);

	DWORD dwExitCode;

#define INIT(exp)		if((dwExitCode = (exp)) == ERROR_SUCCESS){ ((void)0)
#define CLEANUP(exp)	(exp); } ((void)0)

	INIT(__MCF_CRT_ThreadInitialize());

	dwExitCode = (*ThreadInitInfo.pfnProc)(ThreadInitInfo.nParam);

	CLEANUP(__MCF_CRT_ThreadUninitialize());

	return dwExitCode;
}

unsigned long __MCF_CRT_TlsEnvInitialize(){
	g_dwTlsIndex = TlsAlloc();
	if(g_dwTlsIndex == TLS_OUT_OF_INDEXES){
		return GetLastError();
	}
	return ERROR_SUCCESS;
}
void __MCF_CRT_TlsEnvUninitialize(){
	TlsFree(g_dwTlsIndex);
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;
}

unsigned long __MCF_CRT_ThreadInitialize(){
	__MCF_CRT_FEnvInitialize();

	THREAD_ENV *const pNewThreadEnv = (THREAD_ENV *)malloc(sizeof(THREAD_ENV));
	if(pNewThreadEnv == NULL){
		return GetLastError();
	}
	pNewThreadEnv->pAtExitHead	= NULL;
	pNewThreadEnv->pObjectMap	= NULL;
	pNewThreadEnv->pLastObject	= NULL;

	TlsSetValue(g_dwTlsIndex, pNewThreadEnv);

	return ERROR_SUCCESS;
}

void __MCF_CRT_ThreadUninitialize(){
	THREAD_ENV *const pThreadEnv = (THREAD_ENV *)TlsGetValue(g_dwTlsIndex);
	if(GetLastError() == ERROR_SUCCESS){
		register AT_EXIT_NODE *pAtExitHead = pThreadEnv->pAtExitHead;
		while(pAtExitHead != NULL){
			AT_EXIT_NODE *const pNext = pAtExitHead->pNext;
			(*pAtExitHead->pfnProc)(pAtExitHead->nContext);
			free(pAtExitHead);
			pAtExitHead = pNext;
		}

		register TLS_OBJECT *pObject = pThreadEnv->pLastObject;
		while(pObject != NULL){
			TLS_OBJECT *const pPrev = pObject->pPrev;
			if(pObject->pfnDtor != NULL){
				(*pObject->pfnDtor)(pObject->pMem);
			}
			free(pObject->pMem);
			free(pObject);
			pObject = pPrev;
		}

		TlsSetValue(g_dwTlsIndex, NULL);
		free(pThreadEnv);
	}
	__MCF_CRT_RunEmutlsThreadDtors();
}

void *__MCF_CreateCRTThread(
	unsigned int (*pfnProc)(intptr_t),
	intptr_t nParam,
	unsigned long ulFlags,
	unsigned long *pulThreadId
){
	THREAD_INIT_INFO *const pThreadInitInfo = (THREAD_INIT_INFO *)malloc(sizeof(THREAD_INIT_INFO));
	if(pThreadInitInfo == NULL){
		return NULL;
	}
	pThreadInitInfo->pfnProc = pfnProc;
	pThreadInitInfo->nParam = nParam;

	const HANDLE hThread = CreateThread(NULL, 0, &CRTThreadProc, pThreadInitInfo, ulFlags, pulThreadId);
	if(hThread == NULL){
		const DWORD dwErrorCode = GetLastError();
		free(pThreadInitInfo);
		SetLastError(dwErrorCode);
	}
	return (void *)hThread;
}

int __MCF_AtCRTThreadExit(
	void (*pfnProc)(intptr_t),
	intptr_t nContext
){
	THREAD_ENV *const pThreadEnv = (THREAD_ENV *)TlsGetValue(g_dwTlsIndex);
	if((pThreadEnv == NULL) || (GetLastError() != ERROR_SUCCESS)){
		return -1;
	}

	AT_EXIT_NODE *const pNewNode = (AT_EXIT_NODE *)malloc(sizeof(AT_EXIT_NODE));
	if(pNewNode == NULL){
		return -1;
	}

	pNewNode->pfnProc	= pfnProc;
	pNewNode->nContext	= nContext;
	pNewNode->pNext		= pThreadEnv->pAtExitHead;

	pThreadEnv->pAtExitHead	= pNewNode;

	return 0;
}

void *__MCF_CRT_RetrieveTls(
	intptr_t nKey,
	size_t uSizeToAlloc,
	void (*pfnConstructor)(void *, intptr_t),
	intptr_t nParam,
	void (*pfnDestructor)(void *)
){
	THREAD_ENV *const pThreadEnv = (THREAD_ENV *)TlsGetValue(g_dwTlsIndex);
	if((pThreadEnv == NULL) || (GetLastError() != ERROR_SUCCESS)){
		return NULL;
	}

	TLS_OBJECT *pObject = (TLS_OBJECT *)__MCF_AVLFind(pThreadEnv->pObjectMap, nKey);
	if(pObject == NULL){
		pObject = (TLS_OBJECT *)malloc(sizeof(TLS_OBJECT));
		if(pObject == NULL){
			return NULL;
		}

		void *const pMem = malloc((uSizeToAlloc == 0) ? 1 : uSizeToAlloc);
		if(pMem == NULL){
			free(pObject);
			return NULL;
		}
		if(pfnConstructor != NULL){
			(*pfnConstructor)(pMem, nParam);
		}

		pObject->pMem = pMem;
		pObject->pfnDtor = pfnDestructor;

		TLS_OBJECT *const pPrev = pThreadEnv->pLastObject;

		pObject->pPrev = pPrev;
		pObject->pNext = NULL;

#ifndef NDEBUG
		pObject->uMemSize = uSizeToAlloc;
#endif

		if(pPrev != NULL){
			pPrev->pNext = pObject;
		}
		pThreadEnv->pLastObject = pObject;

		__MCF_AVLAttach(&pThreadEnv->pObjectMap, nKey, (__MCF_AVL_NODE_HEADER *)pObject);
	}
#ifndef NDEBUG
	if(pObject->uMemSize != uSizeToAlloc){
		__MCF_BailF(
			L"__MCF_CRT_RetrieveTls() 失败：两次试图使用相同的键获得 TLS，但指定的大小不一致。\n\n"
			"键：0x%p\n"
			"该 TLS 创建时的大小：0x%p\n"
			"本次调用指定的大小 ：0x%p\n",
			(void *)nKey,
			(void *)pObject->uMemSize,
			(void *)uSizeToAlloc
		);
	}
#endif
	return pObject->pMem;
}
void __MCF_CRT_DeleteTls(
	intptr_t nKey
){
	THREAD_ENV *const pThreadEnv = (THREAD_ENV *)TlsGetValue(g_dwTlsIndex);
	if((pThreadEnv == NULL) || (GetLastError() != ERROR_SUCCESS)){
		return;
	}

	TLS_OBJECT *pObject = (TLS_OBJECT *)__MCF_AVLFind(pThreadEnv->pObjectMap, nKey);
	if(pObject != NULL){
		TLS_OBJECT *const pPrev = pObject->pPrev;
		TLS_OBJECT *const pNext = pObject->pNext;
		if(pPrev != NULL){
			pPrev->pNext = pNext;
		}
		if(pNext != NULL){
			pNext->pPrev = pPrev;
		}
		if(pThreadEnv->pLastObject == pObject){
			pThreadEnv->pLastObject = pPrev;
		}
		if(pObject->pfnDtor != NULL){
			(*pObject->pfnDtor)(pObject->pMem);
		}
		free(pObject->pMem);
		free(pObject);
	}
}
