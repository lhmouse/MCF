// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_AVL_H__
#define __MCF_CRT_AVL_H__

#include "_crtdef.h"

typedef struct tagAVLNodeHeader{
	__MCF_STD intptr_t nKey;
	struct tagAVLNodeHeader *pParent;
	struct tagAVLNodeHeader **ppRefl;
	struct tagAVLNodeHeader *pLeft;
	struct tagAVLNodeHeader *pRight;
	__MCF_STD size_t uHeight;
} __MCF_AVL_NODE_HEADER, *__MCF_AVL_ROOT;

// 若 arg0 < arg1 应返回非零值，否则应返回零。
__MCF_CRT_TYPEDEF_PROC int (*__MCF_AVL_KEY_COMPARER)(__MCF_STD intptr_t, __MCF_STD intptr_t);

__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLAttach(
	__MCF_AVL_NODE_HEADER **ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode
);
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLAttachCustomComp(
	__MCF_AVL_NODE_HEADER **ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
);
__MCF_CRT_EXTERN void __MCF_AVLDetach(
	const __MCF_AVL_NODE_HEADER *pWhere
);
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLFind(
	// Q: 为什么这里是 const __MCF_AVL_NODE_HEADER * 而不是 __MCF_AVL_NODE_HEADER * 呢？
	// A: 参考 strchr 函数。
	const __MCF_AVL_NODE_HEADER *pRoot,
	__MCF_STD intptr_t nKey
);
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLFindCustomComp(
	const __MCF_AVL_NODE_HEADER *pRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
);

__MCF_CRT_EXTERN void __MCF_AVLSwap(
	__MCF_AVL_NODE_HEADER **ppRoot1,
	__MCF_AVL_NODE_HEADER **ppRoot2
);

__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLBegin(
	const __MCF_AVL_NODE_HEADER *pRoot
);
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLRBegin(
	const __MCF_AVL_NODE_HEADER *pRoot
);
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLNext(
	const __MCF_AVL_NODE_HEADER *pWhere
);
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLPrev(
	const __MCF_AVL_NODE_HEADER *pWhere
);

__MCF_CRT_EXTERN int __MCF_AVLTraverse(
	const __MCF_AVL_NODE_HEADER *pRoot,
	int (*pfnCallback)(__MCF_AVL_NODE_HEADER *, __MCF_STD intptr_t),
	__MCF_STD intptr_t nParam
);

#endif
