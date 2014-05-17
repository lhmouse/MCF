// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "thread.h"
#include "fenv.h"
#include "mingw_hacks.h"
#include "avl_tree.h"
#include "bail.h"
#include <stdlib.h>
#include <windows.h>

#ifdef _WIN64
#	define UINTPTR_FORMAT	"0x%016zX"
#else
#	define UINTPTR_FORMAT	"0x%08zX"
#endif

typedef struct tagThreadInitInfo {
	unsigned int (*pfnProc)(intptr_t);
	intptr_t nParam;
} THREAD_INIT_INFO;

typedef struct tagAtExitNode {
	struct tagAtExitNode *pNext;

	void (*pfnProc)(intptr_t);
	intptr_t nContext;
} AT_EXIT_NODE;

typedef struct tagTlsObject {
	MCF_AVL_NODE_HEADER vHeader;
	struct tagTlsObject *pPrev;
	struct tagTlsObject *pNext;

	intptr_t nKey;
	void *pMem;
	void (*pfnDtor)(void *);
	size_t uMemSize;
} TLS_OBJECT;

static int TlsObjectComparerNodes(
	const MCF_AVL_NODE_HEADER *pObj1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return ((const TLS_OBJECT *)pObj1)->nKey < ((const TLS_OBJECT *)pObj2)->nKey;
}
static int TlsObjectComparerNodeKey(
	const MCF_AVL_NODE_HEADER *pObj1,
	intptr_t nKey2
){
	return ((const TLS_OBJECT *)pObj1)->nKey < nKey2;
}
static int TlsObjectComparerKeyNode(
	intptr_t nKey1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return nKey1 < ((const TLS_OBJECT *)pObj2)->nKey;
}

typedef struct tagThreadEnv {
	AT_EXIT_NODE *pAtExitHead;
	MCF_AVL_ROOT avlObjects;
	TLS_OBJECT *pLastObject;
} THREAD_ENV;

static DWORD g_dwTlsIndex = TLS_OUT_OF_INDEXES;

static __attribute__((__cdecl__, __used__, __noreturn__)) DWORD AlignedCRTThreadProc(LPVOID pParam){
	const THREAD_INIT_INFO ThreadInitInfo = *(const THREAD_INIT_INFO *)pParam;
	free(pParam);

	DWORD dwExitCode;

#define INIT(exp)		if((dwExitCode = (exp)) == ERROR_SUCCESS){ ((void)0)
#define CLEANUP(exp)	(exp); } ((void)0)

	INIT(__MCF_CRT_ThreadInitialize());

	dwExitCode = (*ThreadInitInfo.pfnProc)(ThreadInitInfo.nParam);

	CLEANUP(__MCF_CRT_ThreadUninitialize());

	ExitThread(dwExitCode);
	__builtin_trap();
}

extern __attribute__((__stdcall__, __noreturn__)) DWORD CRTThreadProc(LPVOID pParam)
	__asm__("CRTThreadProc");
__asm__(
	"	.text \n"
	"	.align 16 \n"
	"CRTThreadProc: \n"
#ifdef _WIN64
	"	and rsp, -0x10 \n"
	"	sub rsp, 0x10 \n"
	"	call AlignedCRTThreadProc \n"
#else
	"	mov eax, dword ptr[esp + 4] \n"
	"	and esp, -0x10 \n"
	"	sub esp, 0x10 \n"
	"	mov dword ptr[esp], eax \n"
	"	call _AlignedCRTThreadProc \n"
#endif
);

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

	THREAD_ENV *const pThreadEnv = malloc(sizeof(THREAD_ENV));
	if(!pThreadEnv){
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	pThreadEnv->pAtExitHead		= NULL;
	pThreadEnv->avlObjects		= NULL;
	pThreadEnv->pLastObject		= NULL;

	if(!TlsSetValue(g_dwTlsIndex, pThreadEnv)){
		const unsigned long ulErrorCode = GetLastError();
		free(pThreadEnv);
		return ulErrorCode;
	}

	return ERROR_SUCCESS;
}

void __MCF_CRT_ThreadUninitialize(){
	THREAD_ENV *const pThreadEnv = TlsGetValue(g_dwTlsIndex);
	if(GetLastError() != ERROR_SUCCESS){
		return;
	}

	register AT_EXIT_NODE *pAtExitHead = pThreadEnv->pAtExitHead;
	while(pAtExitHead){
		AT_EXIT_NODE *const pNext = pAtExitHead->pNext;
		(*pAtExitHead->pfnProc)(pAtExitHead->nContext);
		free(pAtExitHead);
		pAtExitHead = pNext;
	}

	register TLS_OBJECT *pObject = pThreadEnv->pLastObject;
	while(pObject){
		TLS_OBJECT *const pPrev = pObject->pPrev;
		if(pObject->uMemSize <= sizeof(void *)){
			if(pObject->pfnDtor){
				(*pObject->pfnDtor)(&(pObject->pMem));
			}
		} else {
			if(pObject->pfnDtor){
				(*pObject->pfnDtor)(pObject->pMem);
			}
			free(pObject->pMem);
		}
		free(pObject);
		pObject = pPrev;
	}

	TlsSetValue(g_dwTlsIndex, NULL);
	free(pThreadEnv);

	__MCF_CRT_RunEmutlsThreadDtors();
}

