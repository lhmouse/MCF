// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "mingw_hacks.h"
#include "../ext/assert.h"
#include "../ext/expect.h"
#include "thread.h"
#include "avl_tree.h"
#include <stdlib.h>
#include <windows.h>

typedef struct tagKeyDtorNode {
	MCF_AVL_NODE_HEADER vHeader;
	struct tagKeyDtorNode *pNext;

	unsigned long ulKey;
	void (*pfnDtor)(void *);
} KEY_DTOR_NODE;

static CRITICAL_SECTION	g_csLock;
static KEY_DTOR_NODE *	g_pDtorHead		= NULL;
static MCF_AVL_ROOT		g_avlDtorRoot	= NULL;

static int DtorComparatorNodes(
	const MCF_AVL_NODE_HEADER *pObj1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return ((const KEY_DTOR_NODE *)pObj1)->ulKey < ((const KEY_DTOR_NODE *)pObj2)->ulKey;
}
static int DtorComparatorNodeKey(
	const MCF_AVL_NODE_HEADER *pObj1,
	intptr_t nKey2
){
	return ((const KEY_DTOR_NODE *)pObj1)->ulKey < (unsigned long)nKey2;
}
static int DtorComparatorKeyNode(
	intptr_t nKey1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return (unsigned long)nKey1 < ((const KEY_DTOR_NODE *)pObj2)->ulKey;
}

unsigned long __MCF_CRT_MinGWHacksInitialize(){
	InitializeCriticalSectionAndSpinCount(&g_csLock, 0x400);
	return ERROR_SUCCESS;
}
void __MCF_CRT_MinGWHacksUninitialize(){
	DeleteCriticalSection(&g_csLock);
}

void __MCF_CRT_RunEmutlsThreadDtors(){
	EnterCriticalSection(&g_csLock);
		const KEY_DTOR_NODE *pNode = g_pDtorHead;
		while(pNode){
			const LPVOID pMem = TlsGetValue(pNode->ulKey);
			if((GetLastError() == ERROR_SUCCESS) && pMem){
				(*(pNode->pfnDtor))(pMem);
			}
			pNode = pNode->pNext;
		}
	LeaveCriticalSection(&g_csLock);
}

void __MCF_CRT_EmutlsCleanup(){
	EnterCriticalSection(&g_csLock);
		for(;;){
			KEY_DTOR_NODE *const pNode = g_pDtorHead;
			if(!pNode){
				break;
			}
			g_pDtorHead = pNode->pNext;
			free(pNode);
		}
		g_avlDtorRoot = NULL;
	LeaveCriticalSection(&g_csLock);
}

int __mingwthr_key_dtor(unsigned long ulKey, void (*pfnDtor)(void *)){
	if(pfnDtor){
		KEY_DTOR_NODE *const pNode = malloc(sizeof(KEY_DTOR_NODE));
		if(!pNode){
			return -1;
		}
		pNode->ulKey = ulKey;
		pNode->pfnDtor = pfnDtor;

		EnterCriticalSection(&g_csLock);
			pNode->pNext = g_pDtorHead;
			g_pDtorHead = pNode;

			MCF_AvlAttach(
				&g_avlDtorRoot,
				(MCF_AVL_NODE_HEADER *)pNode,
				&DtorComparatorNodes
			);
		LeaveCriticalSection(&g_csLock);
	}
	return 0;
}

int __mingwthr_remove_key_dtor(unsigned long ulKey){
	EnterCriticalSection(&g_csLock);
		KEY_DTOR_NODE *pNode = (KEY_DTOR_NODE *)MCF_AvlFind(
			&g_avlDtorRoot,
			(intptr_t)ulKey,
			&DtorComparatorNodeKey,
			&DtorComparatorKeyNode
		);
		if(pNode){
			if(g_pDtorHead == pNode){
				g_pDtorHead = pNode->pNext;
			}
			MCF_AvlDetach((MCF_AVL_NODE_HEADER *)pNode);

			free(pNode);
		}
	LeaveCriticalSection(&g_csLock);
	return 0;
}

unsigned int _get_output_format(){
	return 0;
}
