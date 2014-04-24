// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_AVL_H_
#define MCF_CRT_AVL_H_

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
} MCF_AVL_NODE_HEADER;

typedef MCF_AVL_NODE_HEADER *MCF_AVL_ROOT;

// 若 arg0 < arg1 应返回非零值，否则应返回零。
typedef int (*MCF_AVL_KEY_COMPARER)(__MCF_STD intptr_t, __MCF_STD intptr_t);

static inline MCF_AVL_NODE_HEADER *MCF_AvlPrev(const MCF_AVL_NODE_HEADER *pNode) __MCF_NOEXCEPT {
	return pNode->pPrev;
}
static inline MCF_AVL_NODE_HEADER *MCF_AvlNext(const MCF_AVL_NODE_HEADER *pNode) __MCF_NOEXCEPT {
	return pNode->pNext;
}

extern void MCF_AvlSwap(MCF_AVL_ROOT *ppRoot1, MCF_AVL_ROOT *ppRoot2) __MCF_NOEXCEPT;

void MCF_AvlInsertNoCheck(
	MCF_AVL_NODE_HEADER *pNode,
	__MCF_STD intptr_t nKey,
	MCF_AVL_NODE_HEADER *pParent,
	MCF_AVL_NODE_HEADER **ppIns
) __MCF_NOEXCEPT;

extern void MCF_AvlAttach(
	MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	MCF_AVL_NODE_HEADER *pNode
) __MCF_NOEXCEPT;

extern void MCF_AvlAttachCustomComp(
	MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	MCF_AVL_NODE_HEADER *pNode,
	MCF_AVL_KEY_COMPARER pfnKeyComparer
) __MCF_NOEXCEPT;

extern void MCF_AvlDetach(
	const MCF_AVL_NODE_HEADER *pNode
) __MCF_NOEXCEPT;

// Q: 为什么这里是 const MCF_AVL_NODE_HEADER * 而不是 MCF_AVL_NODE_HEADER * 呢？
// A: 参考 strchr 函数。
extern MCF_AVL_NODE_HEADER *MCF_AvlLowerBound(
	const MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;

extern MCF_AVL_NODE_HEADER *MCF_AvlLowerBoundCustomComp(
	const MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nOther,
	MCF_AVL_KEY_COMPARER pfnComparerKeyOther,
	MCF_AVL_KEY_COMPARER pfnComparerOtherKey
) __MCF_NOEXCEPT;

extern MCF_AVL_NODE_HEADER *MCF_AvlUpperBound(
	const MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;

extern MCF_AVL_NODE_HEADER *MCF_AvlUpperBoundCustomComp(
	const MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nOther,
	MCF_AVL_KEY_COMPARER pfnComparerKeyOther,
	MCF_AVL_KEY_COMPARER pfnComparerOtherKey
) __MCF_NOEXCEPT;

extern MCF_AVL_NODE_HEADER *MCF_AvlFind(
	const MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;

extern MCF_AVL_NODE_HEADER *MCF_AvlFindCustomComp(
	const MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nOther,
	MCF_AVL_KEY_COMPARER pfnComparerKeyOther,
	MCF_AVL_KEY_COMPARER pfnComparerOtherKey
) __MCF_NOEXCEPT;

extern void MCF_AvlEqualRange(
	MCF_AVL_NODE_HEADER **ppFrom,
	MCF_AVL_NODE_HEADER **ppTo,
	const MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;

extern void MCF_AvlEqualRangeCustomComp(
	MCF_AVL_NODE_HEADER **ppFrom,
	MCF_AVL_NODE_HEADER **ppTo,
	const MCF_AVL_ROOT *ppRoot,
	__MCF_STD intptr_t nOther,
	MCF_AVL_KEY_COMPARER pfnComparerKeyOther,
	MCF_AVL_KEY_COMPARER pfnComparerOtherKey
) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