void *MCF_CRT_CreateThread(
	unsigned int (*pfnProc)(intptr_t),
	intptr_t nParam,
	unsigned long ulFlags,
	unsigned long *pulThreadId
){
	THREAD_INIT_INFO *const pThreadInitInfo = malloc(sizeof(THREAD_INIT_INFO));
	if(!pThreadInitInfo){
		return NULL;
	}
	pThreadInitInfo->pfnProc = pfnProc;
	pThreadInitInfo->nParam = nParam;

	const HANDLE hThread = CreateThread(NULL, 0, &CRTThreadProc, pThreadInitInfo, ulFlags, pulThreadId);
	if(!hThread){
		const DWORD dwErrorCode = GetLastError();
		free(pThreadInitInfo);
		SetLastError(dwErrorCode);
	}
	return (void *)hThread;
}

int MCF_CRT_AtThreadExit(
	void (*pfnProc)(intptr_t),
	intptr_t nContext
){
	THREAD_ENV *const pThreadEnv = TlsGetValue(g_dwTlsIndex);
	if((GetLastError() != ERROR_SUCCESS) || !pThreadEnv){
		return -1;
	}

	AT_EXIT_NODE *const pNewNode = malloc(sizeof(AT_EXIT_NODE));
	if(!pNewNode){
		return -1;
	}

	pNewNode->pfnProc	= pfnProc;
	pNewNode->nContext	= nContext;
	pNewNode->pNext		= pThreadEnv->pAtExitHead;

	pThreadEnv->pAtExitHead	= pNewNode;

	return 0;
}

void *MCF_CRT_RetrieveTls(
	intptr_t nKey,
	size_t uSizeToAlloc,
	int (*pfnConstructor)(void *, intptr_t),
	intptr_t nParam,
	void (*pfnDestructor)(void *)
){
	THREAD_ENV *const pThreadEnv = TlsGetValue(g_dwTlsIndex);
	if((GetLastError() != ERROR_SUCCESS) || !pThreadEnv){
		return NULL;
	}

	TLS_OBJECT *pObject = (TLS_OBJECT *)MCF_AvlFind(
		&(pThreadEnv->avlObjects),
		nKey,
		&TlsObjectComparerNodeKey,
		&TlsObjectComparerKeyNode
	);
	if(!pObject){
		pObject = malloc(sizeof(TLS_OBJECT));
		if(!pObject){
			return NULL;
		}
		if(uSizeToAlloc <= sizeof(void *)){
			if(pfnConstructor && !(*pfnConstructor)(&(pObject->pMem), nParam)){
				free(pObject);
				return NULL;
			}
		} else {
			void *const pMem = malloc(uSizeToAlloc);
			if(!pMem){
				free(pObject);
				return NULL;
			}
			if(pfnConstructor && !(*pfnConstructor)(pMem, nParam)){
				free(pMem);
				free(pObject);
				return NULL;
			}
			pObject->pMem = pMem;
		}

		TLS_OBJECT *const pPrev = pThreadEnv->pLastObject;
		pObject->pPrev = pPrev;
		pObject->pNext = NULL;
		if(pPrev){
			pPrev->pNext = pObject;
		}

		pObject->nKey = nKey;
		pObject->pfnDtor = pfnDestructor;
		pObject->uMemSize = uSizeToAlloc;

		pThreadEnv->pLastObject = pObject;

		MCF_AvlAttach(
			&(pThreadEnv->avlObjects),
			(MCF_AVL_NODE_HEADER *)pObject,
			&TlsObjectComparerNodes
		);
	} else if(pObject->uMemSize != uSizeToAlloc){
		MCF_CRT_BailF(
			L"MCF_CRT_RetrieveTls() 失败：两次试图使用相同的键获得 TLS，但指定的大小不一致。\n\n"
			"键：" UINTPTR_FORMAT "\n"
			"该 TLS 创建时的大小：" UINTPTR_FORMAT "\n"
			"本次调用指定的大小 ：" UINTPTR_FORMAT "\n",
			(uintptr_t)nKey,
			(uintptr_t)pObject->uMemSize,
			(uintptr_t)uSizeToAlloc
		);
	}

	if(pObject->uMemSize <= sizeof(void *)){
		return &(pObject->pMem);
	} else {
		return pObject->pMem;
	}
}
void MCF_CRT_DeleteTls(
	intptr_t nKey
){
	THREAD_ENV *const pThreadEnv = TlsGetValue(g_dwTlsIndex);
	if((GetLastError() != ERROR_SUCCESS) || !pThreadEnv){
		return;
	}

	TLS_OBJECT *pObject = (TLS_OBJECT *)MCF_AvlFind(
		&(pThreadEnv->avlObjects),
		nKey,
		&TlsObjectComparerNodeKey,
		&TlsObjectComparerKeyNode
	);
	if(pObject){
		TLS_OBJECT *const pPrev = pObject->pPrev;
		TLS_OBJECT *const pNext = pObject->pNext;
		if(pPrev){
			pPrev->pNext = pNext;
		}
		if(pNext){
			pNext->pPrev = pPrev;
		}
		if(pThreadEnv->pLastObject == pObject){
			pThreadEnv->pLastObject = pPrev;
		}
		if(pObject->uMemSize <= sizeof(void *)){
			if(pObject->pfnDtor){
				(*pObject->pfnDtor)(&(pObject->pMem));
			}
		} else {
			if(pObject->pfnDtor){
				(*pObject->pfnDtor)(pObject->pMem);
			}
			free(pObject->pMem);
		}
		free(pObject);
	}
}
