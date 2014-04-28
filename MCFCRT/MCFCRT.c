// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "MCFCRT.h"
#include "env/mingw_hacks.h"
#include <stdlib.h>
#include <windows.h>
#include <winnt.h>

extern void __cdecl __main(void);

// ld 自动添加此符号。
extern IMAGE_DOS_HEADER g_vDosHeader __asm__("__image_base__");

typedef struct tagAtExitNode {
	struct tagAtExitNode *pNext;

	void (__cdecl *pfnProc)(intptr_t);
	intptr_t nContext;
} AT_EXIT_NODE;

static AT_EXIT_NODE *g_pAtExitHead = NULL;

unsigned long __MCF_CRT_Begin(){
	DWORD dwExitCode;

#define INIT(exp)		if((dwExitCode = (exp)) == ERROR_SUCCESS){ ((void)0)
#define CLEANUP(exp)	(exp); } ((void)0)

	INIT(____MCF_CRT_HeapInitialize());
	INIT(__MCF_CRT_TlsEnvInitialize());

	__main();
	return ERROR_SUCCESS;

	CLEANUP(__MCF_CRT_TlsEnvUninitialize());
	CLEANUP(____MCF_CRT_HeapUninitialize());

	return dwExitCode;
}
void __MCF_CRT_End(){
	for(;;){
		AT_EXIT_NODE *pNode = __atomic_load_n(&g_pAtExitHead, __ATOMIC_ACQUIRE);
		while(pNode && !__atomic_compare_exchange_n(&g_pAtExitHead, &pNode, pNode->pNext, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)){
			// 空的。
		}
		if(!pNode){
			break;
		}

		(*pNode->pfnProc)(pNode->nContext);
		free(pNode);
	}

	__MCF_CRT_EmutlsCleanup();
	__MCF_CRT_TlsEnvUninitialize();
	____MCF_CRT_HeapUninitialize();
}

void *MCF_GetModuleBase(){
	return &g_vDosHeader;
}

int MCF_AtCRTEnd(void (__cdecl *pfnProc)(intptr_t), intptr_t nContext){
	AT_EXIT_NODE *const pNode = malloc(sizeof(AT_EXIT_NODE));
	if(!pNode){
		return -1;
	}

	pNode->pfnProc = pfnProc;
	pNode->nContext = nContext;

	pNode->pNext = __atomic_load_n(&g_pAtExitHead, __ATOMIC_ACQUIRE);
	while(!__atomic_compare_exchange_n(&g_pAtExitHead, &(pNode->pNext), pNode, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)){
		// 空的。
	}

	return 0;
}
