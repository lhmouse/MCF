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

#ifdef __cplusplus

#include <functional>

namespace MCF {

typedef ::MCF_AVL_NODE_HEADER	AvlNodeHeader;
typedef ::MCF_AVL_ROOT			AvlRoot;

static inline const AvlNodeHeader *AvlPrev(const AvlNodeHeader *pNode) noexcept {
	return pNode->pPrev;
}
static inline AvlNodeHeader *AvlPrev(AvlNodeHeader *pNode) noexcept {
	return pNode->pPrev;
}

static inline const AvlNodeHeader *AvlNext(const AvlNodeHeader *pNode) noexcept {
	return pNode->pNext;
}
static inline AvlNodeHeader *AvlNext(AvlNodeHeader *pNode) noexcept {
	return pNode->pNext;
}

static inline void AvlSwap(AvlRoot *ppRoot1, AvlRoot *ppRoot2) noexcept {
	return ::MCF_AvlSwap(ppRoot1, ppRoot2);
}

template<class Comparer_t>
inline void AvlAttach(
	AvlRoot *ppRoot,
	std::intptr_t nKey,
	AvlNodeHeader *pNode,
	Comparer_t &&vComparer = std::less<std::intptr_t>()
) noexcept(noexcept(vComparer(0, 0))) {
	AvlNodeHeader *pParent = nullptr;
	AvlNodeHeader **ppIns = ppRoot;
	for(;;){
		AvlNodeHeader *const pCur = *ppIns;
		if(!pCur){
			break;
		}
		if(vComparer(nKey, pCur->nKey)){
			pParent = pCur;
			ppIns = &(pCur->pLeft);
		} else {
			pParent = pCur;
			ppIns = &(pCur->pRight);
		}
	}
	::MCF_AvlInsertNoCheck(pNode, nKey, pParent, ppIns);
}

static inline void AvlDetach(
	const AvlNodeHeader *pNode
) noexcept {
	return ::MCF_AvlDetach(pNode);
}

template<class ComparerKeyOther_t, class ComparerOtherKey_t>
inline AvlNodeHeader *AvlLowerBound(
	const AvlRoot *ppRoot,
	std::intptr_t nOther,
	ComparerKeyOther_t &&vComparerKeyOther = std::less<std::intptr_t>(),
	ComparerOtherKey_t && /* vComparerOtherKey */ = std::less<std::intptr_t>()
) noexcept(noexcept(vComparerKeyOther(0, 0)) /* && noexcept(vComparerOtherKey(0, 0)) */) {
	const AvlNodeHeader *pRet = nullptr;
	const AvlNodeHeader *pCur = *ppRoot;
	while(pCur){
		if(vComparerKeyOther(pCur->nKey, nOther)){
			pCur = pCur->pRight;
		} else {
			pRet = pCur;
			pCur = pCur->pLeft;
		}
	}
	return (AvlNodeHeader *)pRet;
}

template<class ComparerKeyOther_t, class ComparerOtherKey_t>
inline AvlNodeHeader *AvlUpperBound(
	const AvlRoot *ppRoot,
	std::intptr_t nOther,
	ComparerKeyOther_t && /* vComparerKeyOther */ = std::less<std::intptr_t>(),
	ComparerOtherKey_t &&vComparerOtherKey = std::less<std::intptr_t>()
) noexcept(/* noexcept(vComparerKeyOther(0, 0)) && */ noexcept(vComparerOtherKey(0, 0))) {
	const AvlNodeHeader *pRet = NULL;
	const AvlNodeHeader *pCur = *ppRoot;
	while(pCur){
		if(!vComparerOtherKey(nOther, pCur->nKey)){
			pCur = pCur->pRight;
		} else {
			pRet = pCur;
			pCur = pCur->pLeft;
		}
	}
	return (AvlNodeHeader *)pRet;
}

template<class ComparerKeyOther_t, class ComparerOtherKey_t>
inline AvlNodeHeader *AvlFind(
	const AvlRoot *ppRoot,
	std::intptr_t nOther,
	ComparerKeyOther_t &&vComparerKeyOther = std::less<std::intptr_t>(),
	ComparerOtherKey_t &&vComparerOtherKey = std::less<std::intptr_t>()
) noexcept(noexcept(vComparerKeyOther(0, 0)) && noexcept(vComparerOtherKey(0, 0))) {
	const AvlNodeHeader *pCur = *ppRoot;
	while(pCur){
		if(vComparerKeyOther(pCur->nKey, nOther)){
			pCur = pCur->pRight;
		} else if(vComparerOtherKey(nOther, pCur->nKey)){
			pCur = pCur->pLeft;
		} else {
			break;
		}
	}
	return (AvlNodeHeader *)pCur;
}

template<class ComparerKeyOther_t, class ComparerOtherKey_t>
inline void AvlEqualRange(
	AvlNodeHeader **ppFrom,
	AvlNodeHeader **ppTo,
	const AvlRoot *ppRoot,
	std::intptr_t nOther,
	ComparerKeyOther_t &&vComparerKeyOther = std::less<std::intptr_t>(),
	ComparerOtherKey_t &&vComparerOtherKey = std::less<std::intptr_t>()
) noexcept(noexcept(vComparerKeyOther(0, 0)) && noexcept(vComparerOtherKey(0, 0))) {
	const AvlNodeHeader *const pTop = AvlFind(
		ppRoot,
		nOther,
		std::move(vComparerKeyOther),
		std::move(vComparerOtherKey)
	);
	if(pTop){
		const AvlNodeHeader *pCur = pTop;
		for(;;){
			const AvlNodeHeader *const pLower = pCur->pLeft;
			if(!pLower || vComparerKeyOther(pLower->nKey, nOther)){
				break;
			}
			pCur = pLower;
		}
		*ppFrom = (AvlNodeHeader *)pCur;

		pCur = pTop;
		for(;;){
			const AvlNodeHeader *const pUpper = pCur->pRight;
			if(!pUpper || vComparerOtherKey(nOther, pUpper->nKey)){
				break;
			}
			pCur = pUpper;
		}
		*ppTo = (AvlNodeHeader *)(pCur ? pCur->pNext : NULL);
	} else {
		*ppFrom = NULL;
		*ppTo = NULL;
	}
}

}

#endif

#endif
