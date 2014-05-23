// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "mingw_hacks.h"
#include "../ext/assert.h"
#include "../ext/expect.h"
#include "thread.h"
#include <stdlib.h>
#include <windows.h>

typedef struct tagKeyDtorNode {
	struct tagKeyDtorNode *pNext;

	unsigned long ulKey;
	void (*pfnDtor)(void *);
} KEY_DTOR_NODE;

static KEY_DTOR_NODE *g_pDtorHead = NULL;

void __MCF_CRT_RunEmutlsThreadDtors(){
	const KEY_DTOR_NODE *pNode = __atomic_load_n(&g_pDtorHead, __ATOMIC_ACQUIRE);
	while(pNode){
		const LPVOID pMem = TlsGetValue(pNode->ulKey);
		if((GetLastError() == ERROR_SUCCESS) && pMem){
			(*(pNode->pfnDtor))(pMem);
		}
		pNode = pNode->pNext;
	}
}

void __MCF_CRT_EmutlsCleanup(){
	for(;;){
		KEY_DTOR_NODE *pNode = __atomic_load_n(&g_pDtorHead, __ATOMIC_ACQUIRE);
		while(EXPECT(pNode && !__atomic_compare_exchange_n(&g_pDtorHead, &pNode, pNode->pNext, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))){
			SwitchToThread();
		}
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

		pNode->pNext = __atomic_load_n(&g_pDtorHead, __ATOMIC_ACQUIRE);
		while(EXPECT(!__atomic_compare_exchange_n(&g_pDtorHead, &(pNode->pNext), pNode, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))){
			SwitchToThread();
		}
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
