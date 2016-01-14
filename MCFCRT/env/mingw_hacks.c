// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "mingw_hacks.h"
#include "mcfwin.h"
#include "thread.h"
#include "avl_tree.h"
#include <stdlib.h>

typedef struct tagKeyDtorNode {
	MCFCRT_AvlNodeHeader vHeader;
	struct tagKeyDtorNode *pPrev;
	struct tagKeyDtorNode *pNext;

	unsigned long ulKey;
	void (*pfnDtor)(void *);
} KeyDtorNode;

_Static_assert(sizeof(unsigned long) <= sizeof(uintptr_t), "This platform is not supported.");

static int DtorComparatorNodeKey(const MCFCRT_AvlNodeHeader *pObj1, intptr_t nKey2){
	const unsigned long ulKey1 = ((const KeyDtorNode *)pObj1)->ulKey;
	const unsigned long ulKey2 = (unsigned long)(uintptr_t)nKey2;
	return (ulKey1 < ulKey2) ? -1 : ((ulKey1 > ulKey2) ? 1 : 0);
}
static int DtorComparatorNodes(const MCFCRT_AvlNodeHeader *pObj1, const MCFCRT_AvlNodeHeader *pObj2){
	return DtorComparatorNodeKey(pObj1, (intptr_t)(uintptr_t)((const KeyDtorNode *)pObj2)->ulKey);
}

static SRWLOCK          g_srwDtorMapLock = SRWLOCK_INIT;
static KeyDtorNode *    g_pDtorHead      = nullptr;
static MCFCRT_AvlRoot      g_pavlDtorRoot   = nullptr;

bool __MCFCRT_MinGWHacksInit(){
	return true;
}
void __MCFCRT_MinGWHacksUninit(){
	while(g_pDtorHead){
		const LPVOID pMem = TlsGetValue(g_pDtorHead->ulKey);
		if(pMem){
			(*(g_pDtorHead->pfnDtor))(pMem);
			TlsSetValue(g_pDtorHead->ulKey, nullptr);
		}

		KeyDtorNode *const pNext = g_pDtorHead->pNext;
		free(g_pDtorHead);
		g_pDtorHead = pNext;
	}
	g_pavlDtorRoot = nullptr;
}

void __MCFCRT_RunEmutlsDtors(){
	AcquireSRWLockExclusive(&g_srwDtorMapLock);
	{
		for(const KeyDtorNode *pCur = g_pDtorHead; pCur; pCur = pCur->pNext){
			const LPVOID pMem = TlsGetValue(pCur->ulKey);
			if(pMem){
				(*(pCur->pfnDtor))(pMem);
				TlsSetValue(pCur->ulKey, nullptr);
			}
		}
	}
	ReleaseSRWLockExclusive(&g_srwDtorMapLock);
}

int __mingwthr_key_dtor(unsigned long ulKey, void (*pfnDtor)(void *)){
	if(pfnDtor){
		KeyDtorNode *const pNode = malloc(sizeof(KeyDtorNode));
		if(!pNode){
			return -1;
		}
		pNode->ulKey   = ulKey;
		pNode->pfnDtor = pfnDtor;
		pNode->pPrev   = nullptr;

		AcquireSRWLockExclusive(&g_srwDtorMapLock);
		{
			pNode->pNext = g_pDtorHead;
			if(g_pDtorHead){
				g_pDtorHead->pPrev = pNode;
			}
			g_pDtorHead = pNode;

			MCFCRT_AvlAttach(&g_pavlDtorRoot, (MCFCRT_AvlNodeHeader *)pNode, &DtorComparatorNodes);
		}
		ReleaseSRWLockExclusive(&g_srwDtorMapLock);
	}
	return 0;
}

int __mingwthr_remove_key_dtor(unsigned long ulKey){
	AcquireSRWLockExclusive(&g_srwDtorMapLock);
	{
		KeyDtorNode *pNode = (KeyDtorNode *)MCFCRT_AvlFind(
			&g_pavlDtorRoot, (intptr_t)ulKey, &DtorComparatorNodeKey);
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
			MCFCRT_AvlDetach((MCFCRT_AvlNodeHeader *)pNode);

			free(pNode);
		}
	}
	ReleaseSRWLockExclusive(&g_srwDtorMapLock);
	return 0;
}

__attribute__((__used__))
int _CRT_MT = 2;
