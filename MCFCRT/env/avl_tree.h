// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_AVL_H_
#define MCF_CRT_AVL_H_

#include "_crtdef.h"
#include "../ext/assert.h"

__MCF_CRT_EXTERN_C_BEGIN

typedef struct tagMCF_AvlNodeHeader {
	struct tagMCF_AvlNodeHeader *pParent;
	struct tagMCF_AvlNodeHeader **ppRefl;
	struct tagMCF_AvlNodeHeader *pLeft;
	struct tagMCF_AvlNodeHeader *pRight;
	MCF_STD size_t uHeight;
	struct tagMCF_AvlNodeHeader *pPrev;
	struct tagMCF_AvlNodeHeader *pNext;
} MCF_AvlNodeHeader;

static inline MCF_AvlNodeHeader *MCF_AvlPrev(const MCF_AvlNodeHeader *pNode) MCF_NOEXCEPT {
	return pNode->pPrev;
}
static inline MCF_AvlNodeHeader *MCF_AvlNext(const MCF_AvlNodeHeader *pNode) MCF_NOEXCEPT {
	return pNode->pNext;
}

extern void MCF_AvlInternalAttach(MCF_AvlNodeHeader *pNode,
	MCF_AvlNodeHeader *pParent, MCF_AvlNodeHeader **ppRefl) MCF_NOEXCEPT;
extern void MCF_AvlInternalDetach(const MCF_AvlNodeHeader *pNode) MCF_NOEXCEPT;

typedef MCF_AvlNodeHeader *MCF_AvlRoot;

extern MCF_AvlNodeHeader *MCF_AvlFront(const MCF_AvlRoot *ppRoot) MCF_NOEXCEPT;
extern MCF_AvlNodeHeader *MCF_AvlBack(const MCF_AvlRoot *ppRoot) MCF_NOEXCEPT;

extern void MCF_AvlSwap(MCF_AvlRoot *ppRoot1, MCF_AvlRoot *ppRoot2) MCF_NOEXCEPT;

// 参考 strcmp()。
typedef int (*MCF_AvlComparatorNodes)(const MCF_AvlNodeHeader *, const MCF_AvlNodeHeader *);
typedef int (*MCF_AvlComparatorNodeOther)(const MCF_AvlNodeHeader *, MCF_STD intptr_t);

static inline void MCF_AvlAttachWithHint(MCF_AvlRoot *ppRoot,
	// 如果新节点被插入到该节点前后相邻的位置，则效率被优化。
	// 此处行为和 C++03 C++11 都兼容。
	// pHint 为空则调用 MCF_AvlAttach()。
	MCF_AvlNodeHeader *pHint,
	MCF_AvlNodeHeader *pNode, MCF_AvlComparatorNodes pfnComparator)
{
	MCF_AvlNodeHeader *pParent = nullptr;
	MCF_AvlNodeHeader **ppRefl = ppRoot;
	if(pHint){
		if((*pfnComparator)(pNode, pHint) < 0){
			MCF_AvlNodeHeader *const pPrev = pHint->pPrev;
			if(!pPrev){
				ASSERT(!pHint->pLeft);

				pParent = pHint;
				ppRefl = &(pHint->pLeft);
			} else if((*pfnComparator)(pNode, pPrev) >= 0){
				// 条件：	node		<	hint
				//			hint->prev	<=	node
				if(pPrev->uHeight < pHint->uHeight){
					ASSERT(!pPrev->pRight);

					pParent = pPrev;
					ppRefl = &(pPrev->pRight);
				} else {
					ASSERT(!pHint->pLeft);

					pParent = pHint;
					ppRefl = &(pHint->pLeft);
				}
			}
		} else {
			MCF_AvlNodeHeader *const pNext = pHint->pNext;
			if(!pNext){
				ASSERT(!pHint->pRight);

				pParent = pHint;
				ppRefl = &(pHint->pRight);
			} else if((*pfnComparator)(pNode, pNext) < 0){
				// 条件：	hint	<=	node
				//			node	<	hint->next
				if(pHint->uHeight < pNext->uHeight){
					ASSERT(!pHint->pRight);

					pParent = pHint;
					ppRefl = &(pHint->pRight);
				} else {
					ASSERT(!pNext->pLeft);

					pParent = pNext;
					ppRefl = &(pNext->pLeft);
				}
			}
		}
	}
	if(!pParent){
		for(;;){
			MCF_AvlNodeHeader *const pCur = *ppRefl;
			if(!pCur){
				break;
			}
			if((*pfnComparator)(pNode, pCur) < 0){
				pParent = pCur;
				ppRefl = &(pCur->pLeft);
			} else {
				pParent = pCur;
				ppRefl = &(pCur->pRight);
			}
		}
	}
	MCF_AvlInternalAttach(pNode, pParent, ppRefl);
}

