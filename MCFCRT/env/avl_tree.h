// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_AVL_H_
#define __MCFCRT_ENV_AVL_H_

#include "_crtdef.h"
#include "../ext/assert.h"

__MCFCRT_EXTERN_C_BEGIN

typedef struct MCFCRT_tagAvlNodeHeader {
	struct MCFCRT_tagAvlNodeHeader *__pParent;
	struct MCFCRT_tagAvlNodeHeader **__ppRefl;
	struct MCFCRT_tagAvlNodeHeader *__pLeft;
	struct MCFCRT_tagAvlNodeHeader *__pRight;
	MCFCRT_STD size_t __uHeight;
	struct MCFCRT_tagAvlNodeHeader *__pPrev;
	struct MCFCRT_tagAvlNodeHeader *__pNext;
} MCFCRT_AvlNodeHeader, *MCFCRT_AvlRoot;

static inline MCFCRT_AvlNodeHeader *MCFCRT_AvlPrev(const MCFCRT_AvlNodeHeader *__pNode) MCFCRT_NOEXCEPT {
	return __pNode->__pPrev;
}
static inline MCFCRT_AvlNodeHeader *MCFCRT_AvlNext(const MCFCRT_AvlNodeHeader *__pNode) MCFCRT_NOEXCEPT {
	return __pNode->__pNext;
}

extern void MCFCRT_AvlInternalAttach(MCFCRT_AvlNodeHeader *__pNode, MCFCRT_AvlNodeHeader *__pParent, MCFCRT_AvlNodeHeader **__ppRefl) MCFCRT_NOEXCEPT;
extern void MCFCRT_AvlInternalDetach(const MCFCRT_AvlNodeHeader *__pNode) MCFCRT_NOEXCEPT;

__attribute__((__flatten__))
static inline MCFCRT_AvlNodeHeader *MCFCRT_AvlFront(const MCFCRT_AvlRoot *__ppRoot) MCFCRT_NOEXCEPT {
	MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	if(__pCur){
		while(__pCur->__pLeft){
			__pCur = __pCur->__pLeft;
		}
	}
	return __pCur;
}
__attribute__((__flatten__))
static inline MCFCRT_AvlNodeHeader *MCFCRT_AvlBack(const MCFCRT_AvlRoot *__ppRoot) MCFCRT_NOEXCEPT {
	MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	if(__pCur){
		while(__pCur->__pRight){
			__pCur = __pCur->__pRight;
		}
	}
	return __pCur;
}

__attribute__((__flatten__))
static inline void MCFCRT_AvlSwap(MCFCRT_AvlRoot *__ppRoot1, MCFCRT_AvlRoot *__ppRoot2) MCFCRT_NOEXCEPT {
	if(__ppRoot1 == __ppRoot2){
		return;
	}

	MCFCRT_AvlNodeHeader *const __pRoot1 = *__ppRoot1;
	MCFCRT_AvlNodeHeader *const __pRoot2 = *__ppRoot2;

	*__ppRoot2 = __pRoot1;
	if(__pRoot1){
		__pRoot1->__ppRefl = __ppRoot2;
	}

	*__ppRoot1 = __pRoot2;
	if(__pRoot2){
		__pRoot2->__ppRefl = __ppRoot1;
	}
}

// 参考 strcmp()。
typedef int (*MCFCRT_AvlComparatorNodes)(const MCFCRT_AvlNodeHeader *, const MCFCRT_AvlNodeHeader *);
typedef int (*MCFCRT_AvlComparatorNodeOther)(const MCFCRT_AvlNodeHeader *, MCFCRT_STD intptr_t);

__attribute__((__flatten__))
static inline void MCFCRT_AvlAttachWithHint(MCFCRT_AvlRoot *__ppRoot,
	// 如果新节点被插入到该节点前后相邻的位置，则效率被优化。
	// 此处行为和 C++03 以及 C++11 都兼容。
	// __pHint 可以为空。
	const MCFCRT_AvlNodeHeader *__pHint,
	MCFCRT_AvlNodeHeader *__pNode, MCFCRT_AvlComparatorNodes __pfnComparator)
{
	MCFCRT_AvlNodeHeader *__pParent = nullptr;
	MCFCRT_AvlNodeHeader **__ppRefl = __ppRoot;
	if(__pHint){
		MCFCRT_AvlNodeHeader *const __pMutableHint = (MCFCRT_AvlNodeHeader *)__pHint;
		if((*__pfnComparator)(__pNode, __pHint) < 0){
			MCFCRT_AvlNodeHeader *const __pPrev = __pHint->__pPrev;
			if(!__pPrev){
				ASSERT(!__pHint->__pLeft);

				__pParent = __pMutableHint;
				__ppRefl = &(__pMutableHint->__pLeft);
			} else if((*__pfnComparator)(__pNode, __pPrev) >= 0){
				// 条件：  node        <   hint
				//         hint->prev  <=  node
				if(__pPrev->__uHeight < __pHint->__uHeight){
					ASSERT(!__pPrev->__pRight);

					__pParent = __pPrev;
					__ppRefl = &(__pPrev->__pRight);
				} else {
					ASSERT(!__pHint->__pLeft);

					__pParent = __pMutableHint;
					__ppRefl = &(__pMutableHint->__pLeft);
				}
			}
		} else {
			MCFCRT_AvlNodeHeader *const __pNext = __pHint->__pNext;
			if(!__pNext){
				ASSERT(!__pHint->__pRight);

				__pParent = __pMutableHint;
				__ppRefl = &(__pMutableHint->__pRight);
			} else if((*__pfnComparator)(__pNode, __pNext) < 0){
				// 条件：  hint  <=  node
				//         node  <   hint->next
				if(__pHint->__uHeight < __pNext->__uHeight){
					ASSERT(!__pHint->__pRight);

					__pParent = __pMutableHint;
					__ppRefl = &(__pMutableHint->__pRight);
				} else {
					ASSERT(!__pNext->__pLeft);

					__pParent = __pNext;
					__ppRefl = &(__pNext->__pLeft);
				}
			}
		}
	}
	if(!__pParent){
		for(;;){
			MCFCRT_AvlNodeHeader *const __pCur = *__ppRefl;
			if(!__pCur){
				break;
			}
			if((*__pfnComparator)(__pNode, __pCur) < 0){
				__pParent = __pCur;
				__ppRefl = &(__pCur->__pLeft);
			} else {
				__pParent = __pCur;
				__ppRefl = &(__pCur->__pRight);
			}
		}
	}
	MCFCRT_AvlInternalAttach(__pNode, __pParent, __ppRefl);
}

