// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "thread.h"
#include "fenv.h"
#include "daemon.h"
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
	intptr_t nKey;
	void *pMem;
	void (*pfnDestructor)(void *);

	struct tagTlsObject *pNext;
} TLS_OBJECT;

typedef struct tagThreadEnv {
	AT_EXIT_NODE *pAtExitHead;
	TLS_OBJECT *pTlsObjectHead;
} THREAD_ENV;

static DWORD g_dwTlsIndex = TLS_OUT_OF_INDEXES;

static DWORD WINAPI CRTThreadProc(LPVOID pParam){
	const THREAD_INIT_INFO *const pThreadInitInfo = (const THREAD_INIT_INFO *)pParam;
	__MCF_CRTThreadInitialize();
	const DWORD dwExitCode = (*pThreadInitInfo->pfnProc)(pThreadInitInfo->nParam);
	__MCF_CRTThreadUninitialize();
	return dwExitCode;
}

__MCF_CRT_EXTERN void __MCF_CRTTlsEnvInitialize(){
	g_dwTlsIndex = TlsAlloc();
	if(g_dwTlsIndex == TLS_OUT_OF_INDEXES){
		__MCF_Bail(L"__MCF_CRTTlsEnvInitialize() 失败：动态 TLS 数量超过限制。");
	}
}
__MCF_CRT_EXTERN void __MCF_CRTTlsEnvUninitialize(){
	TlsFree(g_dwTlsIndex);
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;
}

__MCF_CRT_EXTERN void __MCF_CRTThreadInitialize(){
	__MCF_CRTFEnvInitialize();

	THREAD_ENV *const pNewThreadEnv = (THREAD_ENV *)malloc(sizeof(THREAD_ENV));
	if(pNewThreadEnv == NULL){
		__MCF_Bail(L"__MCF_CRTThreadInitialize() 失败：内存不足。");
	}

	pNewThreadEnv->pAtExitHead		= NULL;
	pNewThreadEnv->pTlsObjectHead	= NULL;

	TlsSetValue(g_dwTlsIndex, pNewThreadEnv);
}
__MCF_CRT_EXTERN void __MCF_CRTThreadUninitialize(){
	THREAD_ENV *const pThreadEnv = (THREAD_ENV *)TlsGetValue(g_dwTlsIndex);
	if(GetLastError() != ERROR_SUCCESS){
		__MCF_Bail(L"__MCF_CRTThreadUninitialize() 失败：TlsGetValue() 错误。");
	}

	register AT_EXIT_NODE *pAtExitHead = pThreadEnv->pAtExitHead;
	while(pAtExitHead != NULL){
		AT_EXIT_NODE *const pNext = pAtExitHead->pNext;
		(*pAtExitHead->pfnProc)(pAtExitHead->nContext);
		free(pAtExitHead);
		pAtExitHead = pNext;
	}

	register TLS_OBJECT *pTlsObjectHead = pThreadEnv->pTlsObjectHead;
	while(pTlsObjectHead != NULL){
		TLS_OBJECT *const pNext = pTlsObjectHead->pNext;
		if(pTlsObjectHead->pfnDestructor != NULL){
			(*pTlsObjectHead->pfnDestructor)(pTlsObjectHead->pMem);
		}
		free(pTlsObjectHead->pMem);
		free(pTlsObjectHead);
		pTlsObjectHead = pNext;
	}

	TlsSetValue(g_dwTlsIndex, NULL);
	free(pThreadEnv);

	__MCF_CRTFEnvUninitialize();
}

__MCF_CRT_EXTERN void *__MCF_CreateCRTThread(
	unsigned int (*pfnProc)(intptr_t),
	intptr_t nParam,
	unsigned long ulFlags,
	unsigned long *pulThreadID
){
	THREAD_INIT_INFO ThreadInitInfo;
	ThreadInitInfo.pfnProc	= pfnProc;
	ThreadInitInfo.nParam	= nParam;
	return (void *)CreateThread(NULL, 0, &CRTThreadProc, &ThreadInitInfo, ulFlags, pulThreadID);
}

__MCF_CRT_EXTERN int __MCF_AtCRTThreadExit(
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

	pNewNode->pfnProc		= pfnProc;
	pNewNode->nContext		= nContext;
	pNewNode->pNext			= pThreadEnv->pAtExitHead;

	pThreadEnv->pAtExitHead	= pNewNode;

	return 0;
}

__MCF_CRT_EXTERN void *__MCF_CRTRetrieveTls(
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

	TLS_OBJECT *const pNewObject = (TLS_OBJECT *)malloc(sizeof(TLS_OBJECT));
	if(pNewObject == NULL){
		return NULL;
	}

	void *const pMem = malloc((uSizeToAlloc == 0) ? 1 : uSizeToAlloc);
	if(pMem == NULL){
		free(pNewObject);
		return NULL;
	}
	if(pfnConstructor != NULL){
		(*pfnConstructor)(pMem, nParam);
	}

	pNewObject->nKey			= nKey;
	pNewObject->pMem			= pMem;
	pNewObject->pfnDestructor	= pfnDestructor;
	pNewObject->pNext			= pThreadEnv->pTlsObjectHead;

	pThreadEnv->pTlsObjectHead	= pNewObject;

	return pMem;
}
__MCF_CRT_EXTERN void __MCF_CRTDeleteTls(
	intptr_t nKey
){
	THREAD_ENV *const pThreadEnv = (THREAD_ENV *)TlsGetValue(g_dwTlsIndex);
	if((pThreadEnv == NULL) || (GetLastError() != ERROR_SUCCESS)){
		return;
	}

	TLS_OBJECT **ppCur = &pThreadEnv->pTlsObjectHead;
	while(*ppCur != NULL){
		if((*ppCur)->nKey == nKey){
			TLS_OBJECT *const pNext = (*ppCur)->pNext;
			if((*ppCur)->pfnDestructor != NULL){
				(*(*ppCur)->pfnDestructor)((*ppCur)->pMem);
			}
			free((*ppCur)->pMem);
			free(*ppCur);
			*ppCur = pNext;
		} else {
			ppCur = &(*ppCur)->pNext;
		}
	}
}
