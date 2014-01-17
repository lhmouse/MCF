// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

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

void __MCF_CRT_EmutlsInitialize(){
}
void __MCF_CRT_EmutlsUninitialize(){
	for(;;){
		KEY_DTOR_NODE *const pNode = (KEY_DTOR_NODE *)__MCF_LFListPopFront(&g_pDtorHeader);
		if(pNode == NULL){
			break;
		}
		free(pNode);
	}
}

void __MCF_CRT_RunEmutlsThreadDtors(){
	const KEY_DTOR_NODE *pNode = (const KEY_DTOR_NODE *)g_pDtorHeader;
	while(pNode != NULL){
		const KEY_DTOR_NODE *const pNext = (const KEY_DTOR_NODE *)__MCF_LFListNext((const __MCF_LFLIST_NODE_HEADER *)pNode);

		const LPVOID pMem = TlsGetValue(pNode->ulKey);
		if(pMem != NULL){
			(*pNode->pfnDtor)(pMem);
		}

		pNode = pNext;
	}
}

int __mingwthr_key_dtor(unsigned long ulKey, void (*pfnDtor)(void *)){
	if(pfnDtor != NULL){
		KEY_DTOR_NODE *const pNode = (KEY_DTOR_NODE *)malloc(sizeof(KEY_DTOR_NODE));
		if(pNode == NULL){
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
