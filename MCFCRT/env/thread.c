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
#include <stdlib.h>

typedef struct tagTlsObject {
	MCF_AVL_NODE_HEADER vHeader;
	struct tagTlsObject *pPrev;
	struct tagTlsObject *pNext;

	intptr_t nKey;
	intptr_t nValue;
} TLS_OBJECT;

static int TlsObjectComparatorNodes(
	const MCF_AVL_NODE_HEADER *pObj1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return ((const TLS_OBJECT *)pObj1)->nKey < ((const TLS_OBJECT *)pObj2)->nKey;
}
static int TlsObjectComparatorNodeKey(
	const MCF_AVL_NODE_HEADER *pObj1,
	intptr_t nKey2
){
	return ((const TLS_OBJECT *)pObj1)->nKey < nKey2;
}
static int TlsObjectComparatorKeyNode(
	intptr_t nKey1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return nKey1 < ((const TLS_OBJECT *)pObj2)->nKey;
}

typedef struct tagTlsObjectMap {
	MCF_AVL_ROOT pavlRoot;
	TLS_OBJECT *pLastObject;
} TLS_OBJECT_MAP;

static DWORD g_dwFlsIndex = FLS_OUT_OF_INDEXES;

static __attribute__((__stdcall__)) void FlsCleanupProc(LPVOID pFlsData){
	TLS_OBJECT_MAP *const pObjectMap = pFlsData;
	if(pObjectMap){
		free(pObjectMap);
	}

	__MCF_CRT_RunEmutlsDtors();
}

bool __MCF_CRT_TlsEnvInit(){
	g_dwFlsIndex = FlsAlloc(&FlsCleanupProc);
	if(g_dwFlsIndex == FLS_OUT_OF_INDEXES){
		return false;
	}
	return true;
}
void __MCF_CRT_TlsEnvUninit(){
	FlsFree(g_dwFlsIndex);
	g_dwFlsIndex = FLS_OUT_OF_INDEXES;
}

bool __MCF_CRT_ThreadInit(){
	TLS_OBJECT_MAP *const pObjectMap = malloc(sizeof(TLS_OBJECT_MAP));
	if(!pObjectMap){
		return false;
	}
	pObjectMap->pavlRoot = NULL;
	pObjectMap->pLastObject = NULL;

	if(!FlsSetValue(g_dwFlsIndex, pObjectMap)){
		free(pObjectMap);
		return false;
	}
	return true;
}
void __MCF_CRT_ThreadUninit(){
	const LPVOID pFlsData = FlsGetValue(g_dwFlsIndex);
	FlsCleanupProc(pFlsData);
	FlsSetValue(g_dwFlsIndex, NULL);
}

typedef struct tagThreadInitInfo {
	unsigned int (*pfnProc)(intptr_t);
	intptr_t nParam;

	HANDLE hDoneInit;
	bool bInitSucceeds;
	DWORD dwErrorCode;
} THREAD_INIT_INFO;

extern __attribute__((__stdcall__, __noreturn__)) DWORD CRTThreadProc(LPVOID pParam)
	__asm__("CRTThreadProc");

void *MCF_CRT_CreateThread(
	unsigned int (*pfnThreadProc)(intptr_t),
	intptr_t nParam,
	bool bSuspended,
	unsigned long *pulThreadId
){
	THREAD_INIT_INFO vInitInfo;
	vInitInfo.pfnProc	= pfnThreadProc;
	vInitInfo.nParam	= nParam;
	vInitInfo.hDoneInit	= CreateEvent(NULL, TRUE, FALSE, NULL);
	if(!vInitInfo.hDoneInit){
		return NULL;
	}

	const HANDLE hThread = CreateThread(NULL, 0, &CRTThreadProc, &vInitInfo, 0, pulThreadId);
	if(!hThread){
		const DWORD dwErrorCode = GetLastError();
		CloseHandle(vInitInfo.hDoneInit);
		SetLastError(dwErrorCode);
		return NULL;
	}
	WaitForSingleObject(vInitInfo.hDoneInit, INFINITE);
	CloseHandle(vInitInfo.hDoneInit);

	if(!vInitInfo.bInitSucceeds){
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		SetLastError(vInitInfo.dwErrorCode);
		return NULL;
	}

	if(!bSuspended){
		ResumeThread(hThread);
	}
	return hThread;
}

intptr_t MCF_CRT_AtThreadExit(void (*pfnProc)(intptr_t), intptr_t nContext){
	return MCF_CRT_TlsAllocKey(pfnProc, nContext);
}
bool MCF_CRT_RemoveAtThreadExit(intptr_t nKey){
	return MCF_CRT_TlsFreeKey(nKey);
}

