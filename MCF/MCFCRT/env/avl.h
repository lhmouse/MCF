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
} AVL_NODE_HEADER, *AVL_ROOT;

__MCF_CRT_EXTERN int AVLAttach(
	AVL_NODE_HEADER **ppRoot,
	__MCF_STD intptr_t nKey,
	AVL_NODE_HEADER *pNode
);
__MCF_CRT_EXTERN void AVLDetach(
	const AVL_NODE_HEADER *pWhere
);
__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLFind(
	// Q: 为什么这里是 const AVL_NODE_HEADER * 而不是 AVL_NODE_HEADER * 呢？
	// A: 参考 strchr 函数。
	const AVL_NODE_HEADER *pRoot,
	__MCF_STD intptr_t nKey
);

__MCF_CRT_EXTERN void AVLSwap(
	AVL_NODE_HEADER **ppRoot1,
	AVL_NODE_HEADER **ppRoot2
);

__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLBegin(
	const AVL_NODE_HEADER *pRoot
);
__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLRBegin(
	const AVL_NODE_HEADER *pRoot
);
__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLNext(
	const AVL_NODE_HEADER *pWhere
);
__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLPrev(
	const AVL_NODE_HEADER *pWhere
);

__MCF_CRT_EXTERN int AVLTraverse(
	const AVL_NODE_HEADER *pRoot,
	int (*pfnCallback)(AVL_NODE_HEADER *, __MCF_STD intptr_t),
	__MCF_STD intptr_t nParam
);

#endif