static inline void MCF_AvlAttach(MCF_AvlRoot *ppRoot,
	MCF_AvlNodeHeader *pNode, MCF_AvlComparatorNodes pfnComparator)
{
	MCF_AvlAttachWithHint(ppRoot, nullptr, pNode, pfnComparator);
}

static inline void MCF_AvlDetach(const MCF_AvlNodeHeader *pNode) MCF_NOEXCEPT {
	MCF_AvlInternalDetach(pNode);
}

static inline MCF_AvlNodeHeader *MCF_AvlLowerBound(const MCF_AvlRoot *ppRoot,
	MCF_STD intptr_t nOther, MCF_AvlComparatorNodeOther pfnComparatorNodeOther)
{
	const MCF_AvlNodeHeader *pRet = nullptr;
	const MCF_AvlNodeHeader *pCur = *ppRoot;
	while(pCur){
		if((*pfnComparatorNodeOther)(pCur, nOther) < 0){
			pCur = pCur->pRight;
		} else {
			pRet = pCur;
			pCur = pCur->pLeft;
		}
	}
	return (MCF_AvlNodeHeader *)pRet;
}

static inline MCF_AvlNodeHeader *MCF_AvlUpperBound(const MCF_AvlRoot *ppRoot,
	MCF_STD intptr_t nOther, MCF_AvlComparatorNodeOther pfnComparatorNodeOther)
{
	const MCF_AvlNodeHeader *pRet = nullptr;
	const MCF_AvlNodeHeader *pCur = *ppRoot;
	while(pCur){
		if((*pfnComparatorNodeOther)(pCur, nOther) <= 0){
			pCur = pCur->pRight;
		} else {
			pRet = pCur;
			pCur = pCur->pLeft;
		}
	}
	return (MCF_AvlNodeHeader *)pRet;
}

static inline MCF_AvlNodeHeader *MCF_AvlFind(
	const MCF_AvlRoot *ppRoot, MCF_STD intptr_t nOther,
	MCF_AvlComparatorNodeOther pfnComparatorNodeOther)
{
	const MCF_AvlNodeHeader *pCur = *ppRoot;
	while(pCur){
		const int nResult = (*pfnComparatorNodeOther)(pCur, nOther);
		if(nResult < 0){
			pCur = pCur->pRight;
		} else if(nResult > 0){
			pCur = pCur->pLeft;
		} else {
			break;
		}
	}
	return (MCF_AvlNodeHeader *)pCur;
}

static inline void MCF_AvlEqualRange(
	MCF_AvlNodeHeader **ppBegin, MCF_AvlNodeHeader **ppEnd,
	const MCF_AvlRoot *ppRoot, MCF_STD intptr_t nOther,
	MCF_AvlComparatorNodeOther pfnComparatorNodeOther)
{
	const MCF_AvlNodeHeader *const pTop = MCF_AvlFind(ppRoot, nOther, pfnComparatorNodeOther);
	if(!pTop){
		*ppBegin = nullptr;
		*ppEnd = nullptr;
	} else {
		const MCF_AvlNodeHeader *pCur = pTop;
		for(;;){
			const MCF_AvlNodeHeader *const pLower = pCur->pLeft;
			if(!pLower || ((*pfnComparatorNodeOther)(pLower, nOther) < 0)){
				break;
			}
			pCur = pLower;
		}
		*ppBegin = (MCF_AvlNodeHeader *)pCur;

		pCur = pTop;
		for(;;){
			const MCF_AvlNodeHeader *const pUpper = pCur->pRight;
			if(!pUpper || ((*pfnComparatorNodeOther)(pUpper, nOther) > 0)){
				break;
			}
			pCur = pUpper;
		}
		*ppEnd = (MCF_AvlNodeHeader *)(pCur ? pCur->pNext : nullptr);
	}
}

__MCF_CRT_EXTERN_C_END

#endif
