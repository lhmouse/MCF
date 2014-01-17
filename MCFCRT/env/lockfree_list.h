// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

// 多线程同时写，单线程读，可以不加锁。

#ifndef __MCF_CRT_LOCKFREE_LIST_H__
#define __MCF_CRT_LOCKFREE_LIST_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

typedef struct tagLFListNodeHeader {
	struct tagLFListNodeHeader *pNext;
} __MCF_LFLIST_NODE_HEADER, *volatile __MCF_LFLIST_PHEAD;

extern void __MCF_LFListPushFront(__MCF_LFLIST_PHEAD *ppHead, __MCF_LFLIST_NODE_HEADER *pNode);
extern __MCF_LFLIST_NODE_HEADER *__MCF_LFListPopFront(__MCF_LFLIST_PHEAD *ppHead);

extern __MCF_LFLIST_NODE_HEADER *__MCF_LFListNext(const __MCF_LFLIST_NODE_HEADER *pHead);

__MCF_EXTERN_C_END

#endif
