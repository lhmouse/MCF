// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_AVL_H_
#define MCF_CRT_AVL_H_

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

typedef struct MCF_tagAvlNodeHeader {
	struct MCF_tagAvlNodeHeader *pParent;
	struct MCF_tagAvlNodeHeader **ppRefl;
	struct MCF_tagAvlNodeHeader *pLeft;
	struct MCF_tagAvlNodeHeader *pRight;
	MCF_STD size_t uHeight;
	struct MCF_tagAvlNodeHeader *pPrev;
	struct MCF_tagAvlNodeHeader *pNext;
} MCF_AVL_NODE_HEADER;

typedef MCF_AVL_NODE_HEADER *MCF_AVL_ROOT;

// 若 arg0 < arg1 应返回非零值，否则应返回零。
typedef bool (*MCF_AVL_COMPARATOR_NODES)(
	const MCF_AVL_NODE_HEADER *,
	const MCF_AVL_NODE_HEADER *
);
typedef bool (*MCF_AVL_COMPARATOR_NODE_OTHER)(
	const MCF_AVL_NODE_HEADER *,
	MCF_STD intptr_t
);
typedef bool (*MCF_AVL_COMPARATOR_OTHER_NODE)(
	MCF_STD intptr_t,
	const MCF_AVL_NODE_HEADER *
);

static inline MCF_AVL_NODE_HEADER *MCF_AvlFront(
	const MCF_AVL_ROOT *ppRoot
) MCF_NOEXCEPT {
	MCF_AVL_NODE_HEADER *pCur = *ppRoot;
	if(pCur){
		while(pCur->pLeft){
			pCur = pCur->pLeft;
		}
	}
	return pCur;
}
static inline MCF_AVL_NODE_HEADER *MCF_AvlBack(
	const MCF_AVL_ROOT *ppRoot
) MCF_NOEXCEPT {
	MCF_AVL_NODE_HEADER *pCur = *ppRoot;
	if(pCur){
		while(pCur->pRight){
			pCur = pCur->pRight;
		}
	}
	return pCur;
}

static inline MCF_AVL_NODE_HEADER *MCF_AvlPrev(
	const MCF_AVL_NODE_HEADER *pNode
) MCF_NOEXCEPT {
	return pNode->pPrev;
}
static inline MCF_AVL_NODE_HEADER *MCF_AvlNext(
	const MCF_AVL_NODE_HEADER *pNode
) MCF_NOEXCEPT {
	return pNode->pNext;
}

extern void MCF_AvlSwap(
	MCF_AVL_ROOT *ppRoot1,
	MCF_AVL_ROOT *ppRoot2
) MCF_NOEXCEPT;

extern void MCF_AvlAttach(
	MCF_AVL_ROOT *ppRoot,
	MCF_AVL_NODE_HEADER *pNode,
	MCF_AVL_COMPARATOR_NODES pfnComparator
) MCF_NOEXCEPT;

extern void MCF_AvlDetach(
	const MCF_AVL_NODE_HEADER *pNode
) MCF_NOEXCEPT;

// Q: 为什么这里是 const MCF_AVL_NODE_HEADER * 而不是 MCF_AVL_NODE_HEADER * 呢？
// A: 参考 strchr 函数。
extern MCF_AVL_NODE_HEADER *MCF_AvlLowerBound(
	const MCF_AVL_ROOT *ppRoot,
	MCF_STD intptr_t nOther,
	MCF_AVL_COMPARATOR_NODE_OTHER pfnComparatorNodeOther
) MCF_NOEXCEPT;

extern MCF_AVL_NODE_HEADER *MCF_AvlUpperBound(
	const MCF_AVL_ROOT *ppRoot,
	MCF_STD intptr_t nOther,
	MCF_AVL_COMPARATOR_OTHER_NODE pfnComparatorOtherNode
) MCF_NOEXCEPT;

extern MCF_AVL_NODE_HEADER *MCF_AvlFind(
	const MCF_AVL_ROOT *ppRoot,
	MCF_STD intptr_t nOther,
	MCF_AVL_COMPARATOR_NODE_OTHER pfnComparatorNodeOther,
	MCF_AVL_COMPARATOR_OTHER_NODE pfnComparatorOtherNode
) MCF_NOEXCEPT;

extern void MCF_AvlEqualRange(
	MCF_AVL_NODE_HEADER **ppBegin,
	MCF_AVL_NODE_HEADER **ppEnd,
	const MCF_AVL_ROOT *ppRoot,
	MCF_STD intptr_t nOther,
	MCF_AVL_COMPARATOR_NODE_OTHER pfnComparatorNodeOther,
	MCF_AVL_COMPARATOR_OTHER_NODE pfnComparatorOtherNode
) MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
