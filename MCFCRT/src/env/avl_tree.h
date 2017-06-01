// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_AVL_TREE_H_
#define __MCFCRT_ENV_AVL_TREE_H_

#include "_crtdef.h"
#include "xassert.h"

#ifndef __MCFCRT_AVL_TREE_INLINE_OR_EXTERN
#  define __MCFCRT_AVL_TREE_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

typedef struct __MCFCRT_tagAvlNodeHeader {
	struct __MCFCRT_tagAvlNodeHeader *__pParent;
	struct __MCFCRT_tagAvlNodeHeader **__ppRefl;
	struct __MCFCRT_tagAvlNodeHeader *__pLeft;
	struct __MCFCRT_tagAvlNodeHeader *__pRight;
	_MCFCRT_STD size_t __uHeight;
	struct __MCFCRT_tagAvlNodeHeader *__pPrev;
	struct __MCFCRT_tagAvlNodeHeader *__pNext;
} _MCFCRT_AvlNodeHeader, *_MCFCRT_AvlRoot;

__MCFCRT_AVL_TREE_INLINE_OR_EXTERN _MCFCRT_AvlNodeHeader *_MCFCRT_AvlPrev(const _MCFCRT_AvlNodeHeader *__pNode) _MCFCRT_NOEXCEPT {
	return __pNode->__pPrev;
}
__MCFCRT_AVL_TREE_INLINE_OR_EXTERN _MCFCRT_AvlNodeHeader *_MCFCRT_AvlNext(const _MCFCRT_AvlNodeHeader *__pNode) _MCFCRT_NOEXCEPT {
	return __pNode->__pNext;
}

extern void _MCFCRT_AvlInternalAttach(_MCFCRT_AvlNodeHeader *__pNode, _MCFCRT_AvlNodeHeader *__pParent, _MCFCRT_AvlNodeHeader **__ppRefl) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_AvlInternalDetach(const _MCFCRT_AvlNodeHeader *__pNode) _MCFCRT_NOEXCEPT;

__MCFCRT_AVL_TREE_INLINE_OR_EXTERN _MCFCRT_AvlNodeHeader *_MCFCRT_AvlFront(const _MCFCRT_AvlRoot *__ppRoot) _MCFCRT_NOEXCEPT {
	_MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	if(__pCur){
		while(__pCur->__pLeft){
			__pCur = __pCur->__pLeft;
		}
	}
	return __pCur;
}
__MCFCRT_AVL_TREE_INLINE_OR_EXTERN _MCFCRT_AvlNodeHeader *_MCFCRT_AvlBack(const _MCFCRT_AvlRoot *__ppRoot) _MCFCRT_NOEXCEPT {
	_MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	if(__pCur){
		while(__pCur->__pRight){
			__pCur = __pCur->__pRight;
		}
	}
	return __pCur;
}

__MCFCRT_AVL_TREE_INLINE_OR_EXTERN void _MCFCRT_AvlSwap(_MCFCRT_AvlRoot *__ppRoot1, _MCFCRT_AvlRoot *__ppRoot2) _MCFCRT_NOEXCEPT {
	if(__ppRoot1 == __ppRoot2){
		return;
	}

	_MCFCRT_AvlNodeHeader *const __pRoot1 = *__ppRoot1;
	_MCFCRT_AvlNodeHeader *const __pRoot2 = *__ppRoot2;

	*__ppRoot2 = __pRoot1;
	if(__pRoot1){
		__pRoot1->__ppRefl = __ppRoot2;
	}

	*__ppRoot1 = __pRoot2;
	if(__pRoot2){
		__pRoot2->__ppRefl = __ppRoot1;
	}
}

// These comparators shall return a negative, zero or positive value if the left operand is less than, equal to or greater than the right operand, respectively.
typedef int (*_MCFCRT_AvlComparatorNodes)(const _MCFCRT_AvlNodeHeader *, const _MCFCRT_AvlNodeHeader *);
typedef int (*_MCFCRT_AvlComparatorNodeOther)(const _MCFCRT_AvlNodeHeader *, _MCFCRT_STD intptr_t);

