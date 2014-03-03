// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

// 多线程同时写，单线程读，可以不加锁。

#ifndef __MCF_CRT_LOCKFREE_LIST_H__
#define __MCF_CRT_LOCKFREE_LIST_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

typedef struct tagLFListNodeHeader {
	struct tagLFListNodeHeader *pNext;
} __MCF_LFLIST_NODE_HEADER;

typedef __MCF_LFLIST_NODE_HEADER *volatile __MCF_LFLIST_PHEAD;

static inline void __MCF_LFListPushFront(__MCF_LFLIST_PHEAD *ppHead, __MCF_LFLIST_NODE_HEADER *pNode) __MCF_NOEXCEPT {
	__MCF_LFLIST_NODE_HEADER *pOldHead = __atomic_load_n(ppHead, __ATOMIC_RELAXED);
	do {
		pNode->pNext = pOldHead;
	} while(!__atomic_compare_exchange_n(ppHead, &pOldHead, pNode, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
}
static inline __MCF_LFLIST_NODE_HEADER *__MCF_LFListPopFront(__MCF_LFLIST_PHEAD *ppHead) __MCF_NOEXCEPT {
	__MCF_LFLIST_NODE_HEADER *pNode = __atomic_load_n(ppHead, __ATOMIC_RELAXED);
	do {
		if(!pNode){
			break;
		}
	} while(!__atomic_compare_exchange_n(ppHead, &pNode, pNode->pNext, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
	return pNode;
}

static inline __MCF_LFLIST_NODE_HEADER *__MCF_LFListBegin(const __MCF_LFLIST_PHEAD *ppHead) __MCF_NOEXCEPT {
	return __atomic_load_n(ppHead, __ATOMIC_RELAXED);
}
static inline __MCF_LFLIST_NODE_HEADER *__MCF_LFListNext(const __MCF_LFLIST_NODE_HEADER *pNode){
	return pNode->pNext;
}

__MCF_EXTERN_C_END

#endif