intptr_t MCF_CRT_TlsAllocKey(void (*pfnTlsCallback)(intptr_t), intptr_t nInitValue){
}
bool MCF_CRT_TlsFreeKey(intptr_t nKey){
}

bool MCF_CRT_TlsGetValue(intptr_t nKey, intptr_t *pnValue){
/*	TLS_OBJECT_MAP *const pObjectMap = FlsGetValue(g_dwFlsIndex);
	if(!pObjectMap){
		return false;
	}
	TLS_OBJECT *const pObject = (TLS_OBJECT *)MCF_AvlFind(
		&(pObjectMap->pavlRoot),
		nKey,
		&TlsObjectComparatorNodeKey,
		&TlsObjectComparatorKeyNode
	);
	if(pObject){
		*pnValue = pObject->nValue;
	} else {
		*pnValue = 0;
	}
	return true;*/
}
bool MCF_CRT_TlsSetValue(intptr_t nKey, intptr_t *pnOldValue, intptr_t nNewValue){
/*	TLS_OBJECT_MAP *const pObjectMap = FlsGetValue(g_dwFlsIndex);
	if(!pObjectMap){
		return false;
	}
	TLS_OBJECT *const pObject = (TLS_OBJECT *)MCF_AvlFind(
		&(pObjectMap->pavlRoot),
		nKey,
		&TlsObjectComparatorNodeKey,
		&TlsObjectComparatorKeyNode
	);
	if(pObject){
		*pnOldValue = pObject->nValue;
		pObject->nValue = nNewValue;
	} else {
		*pnOldValue = 0;

		TLS_OBJECT *const pNewObject = malloc(sizeof(TLS_OBJECT));
		if(!pNewObject){
			return false;
		}

		TLS_OBJECT *const pPrev = pObjectMap->pLastObject;
		if(pPrev){
			pPrev->pNext = pNewObject;
		}
		pObjectMap->pLastObject = pNewObject;

		pNewObject->pPrev	= pPrev;
		pNewObject->pNext	= NULL;
		pNewObject->nKey	= nKey;
		pNewObject->nValue	= nNewValue;

		MCF_AvlAttach(
			&(pObjectMap->pavlRoot),
			pNewObject,
			&TlsObjectComparatorNodes
		);
	}
	return true;
*/}
/*
intptr_t MCF_CRT_TlsAllocKey(void (*pfnTlsCallback)(intptr_t)){
	TLS_OBJECT_MAP *const pObjectMap = FlsGetValue(g_dwFlsIndex);
	if(!pObjectMap){
		return -1;
	}


	TLS_OBJECT *const pObject = malloc(sizeof(TLS_OBJECT));
	if(!pObject){
		return -1;
	}





typedef struct tagTlsObject {
	MCF_AVL_NODE_HEADER vHeader;
	struct tagTlsObject *pPrev;
	struct tagTlsObject *pNext;

	intptr_t nKey;
	intptr_t nValue;
	void (*pfnTlsCallback)(intptr_t);
} TLS_OBJECT;

typedef struct tagTlsObjectMap {
	MCF_AVL_ROOT pavlRoot;
	TLS_OBJECT *pLastObject;
} TLS_OBJECT_MAP;

	return -1;
}
void MCF_CRT_TlsFreeKey(intptr_t nKey){
}

bool MCF_CRT_TlsGetValue(
	intptr_t nKey,
	intptr_t *pValue
){
	TLS_OBJECT_MAP *const pObjectMap = FlsGetValue(g_dwFlsIndex);
	if(!pObjectMap){
		return false;
	}
	TLS_OBJECT *const pObject = (TLS_OBJECT *)MCF_AvlFind(
		&(pObjectMap->pavlRoot),
		nKey,
		&TlsObjectComparatorNodeKey,
		&TlsObjectComparatorKeyNode
	);
	if(!pObject){
		return false;
	}
	*pValue = pObject->nValue;
	return true;
}
bool MCF_CRT_TlsSetValue(
	intptr_t nKey,
	intptr_t *pOldValue,
	intptr_t nNewValue
){
	TLS_OBJECT_MAP *const pObjectMap = FlsGetValue(g_dwFlsIndex);
	if(!pObjectMap){
		return false;
	}
	TLS_OBJECT *const pObject = (TLS_OBJECT *)MCF_AvlFind(
		&(pObjectMap->pavlRoot),
		nKey,
		&TlsObjectComparatorNodeKey,
		&TlsObjectComparatorKeyNode
	);
	if(!pObject){
		return false;
	}
	*pOldValue = pObject->nValue;
	pObject->nValue = nNewValue;
	return true;
}

intptr_t MCF_CRT_TlsGetValue(intptr_t nKey){
}
intptr_t MCF_CRT_TlsSetValue(intptr_t nKey, intptr_t nNewValue){
}




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
	union {
		void *pMem;
		unsigned char abySmall[4 * sizeof(void *)];
	};
	void (*pfnDtor)(void *);
	size_t uMemSize;
} TLS_OBJECT;

static int TlsObjectComparatorNodes(
	const MCF_AVL_NODE_HEADER *pObj1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return ((const TLS_OBJECT *)pObj1)->nKey < ((const TLS_OBJECT *)pObj2)->nKey;
}
static int TlsObjectComparatorNodeKey(
	const MCF_AVL_NODE_HEADER *pObj1,
	intptr_t nKey2
){
	return ((const TLS_OBJECT *)pObj1)->nKey < nKey2;
}
static int TlsObjectComparatorKeyNode(
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

extern __attribute__((__stdcall__, __noreturn__)) DWORD CRTThreadProc(LPVOID pParam)
	__asm__("CRTThreadProc");

unsigned long MCF_CRT_TlsEnvInitialize(){
	g_dwTlsIndex = TlsAlloc();
	if(g_dwTlsIndex == TLS_OUT_OF_INDEXES){
		return GetLastError();
	}

	return ERROR_SUCCESS;
}
void MCF_CRT_TlsEnvUninitialize(){
	TlsFree(g_dwTlsIndex);
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;
}

unsigned long __MCF_CRT_ThreadInitialize(){
	__MCF_CRT_FEnvInitialize();

	THREAD_ENV *const pThreadEnv = malloc(sizeof(THREAD_ENV));
	if(!pThreadEnv){
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	pThreadEnv->pAtExitHead	= NULL;
	pThreadEnv->avlObjects	= NULL;
	pThreadEnv->pLastObject	= NULL;

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
		if(pObject->uMemSize <= sizeof(pObject->abySmall)){
			if(pObject->pfnDtor){
				(*pObject->pfnDtor)(pObject->abySmall);
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
		&TlsObjectComparatorNodeKey,
		&TlsObjectComparatorKeyNode
	);
	if(!pObject){
		pObject = malloc(sizeof(TLS_OBJECT));
		if(!pObject){
			return NULL;
		}
		if(uSizeToAlloc <= sizeof(pObject->abySmall)){
			if(pfnConstructor && !(*pfnConstructor)(pObject->abySmall, nParam)){
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
			&TlsObjectComparatorNodes
		);
	} else if(pObject->uMemSize != uSizeToAlloc){
		MCF_CRT_BailF(
			L"MCF_CRT_RetrieveTls() 失败：两次试图使用相同的键获得 TLS，但指定的大小不一致。\n\n"
			"键：%0*zX\n"
			"该 TLS 创建时的大小：%0*zX\n"
			"本次调用指定的大小 ：%0*zX\n",
			sizeof(size_t) * 2, (size_t)nKey,
			sizeof(size_t) * 2, pObject->uMemSize,
			sizeof(size_t) * 2, uSizeToAlloc
		);
	}

	if(pObject->uMemSize <= sizeof(pObject->abySmall)){
		return pObject->abySmall;
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
		&TlsObjectComparatorNodeKey,
		&TlsObjectComparatorKeyNode
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
		if(pObject->uMemSize <= sizeof(pObject->abySmall)){
			if(pObject->pfnDtor){
				(*pObject->pfnDtor)(pObject->abySmall);
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
*/
#pragma GCC optimize "-fno-function-sections"

static __attribute__((__cdecl__, __used__, __noreturn__)) DWORD AlignedCRTThreadProc(LPVOID pParam){
	THREAD_INIT_INFO *const pInitInfo = (THREAD_INIT_INFO *)pParam;
	unsigned int (*const pfnProc)(intptr_t) = pInitInfo->pfnProc;
	const intptr_t nParam = pInitInfo->nParam;

	DWORD dwExitCode;

	__MCF_EH_TOP_BEGIN
	{
		pInitInfo->bInitSucceeds = true;

		if(!__MCF_CRT_ThreadInit()){
			dwExitCode = GetLastError();

			pInitInfo->bInitSucceeds = false;
			pInitInfo->dwErrorCode = dwExitCode;
		}
		SetEvent(pInitInfo->hDoneInit);

		if(pInitInfo->bInitSucceeds){
			SuspendThread(GetCurrentThread());

			dwExitCode = (*pfnProc)(nParam);
		}
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
