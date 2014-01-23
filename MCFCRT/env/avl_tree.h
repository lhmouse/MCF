// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_AVL_H__
#define __MCF_CRT_AVL_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

typedef struct tagAVLNodeHeader {
	__MCF_STD intptr_t nKey;
	struct tagAVLNodeHeader *pParent;
	struct tagAVLNodeHeader **ppRefl;
	struct tagAVLNodeHeader *pLeft;
	struct tagAVLNodeHeader *pRight;
	__MCF_STD size_t uHeight;
	struct tagAVLNodeHeader *pPrev;
	struct tagAVLNodeHeader *pNext;
} __MCF_AVL_NODE_HEADER;

typedef __MCF_AVL_NODE_HEADER *__MCF_AVL_PROOT;

static inline __MCF_AVL_NODE_HEADER *__MCF_AVLPrev(const __MCF_AVL_NODE_HEADER *pNode){
	return pNode->pPrev;
}
static inline __MCF_AVL_NODE_HEADER *__MCF_AVLNext(const __MCF_AVL_NODE_HEADER *pNode){
	return pNode->pNext;
}

extern void __MCF_AVLSwap(__MCF_AVL_PROOT *ppRoot1, __MCF_AVL_PROOT *ppRoot2);

// 若 arg0 < arg1 应返回非零值，否则应返回零。
typedef int (*__MCF_AVL_KEY_COMPARER)(__MCF_STD intptr_t, __MCF_STD intptr_t);

extern __MCF_AVL_NODE_HEADER *__MCF_AVLAttach(
	__MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode
);
extern __MCF_AVL_NODE_HEADER *__MCF_AVLAttachCustomComp(
	__MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
);
extern void __MCF_AVLDetach(
	const __MCF_AVL_NODE_HEADER *pNode
);

// Q: 为什么这里是 const __MCF_AVL_NODE_HEADER * 而不是 __MCF_AVL_NODE_HEADER * 呢？
// A: 参考 strchr 函数。
extern __MCF_AVL_NODE_HEADER *__MCF_AVLLowerBound(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
);
extern __MCF_AVL_NODE_HEADER *__MCF_AVLLowerBoundCustomComp(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
);
extern __MCF_AVL_NODE_HEADER *__MCF_AVLUpperBound(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
);
extern __MCF_AVL_NODE_HEADER *__MCF_AVLUpperBoundCustomComp(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
);
extern __MCF_AVL_NODE_HEADER *__MCF_AVLFind(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
);
extern __MCF_AVL_NODE_HEADER *__MCF_AVLFindCustomComp(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
);

__MCF_EXTERN_C_END

#endif
