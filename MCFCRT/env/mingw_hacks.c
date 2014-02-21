// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "_crtdef.h"
#include "thread.h"
#include "lockfree_list.h"
#include <stdlib.h>
#include <windows.h>

typedef struct tagKeyDtorNode {
	__MCF_LFLIST_NODE_HEADER LFListHeader;

	unsigned long ulKey;
	void (*pfnDtor)(void *);
} KEY_DTOR_NODE;

static __MCF_LFLIST_PHEAD g_pDtorHeader;

void __MCF_CRT_RunEmutlsThreadDtors(){
	const KEY_DTOR_NODE *pNode = (const KEY_DTOR_NODE *)g_pDtorHeader;
	while(pNode){
		const LPVOID pMem = TlsGetValue(pNode->ulKey);
		if((GetLastError() == ERROR_SUCCESS) && pMem){
			(*pNode->pfnDtor)(pMem);
		}
		pNode = (const KEY_DTOR_NODE *)__MCF_LFListNext((const __MCF_LFLIST_NODE_HEADER *)pNode);
	}
}

void __MCF_CRT_EmutlsCleanup(){
	for(;;){
		KEY_DTOR_NODE *const pNode = (KEY_DTOR_NODE *)__MCF_LFListPopFront(&g_pDtorHeader);
		if(!pNode){
			break;
		}
		free(pNode);
	}
}

int __mingwthr_key_dtor(unsigned long ulKey, void (*pfnDtor)(void *)){
	if(pfnDtor){
		KEY_DTOR_NODE *const pNode = malloc(sizeof(KEY_DTOR_NODE));
		if(!pNode){
			return -1;
		}
		pNode->ulKey = ulKey;
		pNode->pfnDtor = pfnDtor;

		__MCF_LFListPushFront(&g_pDtorHeader, (__MCF_LFLIST_NODE_HEADER *)pNode);
	}
	return 0;
}
/*
int __mingwthr_remove_key_dtor(unsigned long ulKey){
	// 这个函数有用吗？
}
*/
unsigned int _get_output_format(){
	return 0;
}
