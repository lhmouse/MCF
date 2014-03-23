// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

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

// 若 arg0 < arg1 应返回非零值，否则应返回零。
typedef int (*__MCF_AVL_KEY_COMPARER)(__MCF_STD intptr_t, __MCF_STD intptr_t);

static inline __MCF_AVL_NODE_HEADER *__MCF_AvlPrev(const __MCF_AVL_NODE_HEADER *pNode) __MCF_NOEXCEPT {
	return pNode->pPrev;
}
static inline __MCF_AVL_NODE_HEADER *__MCF_AvlNext(const __MCF_AVL_NODE_HEADER *pNode) __MCF_NOEXCEPT {
	return pNode->pNext;
}

extern void __MCF_AvlSwap(__MCF_AVL_PROOT *ppRoot1, __MCF_AVL_PROOT *ppRoot2) __MCF_NOEXCEPT;

void __MCF_AvlInsertNoCheck(
	__MCF_AVL_NODE_HEADER *pNode,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pParent,
	__MCF_AVL_NODE_HEADER **ppIns
) __MCF_NOEXCEPT;

extern void __MCF_AvlAttach(
	__MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode
) __MCF_NOEXCEPT;

extern void __MCF_AvlAttachCustomComp(
	__MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
) __MCF_NOEXCEPT;

extern void __MCF_AvlDetach(
	const __MCF_AVL_NODE_HEADER *pNode
) __MCF_NOEXCEPT;

