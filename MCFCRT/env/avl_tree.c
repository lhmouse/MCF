// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "avl_tree.h"
#include "../ext/assert.h"

static inline size_t GetHeight(const _MCFCRT_AvlNodeHeader *pWhere){
	return pWhere ? pWhere->__uHeight : 0;
}
static inline size_t Max(size_t lhs, size_t rhs){
	return (lhs > rhs) ? lhs : rhs;
}
static void UpdateRecur(_MCFCRT_AvlNodeHeader *pWhere){
	_MCFCRT_ASSERT(pWhere);

	_MCFCRT_AvlNodeHeader *pNode = pWhere;
	size_t uLeftHeight = GetHeight(pWhere->__pLeft);
	size_t uRightHeight = GetHeight(pWhere->__pRight);
	for(;;){
		const size_t uOldHeight = pNode->__uHeight;

		_MCFCRT_AvlNodeHeader *const pParent = pNode->__pParent;
		_MCFCRT_AvlNodeHeader **const ppRefl = pNode->__ppRefl;
		_MCFCRT_AvlNodeHeader *const pLeft   = pNode->__pLeft;
		_MCFCRT_AvlNodeHeader *const pRight  = pNode->__pRight;

		if(uLeftHeight > uRightHeight){
			_MCFCRT_ASSERT(uLeftHeight - uRightHeight <= 2);

			if(uLeftHeight - uRightHeight <= 1){
				pNode->__uHeight = uLeftHeight + 1;
			} else {
				_MCFCRT_ASSERT(pLeft);

				_MCFCRT_AvlNodeHeader *const pLL = pLeft->__pLeft;
				_MCFCRT_AvlNodeHeader *const pLR = pLeft->__pRight;

				const size_t uLLHeight = GetHeight(pLL);
				const size_t uLRHeight = GetHeight(pLR);
				if(uLLHeight >= uLRHeight){
					_MCFCRT_ASSERT(pLL);

					/*------------------+-------------------*\
					|        node       |    left            |
					|       /    \      |   /    \           |
					|   left      right > ll      node       |
					|  /    \           |        /    \      |
					| ll     lr         |      lr      right |
					\*------------------+-------------------*/

					if(pLR){
						pLR->__pParent = pNode;
						pLR->__ppRefl = &(pNode->__pLeft);
					}

					pLL->__pParent = pLeft;
					pLL->__ppRefl  = &(pLeft->__pLeft);

					pNode->__pParent = pLeft;
					pNode->__ppRefl  = &(pLeft->__pRight);
					pNode->__pLeft   = pLR;
					// H(lr) >= H(l) - 2   // 平衡二叉树的要求。
					//        = H(r)       // 前置条件。
					pNode->__uHeight = uLRHeight + 1;

					pLeft->__pParent = pParent;
					pLeft->__ppRefl  = ppRefl;
					pLeft->__pRight  = pNode;
					pLeft->__uHeight = Max(pLL->__uHeight, pNode->__uHeight) + 1;

					*ppRefl = pLeft;
					pNode = pLeft;
				} else {
					_MCFCRT_ASSERT(pLR);

					/*-------------------+----------------------------*\
					|         node       |         __ lr __            |
					|        /    \      |        /        \           |
					|    left      right |    left          node       |
					|   /    \           >   /    \        /    \      |
					| ll      lr         | ll      lrl  lrr      right |
					|        /  \        |                             |
					|     lrl    lrr     |                             |
					\*-------------------+----------------------------*/

					_MCFCRT_AvlNodeHeader *const pLRL = pLR->__pLeft;
					_MCFCRT_AvlNodeHeader *const pLRR = pLR->__pRight;

					size_t uLRLHeight = 0;
					if(pLRL){
						pLRL->__pParent = pLeft;
						pLRL->__ppRefl = &(pLeft->__pRight);
						uLRLHeight = pLRL->__uHeight;
					}

					size_t uLRRHeight = 0;
					if(pLRR){
						pLRR->__pParent = pNode;
						pLRR->__ppRefl = &(pNode->__pLeft);
						uLRRHeight = pLRR->__uHeight;
					}

					pLeft->__pParent = pLR;
					pLeft->__ppRefl = &(pLR->__pLeft);
					pLeft->__pRight = pLRL;
					pLeft->__uHeight = Max(GetHeight(pLeft->__pLeft), uLRLHeight) + 1;

					pNode->__pParent = pLR;
					pNode->__ppRefl = &(pLR->__pRight);
					pNode->__pLeft = pLRR;
					pNode->__uHeight = Max(uLRRHeight, GetHeight(pNode->__pRight)) + 1;

					pLR->__pParent = pParent;
					pLR->__ppRefl = ppRefl;
					pLR->__pLeft = pLeft;
					pLR->__pRight = pNode;
					pLR->__uHeight = Max(pLeft->__uHeight, pNode->__uHeight) + 1;

					*ppRefl = pLR;
					pNode = pLR;
				}
			}
		} else {
			_MCFCRT_ASSERT(uRightHeight - uLeftHeight <= 2);

			if(uRightHeight - uLeftHeight <= 1){
				pNode->__uHeight = uRightHeight + 1;
			} else {
				_MCFCRT_ASSERT(pRight);
				_MCFCRT_ASSERT(GetHeight(pRight) - GetHeight(pNode->__pLeft) == 2);

				_MCFCRT_AvlNodeHeader *const pRR = pRight->__pRight;
				_MCFCRT_AvlNodeHeader *const pRL = pRight->__pLeft;

				const size_t uRRHeight = GetHeight(pRR);
				const size_t uRLHeight = GetHeight(pRL);
				if(uRRHeight >= uRLHeight){
					_MCFCRT_ASSERT(pRR);

					if(pRL){
						pRL->__pParent = pNode;
						pRL->__ppRefl = &(pNode->__pRight);
					}

					pRR->__pParent = pRight;
					pRR->__ppRefl = &(pRight->__pRight);

					pNode->__pParent = pRight;
					pNode->__ppRefl = &(pRight->__pLeft);
					pNode->__pRight = pRL;
					pNode->__uHeight = uRLHeight + 1;

					pRight->__pParent = pParent;
					pRight->__ppRefl = ppRefl;
					pRight->__pLeft = pNode;
					pRight->__uHeight = Max(pRR->__uHeight, pNode->__uHeight) + 1;

					*ppRefl = pRight;
					pNode = pRight;
				} else {
					_MCFCRT_ASSERT(pRL);

					_MCFCRT_AvlNodeHeader *const pRLR = pRL->__pRight;
					_MCFCRT_AvlNodeHeader *const pRLL = pRL->__pLeft;

					size_t uRLRHeight = 0;
					if(pRLR){
						pRLR->__pParent = pRight;
						pRLR->__ppRefl = &(pRight->__pLeft);
						uRLRHeight = pRLR->__uHeight;
					}

					size_t uRLLHeight = 0;
					if(pRLL){
						pRLL->__pParent = pNode;
						pRLL->__ppRefl = &(pNode->__pRight);
						uRLLHeight = pRLL->__uHeight;
					}

					pRight->__pParent = pRL;
					pRight->__ppRefl = &(pRL->__pRight);
					pRight->__pLeft = pRLR;
					pRight->__uHeight = Max(GetHeight(pRight->__pRight), uRLRHeight) + 1;

					pNode->__pParent = pRL;
					pNode->__ppRefl = &(pRL->__pLeft);
					pNode->__pRight = pRLL;
					pNode->__uHeight = Max(uRLLHeight, GetHeight(pNode->__pLeft)) + 1;

					pRL->__pParent = pParent;
					pRL->__ppRefl = ppRefl;
					pRL->__pRight = pRight;
					pRL->__pLeft = pNode;
					pRL->__uHeight = Max(pRight->__uHeight, pNode->__uHeight) + 1;

					*ppRefl = pRL;
					pNode = pRL;
				}
			}
		}
		if(!pParent){
			break;
		}

		const size_t uNewHeight = pNode->__uHeight;
		if(uOldHeight == uNewHeight){
			break;
		}

		if(ppRefl == &(pParent->__pLeft)){
			uLeftHeight = uNewHeight;
			uRightHeight = GetHeight(pParent->__pRight);
		} else {
			uLeftHeight = GetHeight(pParent->__pLeft);
			uRightHeight = uNewHeight;
		}
		pNode = pParent;
	}
}

