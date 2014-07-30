// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "mingw_hacks.h"
#include "mcfwin.h"
#include "thread.h"
#include "avl_tree.h"
#include <stdlib.h>

typedef struct tagKeyDtorNode {
	MCF_AVL_NODE_HEADER vHeader;
	struct tagKeyDtorNode *pPrev;
	struct tagKeyDtorNode *pNext;

	unsigned long ulKey;
	void (*pfnDtor)(void *);
} KEY_DTOR_NODE;

static bool DtorComparatorNodes(
	const MCF_AVL_NODE_HEADER *pObj1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return ((const KEY_DTOR_NODE *)pObj1)->ulKey < ((const KEY_DTOR_NODE *)pObj2)->ulKey;
}
static bool DtorComparatorNodeKey(
	const MCF_AVL_NODE_HEADER *pObj1,
	intptr_t nKey2
){
	return ((const KEY_DTOR_NODE *)pObj1)->ulKey < (unsigned long)nKey2;
}
static bool DtorComparatorKeyNode(
	intptr_t nKey1,
	const MCF_AVL_NODE_HEADER *pObj2
){
	return (unsigned long)nKey1 < ((const KEY_DTOR_NODE *)pObj2)->ulKey;
}

static SRWLOCK			g_srwLock		= SRWLOCK_INIT;
static KEY_DTOR_NODE *	g_pDtorHead		= NULL;
static MCF_AVL_ROOT		g_pavlDtorRoot	= NULL;

bool __MCF_CRT_MinGWHacksInit(){
	return true;
}
void __MCF_CRT_MinGWHacksUninit(){
	while(g_pDtorHead){
		const LPVOID pMem = TlsGetValue(g_pDtorHead->ulKey);
		if(pMem){
			(*(g_pDtorHead->pfnDtor))(pMem);
			TlsSetValue(g_pDtorHead->ulKey, NULL);
		}

		KEY_DTOR_NODE *const pNext = g_pDtorHead->pNext;
		free(g_pDtorHead);
		g_pDtorHead = pNext;
	}
	g_pavlDtorRoot = NULL;
}

void __MCF_CRT_RunEmutlsDtors(){
	AcquireSRWLockShared(&g_srwLock);
	{
		for(const KEY_DTOR_NODE *pCur = g_pDtorHead; pCur; pCur = pCur->pNext){
			const LPVOID pMem = TlsGetValue(pCur->ulKey);
			if(pMem){
				(*(pCur->pfnDtor))(pMem);
				TlsSetValue(pCur->ulKey, NULL);
			}
		}
	}
	ReleaseSRWLockShared(&g_srwLock);
}

int __mingwthr_key_dtor(unsigned long ulKey, void (*pfnDtor)(void *)){
	if(pfnDtor){
		KEY_DTOR_NODE *const pNode = malloc(sizeof(KEY_DTOR_NODE));
		if(!pNode){
			return -1;
		}
		pNode->ulKey	= ulKey;
		pNode->pfnDtor	= pfnDtor;
		pNode->pPrev	= NULL;

		AcquireSRWLockExclusive(&g_srwLock);
		{
			pNode->pNext = g_pDtorHead;
			if(g_pDtorHead){
				g_pDtorHead->pPrev = pNode;
			}
			g_pDtorHead = pNode;

			MCF_AvlAttach(
				&g_pavlDtorRoot,
				(MCF_AVL_NODE_HEADER *)pNode,
				&DtorComparatorNodes
			);
		}
		ReleaseSRWLockExclusive(&g_srwLock);
	}
	return 0;
}

int __mingwthr_remove_key_dtor(unsigned long ulKey){
	AcquireSRWLockExclusive(&g_srwLock);
	{
		KEY_DTOR_NODE *pNode = (KEY_DTOR_NODE *)MCF_AvlFind(
			&g_pavlDtorRoot,
			(intptr_t)ulKey,
			&DtorComparatorNodeKey,
			&DtorComparatorKeyNode
		);
		if(pNode){
			if(g_pDtorHead == pNode){
				g_pDtorHead = pNode->pNext;
			}

			if(pNode->pNext){
				pNode->pNext->pPrev = pNode->pPrev;
			}
			if(pNode->pPrev){
				pNode->pPrev->pNext = pNode->pNext;
			}
			MCF_AvlDetach((MCF_AVL_NODE_HEADER *)pNode);

			free(pNode);
		}
	}
	ReleaseSRWLockExclusive(&g_srwLock);
	return 0;
}

unsigned int _get_output_format(){
	return 0;
}