// Insertion is optimized if `__pHint` is non-null and `__pNode` is inserted to an adjacent position to either side of `__pHint`.
// Such behavior is compatible with both C++98 and C++11.
__MCFCRT_AVL_TREE_INLINE_OR_EXTERN void _MCFCRT_AvlAttachWithHint(_MCFCRT_AvlRoot *__ppRoot, const _MCFCRT_AvlNodeHeader *__pHint, _MCFCRT_AvlNodeHeader *__pNode, _MCFCRT_AvlComparatorNodes __pfnComparator) _MCFCRT_NOEXCEPT {
	_MCFCRT_AvlNodeHeader *__pParent = _MCFCRT_NULLPTR;
	_MCFCRT_AvlNodeHeader **__ppRefl = __ppRoot;
	if(__pHint){
		_MCFCRT_AvlNodeHeader *const __pMutableHint = (_MCFCRT_AvlNodeHeader *)__pHint;
		if((*__pfnComparator)(__pNode, __pHint) < 0){
			_MCFCRT_AvlNodeHeader *const __pPrev = __pHint->__pPrev;
			if(!__pPrev){
				_MCFCRT_ASSERT(!__pHint->__pLeft);

				__pParent = __pMutableHint;
				__ppRefl = &(__pMutableHint->__pLeft);
			} else if((*__pfnComparator)(__pNode, __pPrev) >= 0){
				// Requires: node        <   hint
				//           hint->prev  <=  node
				if(__pPrev->__uHeight < __pHint->__uHeight){
					_MCFCRT_ASSERT(!__pPrev->__pRight);

					__pParent = __pPrev;
					__ppRefl = &(__pPrev->__pRight);
				} else {
					_MCFCRT_ASSERT(!__pHint->__pLeft);

					__pParent = __pMutableHint;
					__ppRefl = &(__pMutableHint->__pLeft);
				}
			}
		} else {
			_MCFCRT_AvlNodeHeader *const __pNext = __pHint->__pNext;
			if(!__pNext){
				_MCFCRT_ASSERT(!__pHint->__pRight);

				__pParent = __pMutableHint;
				__ppRefl = &(__pMutableHint->__pRight);
			} else if((*__pfnComparator)(__pNode, __pNext) < 0){
				// Requires: hint  <=  node
				//           node  <   hint->next
				if(__pHint->__uHeight < __pNext->__uHeight){
					_MCFCRT_ASSERT(!__pHint->__pRight);

					__pParent = __pMutableHint;
					__ppRefl = &(__pMutableHint->__pRight);
				} else {
					_MCFCRT_ASSERT(!__pNext->__pLeft);

					__pParent = __pNext;
					__ppRefl = &(__pNext->__pLeft);
				}
			}
		}
	}
	if(!__pParent){
		for(;;){
			_MCFCRT_AvlNodeHeader *const __pCur = *__ppRefl;
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
	_MCFCRT_AvlInternalAttach(__pNode, __pParent, __ppRefl);
}
__MCFCRT_AVL_TREE_INLINE_OR_EXTERN void _MCFCRT_AvlAttach(_MCFCRT_AvlRoot *__ppRoot, _MCFCRT_AvlNodeHeader *__pNode, _MCFCRT_AvlComparatorNodes __pfnComparator) _MCFCRT_NOEXCEPT {
	_MCFCRT_AvlAttachWithHint(__ppRoot, _MCFCRT_NULLPTR, __pNode, __pfnComparator);
}
__MCFCRT_AVL_TREE_INLINE_OR_EXTERN void _MCFCRT_AvlDetach(const _MCFCRT_AvlNodeHeader *__pNode) _MCFCRT_NOEXCEPT {
	_MCFCRT_AvlInternalDetach(__pNode);
}

__MCFCRT_AVL_TREE_INLINE_OR_EXTERN _MCFCRT_AvlNodeHeader *_MCFCRT_AvlGetLowerBound(const _MCFCRT_AvlRoot *__ppRoot,
	_MCFCRT_STD intptr_t __nOther, _MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther) _MCFCRT_NOEXCEPT
{
	const _MCFCRT_AvlNodeHeader *__pRet = _MCFCRT_NULLPTR;
	const _MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	while(__pCur){
		const int __nResult = (*__pfnComparatorNodeOther)(__pCur, __nOther);
		if(__nResult >= 0){
			__pRet = __pCur;
			__pCur = __pCur->__pLeft;
		} else {
			__pCur = __pCur->__pRight;
		}
	}
	return (_MCFCRT_AvlNodeHeader *)__pRet;
}
__MCFCRT_AVL_TREE_INLINE_OR_EXTERN _MCFCRT_AvlNodeHeader *_MCFCRT_AvlGetUpperBound(const _MCFCRT_AvlRoot *__ppRoot,
	_MCFCRT_STD intptr_t __nOther, _MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther) _MCFCRT_NOEXCEPT
{
	const _MCFCRT_AvlNodeHeader *__pRet = _MCFCRT_NULLPTR;
	const _MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	while(__pCur){
		const int __nResult = (*__pfnComparatorNodeOther)(__pCur, __nOther);
		if(__nResult > 0){
			__pRet = __pCur;
			__pCur = __pCur->__pLeft;
		} else {
			__pCur = __pCur->__pRight;
		}
	}
	return (_MCFCRT_AvlNodeHeader *)__pRet;
}
__MCFCRT_AVL_TREE_INLINE_OR_EXTERN _MCFCRT_AvlNodeHeader *_MCFCRT_AvlFind(const _MCFCRT_AvlRoot *__ppRoot,
	_MCFCRT_STD intptr_t __nOther, _MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther) _MCFCRT_NOEXCEPT
{
	const _MCFCRT_AvlNodeHeader *__pRet = _MCFCRT_NULLPTR;
	const _MCFCRT_AvlNodeHeader *__pCur = *__ppRoot;
	while(__pCur){
		const int __nResult = (*__pfnComparatorNodeOther)(__pCur, __nOther);
		if(__nResult == 0){
			__pRet = __pCur;
			break;
		} else if(__nResult > 0){
			__pCur = __pCur->__pLeft;
		} else {
			__pCur = __pCur->__pRight;
		}
	}
	return (_MCFCRT_AvlNodeHeader *)__pRet;
}
__MCFCRT_AVL_TREE_INLINE_OR_EXTERN void _MCFCRT_AvlGetEqualRange(_MCFCRT_AvlNodeHeader **_MCFCRT_RESTRICT __ppLower, _MCFCRT_AvlNodeHeader **_MCFCRT_RESTRICT __ppUpper,
	const _MCFCRT_AvlRoot *__ppRoot, _MCFCRT_STD intptr_t __nOther, _MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther) _MCFCRT_NOEXCEPT
{
	const _MCFCRT_AvlNodeHeader *__pLower = _MCFCRT_NULLPTR;
	const _MCFCRT_AvlNodeHeader *__pUpper = _MCFCRT_NULLPTR;
	const _MCFCRT_AvlNodeHeader *const __pTop = _MCFCRT_AvlFind(__ppRoot, __nOther, __pfnComparatorNodeOther);
	if(__pTop){
		const _MCFCRT_AvlNodeHeader *__pTest;
		__pTest = _MCFCRT_AvlGetLowerBound(&(__pTop->__pLeft), __nOther, __pfnComparatorNodeOther);
		__pLower = __pTest ? __pTest : __pTop;
		__pTest = _MCFCRT_AvlGetUpperBound(&(__pTop->__pRight), __nOther, __pfnComparatorNodeOther);
		__pUpper = __pTest ? __pTest : __pTop;
	}
	*__ppLower = (_MCFCRT_AvlNodeHeader *)__pLower;
	*__ppUpper = (_MCFCRT_AvlNodeHeader *)__pUpper;
}

_MCFCRT_EXTERN_C_END

#endif