void _MCFCRT_AvlInternalAttach(_MCFCRT_AvlNodeHeader *pNode,
	_MCFCRT_AvlNodeHeader *pParent, _MCFCRT_AvlNodeHeader **ppRefl)
{
	_MCFCRT_ASSERT(*ppRefl == nullptr);

	*ppRefl = pNode;

	pNode->__pParent = pParent;
	pNode->__ppRefl  = ppRefl;
	pNode->__pLeft   = nullptr;
	pNode->__pRight  = nullptr;
	pNode->__uHeight = 1;

	if(!pParent){
		pNode->__pPrev = nullptr;
		pNode->__pNext = nullptr;
	} else {
		if(ppRefl == &(pParent->__pLeft)){
			_MCFCRT_AvlNodeHeader *const pPrev = pParent->__pPrev;
			pNode->__pPrev = pPrev;
			pNode->__pNext = pParent;
			pParent->__pPrev = pNode;
			if(pPrev){
				pPrev->__pNext = pNode;
			}
		} else {
			_MCFCRT_AvlNodeHeader *const pNext = pParent->__pNext;
			pNode->__pPrev = pParent;
			pNode->__pNext = pNext;
			if(pNext){
				pNext->__pPrev = pNode;
			}
			pParent->__pNext = pNode;
		}
		if(pParent){
			UpdateRecur(pParent);
		}
	}
}

