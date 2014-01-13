// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "MCFCRT.h"
#include <stdlib.h>
#include <windows.h>

// ld 自动添加此符号。
#ifdef __amd64__
#	define IMAGE_BASE __image_base__
#else
#	define IMAGE_BASE _image_base__
#endif
extern char IMAGE_BASE;

extern void __cdecl __main();

typedef struct tagAtExitNode {
	void (__cdecl *pfnProc)(intptr_t);
	intptr_t nContext;
	struct tagAtExitNode *pNext;
} AT_EXIT_NODE;

static AT_EXIT_NODE *volatile g_pAtExitHead = NULL;

static void PumpAtExits(){
	for(;;){
		AT_EXIT_NODE *pNode;
		do {
			pNode = g_pAtExitHead;
			if(pNode == NULL){
				return;
			}
		} while(!__sync_bool_compare_and_swap(&g_pAtExitHead, pNode, pNode->pNext));

		(*pNode->pfnProc)(pNode->nContext);
		free(pNode);
	}
}

__MCF_CRT_EXTERN unsigned long __MCF_CRT_Begin(){
	DWORD dwRet;
	if((dwRet = __MCF_CRT_HeapInitialize()) == ERROR_SUCCESS){
		if((dwRet = __MCF_CRT_TlsEnvInitialize()) == ERROR_SUCCESS){
			__main();
			return ERROR_SUCCESS;
		}
		__MCF_CRT_HeapUninitialize();
	}
	return dwRet;
}
__MCF_CRT_EXTERN void __MCF_CRT_End(){
	PumpAtExits();

	__MCF_CRT_TlsEnvUninitialize();
	__MCF_CRT_HeapUninitialize();
}

__MCF_CRT_EXTERN void *__MCF_GetModuleBase(){
	return &IMAGE_BASE;
}
__MCF_CRT_EXTERN int __MCF_AtCRTEnd(void (__cdecl *pfnProc)(intptr_t), intptr_t nContext){
	AT_EXIT_NODE *const pNode = (AT_EXIT_NODE *)malloc(sizeof(AT_EXIT_NODE));
	if(pNode == NULL){
		return -1;
	}
	pNode->pfnProc = pfnProc;
	pNode->nContext = nContext;

	AT_EXIT_NODE *pOldHead;
	do {
		pOldHead = g_pAtExitHead;
		pNode->pNext = pOldHead;
	} while(!__sync_bool_compare_and_swap(&g_pAtExitHead, pOldHead, pNode));

	return 0;
}
