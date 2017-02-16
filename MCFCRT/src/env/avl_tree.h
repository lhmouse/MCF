// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_AVL_TREE_H_
#define __MCFCRT_ENV_AVL_TREE_H_

#include "_crtdef.h"

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

extern _MCFCRT_AvlNodeHeader *_MCFCRT_AvlPrev(const _MCFCRT_AvlNodeHeader *__pNode) _MCFCRT_NOEXCEPT;
extern _MCFCRT_AvlNodeHeader *_MCFCRT_AvlNext(const _MCFCRT_AvlNodeHeader *__pNode) _MCFCRT_NOEXCEPT;

extern void _MCFCRT_AvlInternalAttach(_MCFCRT_AvlNodeHeader *__pNode, _MCFCRT_AvlNodeHeader *__pParent, _MCFCRT_AvlNodeHeader **__ppRefl) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_AvlInternalDetach(const _MCFCRT_AvlNodeHeader *__pNode) _MCFCRT_NOEXCEPT;

extern _MCFCRT_AvlNodeHeader *_MCFCRT_AvlFront(const _MCFCRT_AvlRoot *__ppRoot) _MCFCRT_NOEXCEPT;
extern _MCFCRT_AvlNodeHeader *_MCFCRT_AvlBack(const _MCFCRT_AvlRoot *__ppRoot) _MCFCRT_NOEXCEPT;

extern void _MCFCRT_AvlSwap(_MCFCRT_AvlRoot *__ppRoot1, _MCFCRT_AvlRoot *__ppRoot2) _MCFCRT_NOEXCEPT;

// 参考 strcmp()。
typedef int (*_MCFCRT_AvlComparatorNodes)(const _MCFCRT_AvlNodeHeader *, const _MCFCRT_AvlNodeHeader *);
typedef int (*_MCFCRT_AvlComparatorNodeOther)(const _MCFCRT_AvlNodeHeader *, _MCFCRT_STD intptr_t);

// 如果 __pHint 非空并且 __pNode 被插入到 __pHint 前后相邻的位置，则效率被优化。此处行为和 C++03 以及 C++11 都兼容。
extern void _MCFCRT_AvlAttachWithHint(_MCFCRT_AvlRoot *__ppRoot, const _MCFCRT_AvlNodeHeader *__pHint, _MCFCRT_AvlNodeHeader *__pNode, _MCFCRT_AvlComparatorNodes __pfnComparator) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_AvlAttach(_MCFCRT_AvlRoot *__ppRoot, _MCFCRT_AvlNodeHeader *__pNode, _MCFCRT_AvlComparatorNodes __pfnComparator) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_AvlDetach(const _MCFCRT_AvlNodeHeader *__pNode) _MCFCRT_NOEXCEPT;

extern _MCFCRT_AvlNodeHeader *_MCFCRT_AvlGetLowerBound(const _MCFCRT_AvlRoot *__ppRoot,
	_MCFCRT_STD intptr_t __nOther, _MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther) _MCFCRT_NOEXCEPT;
extern _MCFCRT_AvlNodeHeader *_MCFCRT_AvlGetUpperBound(const _MCFCRT_AvlRoot *__ppRoot,
	_MCFCRT_STD intptr_t __nOther, _MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther) _MCFCRT_NOEXCEPT;
extern _MCFCRT_AvlNodeHeader *_MCFCRT_AvlFind(const _MCFCRT_AvlRoot *__ppRoot,
	_MCFCRT_STD intptr_t __nOther, _MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_AvlGetEqualRange(_MCFCRT_AvlNodeHeader **__ppLower, _MCFCRT_AvlNodeHeader **__ppUpper, const _MCFCRT_AvlRoot *__ppRoot,
	_MCFCRT_STD intptr_t __nOther, _MCFCRT_AvlComparatorNodeOther __pfnComparatorNodeOther) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#ifndef __MCFCRT_AVL_TREE_INLINE_OR_EXTERN
#	define __MCFCRT_AVL_TREE_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif
#include "_avl_tree_inl.h"

#endif
