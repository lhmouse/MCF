// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_AVL_H_
#define MCF_CRT_AVL_H_

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

typedef struct tagAvlNodeHeader {
	struct tagAvlNodeHeader *pParent;
	struct tagAvlNodeHeader **ppRefl;
	struct tagAvlNodeHeader *pLeft;
	struct tagAvlNodeHeader *pRight;
	MCF_STD size_t uHeight;
	struct tagAvlNodeHeader *pPrev;
	struct tagAvlNodeHeader *pNext;
} MCF_AvlNodeHeader;

typedef MCF_AvlNodeHeader *MCF_AvlRoot;

// 若 arg0 < arg1 应返回非零值，否则应返回零。
typedef bool (*MCF_AvlComparatorNodes)(const MCF_AvlNodeHeader *, const MCF_AvlNodeHeader *);
typedef bool (*MCF_AvlComparatorNodeOther)(const MCF_AvlNodeHeader *, MCF_STD intptr_t);
typedef bool (*MCF_AvlComparatorOtherNode)(MCF_STD intptr_t, const MCF_AvlNodeHeader *);

static inline MCF_AvlNodeHeader *MCF_AvlFront(const MCF_AvlRoot *ppRoot) MCF_NOEXCEPT {
	MCF_AvlNodeHeader *pCur = *ppRoot;
	if(pCur){
		while(pCur->pLeft){
			pCur = pCur->pLeft;
		}
	}
	return pCur;
}
static inline MCF_AvlNodeHeader *MCF_AvlBack(const MCF_AvlRoot *ppRoot) MCF_NOEXCEPT {
	MCF_AvlNodeHeader *pCur = *ppRoot;
	if(pCur){
		while(pCur->pRight){
			pCur = pCur->pRight;
		}
	}
	return pCur;
}

static inline MCF_AvlNodeHeader *MCF_AvlPrev(const MCF_AvlNodeHeader *pNode) MCF_NOEXCEPT {
	return pNode->pPrev;
}
static inline MCF_AvlNodeHeader *MCF_AvlNext(const MCF_AvlNodeHeader *pNode) MCF_NOEXCEPT {
	return pNode->pNext;
}

extern void MCF_AvlSwap(MCF_AvlRoot *ppRoot1, MCF_AvlRoot *ppRoot2) MCF_NOEXCEPT;

extern void MCF_AvlAttachHint(MCF_AvlRoot *ppRoot,
	// 如果新节点被插入到该节点前后相邻的位置，则效率被优化。
	// 此处行为和 C++03 C++11 都兼容。
	// pHint 为空则调用 MCF_AvlAttach()。
	MCF_AvlNodeHeader *pHint,
	MCF_AvlNodeHeader *pNode, MCF_AvlComparatorNodes pfnComparator) MCF_NOEXCEPT;

static inline void MCF_AvlAttach(MCF_AvlRoot *ppRoot,
	MCF_AvlNodeHeader *pNode, MCF_AvlComparatorNodes pfnComparator) MCF_NOEXCEPT
{
	MCF_AvlAttachHint(ppRoot, NULL, pNode, pfnComparator);
}

extern void MCF_AvlDetach(const MCF_AvlNodeHeader *pNode) MCF_NOEXCEPT;

// Q: 为什么这里是 const MCF_AvlNodeHeader * 而不是 MCF_AvlNodeHeader * 呢？
// A: 参考 strchr 函数。
extern MCF_AvlNodeHeader *MCF_AvlLowerBound(const MCF_AvlRoot *ppRoot,
	MCF_STD intptr_t nOther, MCF_AvlComparatorNodeOther pfnComparatorNodeOther) MCF_NOEXCEPT;

extern MCF_AvlNodeHeader *MCF_AvlUpperBound(const MCF_AvlRoot *ppRoot,
	MCF_STD intptr_t nOther, MCF_AvlComparatorOtherNode pfnComparatorOtherNode) MCF_NOEXCEPT;

extern MCF_AvlNodeHeader *MCF_AvlFind(const MCF_AvlRoot *ppRoot, MCF_STD intptr_t nOther,
	MCF_AvlComparatorNodeOther pfnComparatorNodeOther,
	MCF_AvlComparatorOtherNode pfnComparatorOtherNode) MCF_NOEXCEPT;

extern void MCF_AvlEqualRange(MCF_AvlNodeHeader **ppBegin, MCF_AvlNodeHeader **ppEnd,
	const MCF_AvlRoot *ppRoot, MCF_STD intptr_t nOther,
	MCF_AvlComparatorNodeOther pfnComparatorNodeOther,
	MCF_AvlComparatorOtherNode pfnComparatorOtherNode) MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
