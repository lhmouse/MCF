// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "lockfree_list.h"

void __MCF_LFListPushFront(__MCF_LFLIST_PHEAD *ppHead, __MCF_LFLIST_NODE_HEADER *pNode){
	__MCF_LFLIST_NODE_HEADER *pOldHead = *ppHead;
	do {
		pNode->pNext = pOldHead;
	} while(!__atomic_compare_exchange_n(ppHead, &pOldHead, pNode, 1, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
}
__MCF_LFLIST_NODE_HEADER *__MCF_LFListPopFront(__MCF_LFLIST_PHEAD *ppHead){
	__MCF_LFLIST_NODE_HEADER *pHead = *ppHead;
	do {
		if(pHead == NULL){
			break;
		}
	} while(!__atomic_compare_exchange_n(ppHead, &pHead, pHead->pNext, 1, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
	return pHead;
}

__MCF_LFLIST_NODE_HEADER *__MCF_LFListNext(const __MCF_LFLIST_NODE_HEADER *pHead){
	return pHead->pNext;
}