void _MCFCRT_AvlInternalDetach(const _MCFCRT_AvlNodeHeader *pNode){
	_MCFCRT_AvlNodeHeader *const pParent = pNode->__pParent;
	_MCFCRT_AvlNodeHeader **const ppRefl = pNode->__ppRefl;
	_MCFCRT_AvlNodeHeader *const pLeft = pNode->__pLeft;
	_MCFCRT_AvlNodeHeader *const pRight = pNode->__pRight;
	_MCFCRT_AvlNodeHeader *const pPrev = pNode->__pPrev;
	_MCFCRT_AvlNodeHeader *const pNext = pNode->__pNext;

	if(!pLeft){
		/*-----------+------*\
		| node       | right |
		|     \      >       |
		|      right |       |
		\*-----------+------*/

		*ppRefl = pRight;

		if(pRight){
			pRight->__pParent = pParent;
			pRight->__ppRefl = ppRefl;
		}

		if(pParent){
			UpdateRecur(pParent);
		}
	} else {
		if(pPrev == pLeft){
			/*------------------+--------------*\
			|        node       |    left       |
			|       /    \      |   /    \      |
			|   left      right > ll      right |
			|  /                |               |
			| ll                |               |
			\*------------------+--------------*/

			*ppRefl = pLeft;

			if(pRight){
				pRight->__pParent = pLeft;
				pRight->__ppRefl = &(pLeft->__pRight);
			}

			pLeft->__pParent = pParent;
			pLeft->__ppRefl  = ppRefl;
			pLeft->__pRight  = pRight;
			pLeft->__uHeight = pNode->__uHeight;

			UpdateRecur(pLeft);
		} else {
			/*--------------------------+--------------------------*\
			|      ____ node ____       |      ____ prev ____       |
			|     /              \      |     /              \      |
			| left                right | left                right |
			|     \                     |     \                     |
			|      prevp                >      prevp                |
			|           \               |           \               |
			|            prev           |            prevl          |
			|           /               |                           |
			|      prevl                |                           |
			\*--------------------------+---------------------------*/

			_MCFCRT_AvlNodeHeader *const pPrevParent = pPrev->__pParent;
			_MCFCRT_AvlNodeHeader **const ppPrevRefl = pPrev->__ppRefl;
			_MCFCRT_AvlNodeHeader *const pPrevLeft = pPrev->__pLeft;

			_MCFCRT_ASSERT(pPrevParent && (pPrevParent != pNode));

			*ppRefl = pPrev;

			pPrev->__pParent = pParent;
			pPrev->__ppRefl  = ppRefl;
			pPrev->__pLeft   = pLeft;
			pPrev->__pRight  = pRight;
			pPrev->__uHeight = pNode->__uHeight;

			pLeft->__pParent = pPrev;
			pLeft->__ppRefl = &(pPrev->__pLeft);

			if(pRight){
				pRight->__pParent = pPrev;
				pRight->__ppRefl = &(pPrev->__pRight);
			}

			*ppPrevRefl = pPrevLeft;

			if(pPrevLeft){
				pPrevLeft->__pParent = pPrevParent;
				pPrevLeft->__ppRefl = ppPrevRefl;
			}

			UpdateRecur(pPrevParent);
		}
	}
	if(pPrev){
		pPrev->__pNext = pNext;
	}
	if(pNext){
		pNext->__pPrev = pPrev;
	}
}
