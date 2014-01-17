// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "MCFCRT.h"
#include "env/lockfree_list.h"
#include "env/mingw_hacks.h"
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
	__MCF_LFLIST_NODE_HEADER LFListHeader;

	void (__cdecl *pfnProc)(intptr_t);
	intptr_t nContext;
} AT_EXIT_NODE;

static __MCF_LFLIST_PHEAD g_pAtExitHeader;

unsigned long __MCF_CRT_Begin(){
	DWORD dwExitCode;

#define INIT(exp)		if((dwExitCode = (exp)) == ERROR_SUCCESS){ ((void)0)
#define CLEANUP(exp)	(exp); } ((void)0)

	INIT(__MCF_CRT_HeapInitialize());
	INIT(__MCF_CRT_TlsEnvInitialize());
	INIT(__MCF_CRT_EmutlsInitialize());

	__main();
	return ERROR_SUCCESS;

	CLEANUP(__MCF_CRT_EmutlsUninitialize());
	CLEANUP(__MCF_CRT_TlsEnvUninitialize());
	CLEANUP(__MCF_CRT_HeapUninitialize());

	return dwExitCode;
}
void __MCF_CRT_End(){
	for(;;){
		AT_EXIT_NODE *const pNode = (AT_EXIT_NODE *)__MCF_LFListPopFront(&g_pAtExitHeader);
		if(pNode == NULL){
			break;
		}
		(*pNode->pfnProc)(pNode->nContext);
		free(pNode);
	}

	__MCF_CRT_EmutlsUninitialize();
	__MCF_CRT_TlsEnvUninitialize();
	__MCF_CRT_HeapUninitialize();
}

void *__MCF_GetModuleBase(){
	return &IMAGE_BASE;
}
int __MCF_AtCRTEnd(void (__cdecl *pfnProc)(intptr_t), intptr_t nContext){
	AT_EXIT_NODE *const pNode = (AT_EXIT_NODE *)malloc(sizeof(AT_EXIT_NODE));
	if(pNode == NULL){
		return -1;
	}
	pNode->pfnProc = pfnProc;
	pNode->nContext = nContext;

	__MCF_LFListPushFront(&g_pAtExitHeader, (__MCF_LFLIST_NODE_HEADER *)pNode);

	return 0;
}
