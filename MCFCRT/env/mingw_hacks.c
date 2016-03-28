// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "mingw_hacks.h"
#include "mcfwin.h"
#include "mutex.h"
#include "thread.h"
#include "avl_tree.h"
#include <stdlib.h>

enum {
	kMutexSpinCount = 100,
};

typedef struct tagKeyDtorNode {
	_MCFCRT_AvlNodeHeader vHeader;
	struct tagKeyDtorNode *pPrev;
	struct tagKeyDtorNode *pNext;

	unsigned long ulKey;
	void (*pfnDtor)(void *);
} KeyDtorNode;

_Static_assert(sizeof(unsigned long) <= sizeof(uintptr_t), "This platform is not supported.");

static int DtorComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pObj1, intptr_t nKey2){
	const unsigned long ulKey1 = ((const KeyDtorNode *)pObj1)->ulKey;
	const unsigned long ulKey2 = (unsigned long)(uintptr_t)nKey2;
	return (ulKey1 < ulKey2) ? -1 : ((ulKey1 > ulKey2) ? 1 : 0);
}
static int DtorComparatorNodes(const _MCFCRT_AvlNodeHeader *pObj1, const _MCFCRT_AvlNodeHeader *pObj2){
	return DtorComparatorNodeKey(pObj1, (intptr_t)(uintptr_t)((const KeyDtorNode *)pObj2)->ulKey);
}

static _MCFCRT_Mutex    g_vDtorMapMutex  = 0;
static KeyDtorNode *    g_pDtorHead      = nullptr;
static _MCFCRT_AvlRoot  g_avlDtorRoot    = nullptr;

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
	g_avlDtorRoot = nullptr;
}

void __MCFCRT_RunEmutlsDtors(){
	_MCFCRT_WaitForMutexForever(&g_vDtorMapMutex, kMutexSpinCount);
	{
		for(const KeyDtorNode *pCur = g_pDtorHead; pCur; pCur = pCur->pNext){
			const LPVOID pMem = TlsGetValue(pCur->ulKey);
			if(pMem){
				(*(pCur->pfnDtor))(pMem);
				TlsSetValue(pCur->ulKey, nullptr);
			}
		}
	}
	_MCFCRT_SignalMutex(&g_vDtorMapMutex);
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

		_MCFCRT_WaitForMutexForever(&g_vDtorMapMutex, kMutexSpinCount);
		{
			pNode->pNext = g_pDtorHead;
			if(g_pDtorHead){
				g_pDtorHead->pPrev = pNode;
			}
			g_pDtorHead = pNode;

			_MCFCRT_AvlAttach(&g_avlDtorRoot, (_MCFCRT_AvlNodeHeader *)pNode, &DtorComparatorNodes);
		}
		_MCFCRT_SignalMutex(&g_vDtorMapMutex);
	}
	return 0;
}

int __mingwthr_remove_key_dtor(unsigned long ulKey){
	_MCFCRT_WaitForMutexForever(&g_vDtorMapMutex, kMutexSpinCount);
	{
		KeyDtorNode *pNode = (KeyDtorNode *)_MCFCRT_AvlFind(&g_avlDtorRoot, (intptr_t)ulKey, &DtorComparatorNodeKey);
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
			_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)pNode);

			free(pNode);
		}
	}
	_MCFCRT_SignalMutex(&g_vDtorMapMutex);
	return 0;
}

__attribute__((__used__))
int _CRT_MT = 2;