// Q: 为什么这里是 const __MCF_AVL_NODE_HEADER * 而不是 __MCF_AVL_NODE_HEADER * 呢？
// A: 参考 strchr 函数。
extern __MCF_AVL_NODE_HEADER *__MCF_AvlLowerBound(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;

extern __MCF_AVL_NODE_HEADER *__MCF_AvlLowerBoundCustomComp(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nOther,
	__MCF_AVL_KEY_COMPARER pfnComparerKeyOther,
	__MCF_AVL_KEY_COMPARER pfnComparerOtherKey
) __MCF_NOEXCEPT;

extern __MCF_AVL_NODE_HEADER *__MCF_AvlUpperBound(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;

extern __MCF_AVL_NODE_HEADER *__MCF_AvlUpperBoundCustomComp(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nOther,
	__MCF_AVL_KEY_COMPARER pfnComparerKeyOther,
	__MCF_AVL_KEY_COMPARER pfnComparerOtherKey
) __MCF_NOEXCEPT;

extern __MCF_AVL_NODE_HEADER *__MCF_AvlFind(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;

extern __MCF_AVL_NODE_HEADER *__MCF_AvlFindCustomComp(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nOther,
	__MCF_AVL_KEY_COMPARER pfnComparerKeyOther,
	__MCF_AVL_KEY_COMPARER pfnComparerOtherKey
) __MCF_NOEXCEPT;

extern void __MCF_AvlEqualRange(
	__MCF_AVL_NODE_HEADER **ppFrom,
	__MCF_AVL_NODE_HEADER **ppTo,
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;

extern void __MCF_AvlEqualRangeCustomComp(
	__MCF_AVL_NODE_HEADER **ppFrom,
	__MCF_AVL_NODE_HEADER **ppTo,
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nOther,
	__MCF_AVL_KEY_COMPARER pfnComparerKeyOther,
	__MCF_AVL_KEY_COMPARER pfnComparerOtherKey
) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#ifdef __cplusplus

#include <functional>

namespace MCF {

static inline const __MCF_AVL_NODE_HEADER *AvlPrev(const __MCF_AVL_NODE_HEADER *pNode) noexcept {
	return pNode->pPrev;
}
static inline __MCF_AVL_NODE_HEADER *AvlPrev(__MCF_AVL_NODE_HEADER *pNode) noexcept {
	return pNode->pPrev;
}

static inline const __MCF_AVL_NODE_HEADER *AvlNext(const __MCF_AVL_NODE_HEADER *pNode) noexcept {
	return pNode->pNext;
}
static inline __MCF_AVL_NODE_HEADER *AvlNext(__MCF_AVL_NODE_HEADER *pNode) noexcept {
	return pNode->pNext;
}

static inline void AvlSwap(__MCF_AVL_PROOT *ppRoot1, __MCF_AVL_PROOT *ppRoot2) noexcept {
	return ::__MCF_AvlSwap(ppRoot1, ppRoot2);
}

template<class Comparer_t>
inline void AvlAttach(
	__MCF_AVL_PROOT *ppRoot,
	std::intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode,
	Comparer_t &&vComparer = std::less<std::intptr_t>()
) noexcept(noexcept(vComparer(0, 0))) {
	__MCF_AVL_NODE_HEADER *pParent = nullptr;
	__MCF_AVL_NODE_HEADER **ppIns = ppRoot;
	for(;;){
		__MCF_AVL_NODE_HEADER *const pCur = *ppIns;
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
	::__MCF_AvlInsertNoCheck(pNode, nKey, pParent, ppIns);
}

static inline void AvlDetach(
	const __MCF_AVL_NODE_HEADER *pNode
) noexcept {
	return ::__MCF_AvlDetach(pNode);
}

template<class ComparerKeyOther_t, class ComparerOtherKey_t>
inline __MCF_AVL_NODE_HEADER *AvlLowerBound(
	const __MCF_AVL_PROOT *ppRoot,
	std::intptr_t nOther,
	ComparerKeyOther_t &&vComparerKeyOther = std::less<std::intptr_t>(),
	ComparerOtherKey_t && /* vComparerOtherKey */ = std::less<std::intptr_t>()
) noexcept(noexcept(vComparerKeyOther(0, 0)) /* && noexcept(vComparerOtherKey(0, 0)) */) {
	const __MCF_AVL_NODE_HEADER *pRet = nullptr;
	const __MCF_AVL_NODE_HEADER *pCur = *ppRoot;
	while(pCur){
		if(vComparerKeyOther(pCur->nKey, nOther)){
			pCur = pCur->pRight;
		} else {
			pRet = pCur;
			pCur = pCur->pLeft;
		}
	}
	return (__MCF_AVL_NODE_HEADER *)pRet;
}

template<class ComparerKeyOther_t, class ComparerOtherKey_t>
inline __MCF_AVL_NODE_HEADER *AvlUpperBound(
	const __MCF_AVL_PROOT *ppRoot,
	std::intptr_t nOther,
	ComparerKeyOther_t && /* vComparerKeyOther */ = std::less<std::intptr_t>(),
	ComparerOtherKey_t &&vComparerOtherKey = std::less<std::intptr_t>()
) noexcept(/* noexcept(vComparerKeyOther(0, 0)) && */ noexcept(vComparerOtherKey(0, 0))) {
	const __MCF_AVL_NODE_HEADER *pRet = NULL;
	const __MCF_AVL_NODE_HEADER *pCur = *ppRoot;
	while(pCur){
		if(!vComparerOtherKey(nOther, pCur->nKey)){
			pCur = pCur->pRight;
		} else {
			pRet = pCur;
			pCur = pCur->pLeft;
		}
	}
	return (__MCF_AVL_NODE_HEADER *)pRet;
}

template<class ComparerKeyOther_t, class ComparerOtherKey_t>
inline __MCF_AVL_NODE_HEADER *AvlFind(
	const __MCF_AVL_PROOT *ppRoot,
	std::intptr_t nOther,
	ComparerKeyOther_t &&vComparerKeyOther = std::less<std::intptr_t>(),
	ComparerOtherKey_t &&vComparerOtherKey = std::less<std::intptr_t>()
) noexcept(noexcept(vComparerKeyOther(0, 0)) && noexcept(vComparerOtherKey(0, 0))) {
	const __MCF_AVL_NODE_HEADER *pCur = *ppRoot;
	while(pCur){
		if(vComparerKeyOther(pCur->nKey, nOther)){
			pCur = pCur->pRight;
		} else if(vComparerOtherKey(nOther, pCur->nKey)){
			pCur = pCur->pLeft;
		} else {
			break;
		}
	}
	return (__MCF_AVL_NODE_HEADER *)pCur;
}

template<class ComparerKeyOther_t, class ComparerOtherKey_t>
inline void AvlEqualRange(
	__MCF_AVL_NODE_HEADER **ppFrom,
	__MCF_AVL_NODE_HEADER **ppTo,
	const __MCF_AVL_PROOT *ppRoot,
	std::intptr_t nOther,
	ComparerKeyOther_t &&vComparerKeyOther = std::less<std::intptr_t>(),
	ComparerOtherKey_t &&vComparerOtherKey = std::less<std::intptr_t>()
) noexcept(noexcept(vComparerKeyOther(0, 0)) && noexcept(vComparerOtherKey(0, 0))) {
	const __MCF_AVL_NODE_HEADER *const pTop = AvlFind(
		ppRoot,
		nOther,
		std::move(vComparerKeyOther),
		std::move(vComparerOtherKey)
	);
	if(pTop){
		const __MCF_AVL_NODE_HEADER *pCur = pTop;
		for(;;){
			const __MCF_AVL_NODE_HEADER *const pLower = pCur->pLeft;
			if(!pLower || vComparerKeyOther(pLower->nKey, nOther)){
				break;
			}
			pCur = pLower;
		}
		*ppFrom = (__MCF_AVL_NODE_HEADER *)pCur;

		pCur = pTop;
		for(;;){
			const __MCF_AVL_NODE_HEADER *const pUpper = pCur->pRight;
			if(!pUpper || vComparerOtherKey(nOther, pUpper->nKey)){
				break;
			}
			pCur = pUpper;
		}
		*ppTo = (__MCF_AVL_NODE_HEADER *)(pCur ? pCur->pNext : NULL);
	} else {
		*ppFrom = NULL;
		*ppTo = NULL;
	}
}

}

#endif

#endif