__attribute__((__flatten__))
static inline void MCFCRT_AvlAttach(MCFCRT_AvlRoot *__ppRoot,
	MCFCRT_AvlNodeHeader *__pNode, MCFCRT_AvlComparatorNodes __pfnComparator)
{
	MCFCRT_AvlAttachWithHint(__ppRoot, nullptr, __pNode, __pfnComparator);
}

__attribute__((__flatten__))
static inline void MCFCRT_AvlDetach(const MCFCRT_AvlNodeHeader *__pNode) MCFCRT_NOEXCEPT {
	MCFCRT_AvlInternalDetach(__pNode);
}

__attribute__((__flatten__))
static inline MCFCRT_AvlNodeHeader *MCFCRT_AvlGetLowerBound(const MCFCRT_AvlRoot *__ppRoot,
	MCFCRT_STD intptr_t __nOther, MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther)
{
	const MCFCRT_AvlNodeHeader *__pRet = nullptr;
	const MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	while(__pCur){
		if((*__pfnComparatorNodeOther)(__pCur, __nOther) < 0){
			__pCur = __pCur->__pRight;
		} else {
			__pRet = __pCur;
			__pCur = __pCur->__pLeft;
		}
	}
	return (MCFCRT_AvlNodeHeader *)__pRet;
}

__attribute__((__flatten__))
static inline MCFCRT_AvlNodeHeader *MCFCRT_AvlGetUpperBound(const MCFCRT_AvlRoot *__ppRoot,
	MCFCRT_STD intptr_t __nOther, MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther)
{
	const MCFCRT_AvlNodeHeader *__pRet = nullptr;
	const MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	while(__pCur){
		if((*__pfnComparatorNodeOther)(__pCur, __nOther) <= 0){
			__pCur = __pCur->__pRight;
		} else {
			__pRet = __pCur;
			__pCur = __pCur->__pLeft;
		}
	}
	return (MCFCRT_AvlNodeHeader *)__pRet;
}

__attribute__((__flatten__))
static inline MCFCRT_AvlNodeHeader *MCFCRT_AvlFind(const MCFCRT_AvlRoot *__ppRoot,
	MCFCRT_STD intptr_t __nOther, MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther)
{
	const MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	while(__pCur){
		const int __nResult = (*__pfnComparatorNodeOther)(__pCur, __nOther);
		if(__nResult < 0){
			__pCur = __pCur->__pRight;
		} else if(__nResult > 0){
			__pCur = __pCur->__pLeft;
		} else {
			return (MCFCRT_AvlNodeHeader *)__pCur;
		}
	}
	return nullptr;
}

__attribute__((__flatten__))
static inline void MCFCRT_AvlGetEqualRange(MCFCRT_AvlNodeHeader **__ppLower, MCFCRT_AvlNodeHeader **__ppUpper,
	const MCFCRT_AvlRoot *__ppRoot, MCFCRT_STD intptr_t __nOther, MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther)
{
	const MCFCRT_AvlNodeHeader *const __pTop = MCFCRT_AvlFind(__ppRoot, __nOther, __pfnComparatorNodeOther);
	if(!__pTop){
		*__ppLower = nullptr;
		*__ppUpper = nullptr;
	} else {
		const MCFCRT_AvlNodeHeader *__pLower = nullptr, *__pUpper = nullptr;

		const MCFCRT_AvlNodeHeader *__pCur = __pTop->__pLeft;
		while(__pCur){
			if((*__pfnComparatorNodeOther)(__pCur, __nOther) < 0){
				__pCur = __pCur->__pRight;
			} else {
				__pLower = __pCur;
				__pCur = __pCur->__pLeft;
			}
		}

		__pCur = __pTop->__pRight;
		while(__pCur){
			if((*__pfnComparatorNodeOther)(__pCur, __nOther) <= 0){
				__pCur = __pCur->__pRight;
			} else {
				__pUpper = __pCur;
				__pCur = __pCur->__pLeft;
			}
		}

		*__ppLower = (MCFCRT_AvlNodeHeader *)__pLower;
		*__ppUpper = (MCFCRT_AvlNodeHeader *)__pUpper;
	}
}

__MCFCRT_EXTERN_C_END

#endif
