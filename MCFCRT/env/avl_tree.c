// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "avl_tree.h"
#include "../ext/assert.h"

static inline size_t GetHeight(const MCF_AvlNodeHeader *pWhere){
	return pWhere ? pWhere->__uHeight : 0;
}
static inline size_t Max(size_t lhs, size_t rhs){
	return (lhs > rhs) ? lhs : rhs;
}
static void UpdateRecur(MCF_AvlNodeHeader *pWhere){
	ASSERT(pWhere);

	MCF_AvlNodeHeader *pNode = pWhere;
	size_t uLeftHeight = GetHeight(pWhere->__pLeft);
	size_t uRightHeight = GetHeight(pWhere->__pRight);
	for(;;){
		const size_t uOldHeight = pNode->__uHeight;

		MCF_AvlNodeHeader *const pParent = pNode->__pParent;
		MCF_AvlNodeHeader **const ppRefl = pNode->__ppRefl;
		MCF_AvlNodeHeader *const pLeft   = pNode->__pLeft;
		MCF_AvlNodeHeader *const pRight  = pNode->__pRight;

		if(uLeftHeight > uRightHeight){
			ASSERT(uLeftHeight - uRightHeight <= 2);

			if(uLeftHeight - uRightHeight <= 1){
				pNode->__uHeight = uLeftHeight + 1;
			} else {
				ASSERT(pLeft);

				MCF_AvlNodeHeader *const pLL = pLeft->__pLeft;
				MCF_AvlNodeHeader *const pLR = pLeft->__pRight;

				const size_t uLLHeight = GetHeight(pLL);
				const size_t uLRHeight = GetHeight(pLR);
				if(uLLHeight >= uLRHeight){
					ASSERT(pLL);

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
					ASSERT(pLR);

					/*-------------------+----------------------------*\
					|         node       |         __ lr __            |
					|        /    \      |        /        \           |
					|    left      right |    left          node       |
					|   /    \           >   /    \        /    \      |
					| ll      lr         | ll      lrl  lrr      right |
					|        /  \        |                             |
					|     lrl    lrr     |                             |
					\*-------------------+----------------------------*/

					MCF_AvlNodeHeader *const pLRL = pLR->__pLeft;
					MCF_AvlNodeHeader *const pLRR = pLR->__pRight;

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
			ASSERT(uRightHeight - uLeftHeight <= 2);

			if(uRightHeight - uLeftHeight <= 1){
				pNode->__uHeight = uRightHeight + 1;
			} else {
				ASSERT(pRight);
				ASSERT(GetHeight(pRight) - GetHeight(pNode->__pLeft) == 2);

				MCF_AvlNodeHeader *const pRR = pRight->__pRight;
				MCF_AvlNodeHeader *const pRL = pRight->__pLeft;

				const size_t uRRHeight = GetHeight(pRR);
				const size_t uRLHeight = GetHeight(pRL);
				if(uRRHeight >= uRLHeight){
					ASSERT(pRR);

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
					ASSERT(pRL);

					MCF_AvlNodeHeader *const pRLR = pRL->__pRight;
					MCF_AvlNodeHeader *const pRLL = pRL->__pLeft;

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

void MCF_AvlInternalAttach(MCF_AvlNodeHeader *pNode,
	MCF_AvlNodeHeader *pParent, MCF_AvlNodeHeader **ppRefl)
{
	ASSERT(*ppRefl == nullptr);

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
			MCF_AvlNodeHeader *const pPrev = pParent->__pPrev;
			pNode->__pPrev = pPrev;
			pNode->__pNext = pParent;
			pParent->__pPrev = pNode;
			if(pPrev){
				pPrev->__pNext = pNode;
			}
		} else {
			MCF_AvlNodeHeader *const pNext = pParent->__pNext;
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

void MCF_AvlInternalDetach(const MCF_AvlNodeHeader *pNode){
	MCF_AvlNodeHeader *const pParent = pNode->__pParent;
	MCF_AvlNodeHeader **const ppRefl = pNode->__ppRefl;
	MCF_AvlNodeHeader *const pLeft = pNode->__pLeft;
	MCF_AvlNodeHeader *const pRight = pNode->__pRight;
	MCF_AvlNodeHeader *const pPrev = pNode->__pPrev;
	MCF_AvlNodeHeader *const pNext = pNode->__pNext;

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

			MCF_AvlNodeHeader *const pPrevParent = pPrev->__pParent;
			MCF_AvlNodeHeader **const ppPrevRefl = pPrev->__ppRefl;
			MCF_AvlNodeHeader *const pPrevLeft = pPrev->__pLeft;

			ASSERT(pPrevParent && (pPrevParent != pNode));

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
