// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "avl_tree.h"
#include "../ext/ext_include.h"

static inline size_t GetHeight(const MCF_AVL_NODE_HEADER *pWhere){
	return pWhere ? pWhere->uHeight : 0;
}
static inline size_t Max(size_t lhs, size_t rhs){
	return (lhs > rhs) ? lhs : rhs;
}
static void UpdateRecur(MCF_AVL_NODE_HEADER *pWhere){
	ASSERT(pWhere);

	MCF_AVL_NODE_HEADER *pNode = pWhere;
	size_t uLeftHeight = GetHeight(pWhere->pLeft);
	size_t uRightHeight = GetHeight(pWhere->pRight);
	for(;;){
		const size_t uOldHeight = pNode->uHeight;

		MCF_AVL_NODE_HEADER *const pParent = pNode->pParent;
		MCF_AVL_NODE_HEADER **const ppRefl = pNode->ppRefl;
		MCF_AVL_NODE_HEADER *const pLeft = pNode->pLeft;
		MCF_AVL_NODE_HEADER *const pRight = pNode->pRight;

		if(uLeftHeight > uRightHeight){
			ASSERT(uLeftHeight - uRightHeight <= 2);

			if(uLeftHeight - uRightHeight <= 1){
				pNode->uHeight = uLeftHeight + 1;
			} else {
				ASSERT(pLeft);

				MCF_AVL_NODE_HEADER *const pLL = pLeft->pLeft;
				MCF_AVL_NODE_HEADER *const pLR = pLeft->pRight;

				const size_t uLLHeight = GetHeight(pLL);
				const size_t uLRHeight = GetHeight(pLR);
				if(uLLHeight >= uLRHeight){
					ASSERT(pLL);

					/*-------------+-------------*\
					|     node     |  left        |
					|     /  \     |  /  \        |
					|  left  right > ll  node     |
					|  /  \        |     /  \     |
					| ll  lr       |    lr  right |
					\*-------------+-------------*/

					if(pLR){
						pLR->pParent = pNode;
						pLR->ppRefl = &(pNode->pLeft);
					}

					pLL->pParent = pLeft;
					pLL->ppRefl = &(pLeft->pLeft);

					pNode->pParent = pLeft;
					pNode->ppRefl = &(pLeft->pRight);
					pNode->pLeft = pLR;
					// H(lr) >= H(l) - 2   // 平衡二叉树的要求。
					//        = H(r)       // 前置条件。
					pNode->uHeight = uLRHeight + 1;

					pLeft->pParent = pParent;
					pLeft->ppRefl = ppRefl;
					pLeft->pRight = pNode;
					pLeft->uHeight = Max(pLL->uHeight, pNode->uHeight) + 1;

					*ppRefl = pLeft;
					pNode = pLeft;
				} else {
					ASSERT(pLR);

					/*-------------+--------------------*\
					|     node     |      __lr__         |
					|     /  \     |     /      \        |
					|  left  right |  left      node     |
					|  /  \        >  /  \      /  \     |
					| ll  lr       | ll  lrl  lrr  right |
					|    /  \      |                     |
					|  lrl  lrr    |                     |
					\*-------------+--------------------*/

					MCF_AVL_NODE_HEADER *const pLRL = pLR->pLeft;
					MCF_AVL_NODE_HEADER *const pLRR = pLR->pRight;

					size_t uLRLHeight = 0;
					if(pLRL){
						pLRL->pParent = pLeft;
						pLRL->ppRefl = &(pLeft->pRight);
						uLRLHeight = pLRL->uHeight;
					}

					size_t uLRRHeight = 0;
					if(pLRR){
						pLRR->pParent = pNode;
						pLRR->ppRefl = &(pNode->pLeft);
						uLRRHeight = pLRR->uHeight;
					}

					pLeft->pParent = pLR;
					pLeft->ppRefl = &(pLR->pLeft);
					pLeft->pRight = pLRL;
					pLeft->uHeight = Max(GetHeight(pLeft->pLeft), uLRLHeight) + 1;

					pNode->pParent = pLR;
					pNode->ppRefl = &(pLR->pRight);
					pNode->pLeft = pLRR;
					pNode->uHeight = Max(uLRRHeight, GetHeight(pNode->pRight)) + 1;

					pLR->pParent = pParent;
					pLR->ppRefl = ppRefl;
					pLR->pLeft = pLeft;
					pLR->pRight = pNode;
					pLR->uHeight = Max(pLeft->uHeight, pNode->uHeight) + 1;

					*ppRefl = pLR;
					pNode = pLR;
				}
			}
		} else {
			ASSERT(uRightHeight - uLeftHeight <= 2);

			if(uRightHeight - uLeftHeight <= 1){
				pNode->uHeight = uRightHeight + 1;
			} else {
				ASSERT(pRight);
				ASSERT(GetHeight(pRight) - GetHeight(pNode->pLeft) == 2);

				MCF_AVL_NODE_HEADER *const pRR = pRight->pRight;
				MCF_AVL_NODE_HEADER *const pRL = pRight->pLeft;

				const size_t uRRHeight = GetHeight(pRR);
				const size_t uRLHeight = GetHeight(pRL);
				if(uRRHeight >= uRLHeight){
					ASSERT(pRR);

					if(pRL){
						pRL->pParent = pNode;
						pRL->ppRefl = &(pNode->pRight);
					}

					pRR->pParent = pRight;
					pRR->ppRefl = &(pRight->pRight);

					pNode->pParent = pRight;
					pNode->ppRefl = &(pRight->pLeft);
					pNode->pRight = pRL;
					pNode->uHeight = uRLHeight + 1;

					pRight->pParent = pParent;
					pRight->ppRefl = ppRefl;
					pRight->pLeft = pNode;
					pRight->uHeight = Max(pRR->uHeight, pNode->uHeight) + 1;

					*ppRefl = pRight;
					pNode = pRight;
				} else {
					ASSERT(pRL);

					MCF_AVL_NODE_HEADER *const pRLR = pRL->pRight;
					MCF_AVL_NODE_HEADER *const pRLL = pRL->pLeft;

					size_t uRLRHeight = 0;
					if(pRLR){
						pRLR->pParent = pRight;
						pRLR->ppRefl = &(pRight->pLeft);
						uRLRHeight = pRLR->uHeight;
					}

					size_t uRLLHeight = 0;
					if(pRLL){
						pRLL->pParent = pNode;
						pRLL->ppRefl = &(pNode->pRight);
						uRLLHeight = pRLL->uHeight;
					}

					pRight->pParent = pRL;
					pRight->ppRefl = &(pRL->pRight);
					pRight->pLeft = pRLR;
					pRight->uHeight = Max(GetHeight(pRight->pRight), uRLRHeight) + 1;

					pNode->pParent = pRL;
					pNode->ppRefl = &(pRL->pLeft);
					pNode->pRight = pRLL;
					pNode->uHeight = Max(uRLLHeight, GetHeight(pNode->pLeft)) + 1;

					pRL->pParent = pParent;
					pRL->ppRefl = ppRefl;
					pRL->pRight = pRight;
					pRL->pLeft = pNode;
					pRL->uHeight = Max(pRight->uHeight, pNode->uHeight) + 1;

					*ppRefl = pRL;
					pNode = pRL;
				}
			}
		}
		if(!pParent){
			break;
		}

		const size_t uNewHeight = pNode->uHeight;
		if(uOldHeight == uNewHeight){
			break;
		}

		if(ppRefl == &(pParent->pLeft)){
			uLeftHeight = uNewHeight;
			uRightHeight = GetHeight(pParent->pRight);
		} else {
			uLeftHeight = GetHeight(pParent->pLeft);
			uRightHeight = uNewHeight;
		}
		pNode = pParent;
	}
}

void MCF_AvlSwap(
	MCF_AVL_ROOT *ppRoot1,
	MCF_AVL_ROOT *ppRoot2
){
	if(ppRoot1 == ppRoot2){
		return;
	}

	MCF_AVL_NODE_HEADER *const pRoot1 = *ppRoot1;
	MCF_AVL_NODE_HEADER *const pRoot2 = *ppRoot2;

	*ppRoot2 = pRoot1;
	if(pRoot1){
		pRoot1->ppRefl = ppRoot2;
	}

	*ppRoot1 = pRoot2;
	if(pRoot2){
		pRoot2->ppRefl = ppRoot1;
	}
}

void MCF_AvlAttach(
	MCF_AVL_ROOT *ppRoot,
	MCF_AVL_NODE_HEADER *pNode,
	MCF_AVL_COMPARATOR_NODES pfnComparator
){
	MCF_AVL_NODE_HEADER *pParent = NULL;
	MCF_AVL_NODE_HEADER **ppIns = ppRoot;
	for(;;){
		MCF_AVL_NODE_HEADER *const pCur = *ppIns;
		if(!pCur){
			break;
		}
		if((*pfnComparator)(pNode, pCur)){
			pParent = pCur;
			ppIns = &(pCur->pLeft);
		} else {
			pParent = pCur;
			ppIns = &(pCur->pRight);
		}
	}

	*ppIns = pNode;

	pNode->pParent	= pParent;
	pNode->ppRefl	= ppIns;
	pNode->pLeft	= NULL;
	pNode->pRight	= NULL;
	pNode->uHeight	= 1;

	if(!pParent){
		pNode->pPrev = NULL;
		pNode->pNext = NULL;
	} else {
		if(ppIns == &(pParent->pLeft)){
			MCF_AVL_NODE_HEADER *const pPrev = pParent->pPrev;
			pNode->pPrev = pPrev;
			pNode->pNext = pParent;
			pParent->pPrev = pNode;
			if(pPrev){
				pPrev->pNext = pNode;
			}
		} else {
			MCF_AVL_NODE_HEADER *const pNext = pParent->pNext;
			pNode->pPrev = pParent;
			pNode->pNext = pNext;
			if(pNext){
				pNext->pPrev = pNode;
			}
			pParent->pNext = pNode;
		}
		if(pParent){
			UpdateRecur(pParent);
		}
	}
}

void MCF_AvlAttachHint(
	MCF_AVL_ROOT *ppRoot,
	MCF_AVL_NODE_HEADER *pHint,
	MCF_AVL_NODE_HEADER *pNode,
	MCF_AVL_COMPARATOR_NODES pfnComparator
){
	if(!pHint){
		MCF_AvlAttach(ppRoot, pNode, pfnComparator);
		return;
	}

	MCF_AVL_NODE_HEADER *pParent = NULL;
	MCF_AVL_NODE_HEADER **ppIns = NULL;
	if((*pfnComparator)(pNode, pHint)){
		MCF_AVL_NODE_HEADER *const pPrev = pHint->pPrev;
		if(pPrev){
			// 条件：	node		<	hint
			//			hint->prev	<=	node
			if(!(*pfnComparator)(pNode, pPrev)){
				ASSERT(!pPrev->pRight);

				pParent = pPrev;
				ppIns = &(pPrev->pRight);
			}
		} else {
			ASSERT(!pHint->pLeft);

			pParent = pHint;
			ppIns = &(pHint->pLeft);
		}
	} else {
		MCF_AVL_NODE_HEADER *const pNext = pHint->pNext;
		if(pNext){
			// 条件：	hint	<=	node
			//			node	<	hint->next
			if((*pfnComparator)(pNode, pNext)){
				ASSERT(!pNext->pLeft);

				pParent = pNext;
				ppIns = &(pNext->pLeft);
			}
		} else {
			ASSERT(!pHint->pRight);

			pParent = pHint;
			ppIns = &(pHint->pRight);
		}
	}

	if(!pParent){
		MCF_AvlAttach(ppRoot, pNode, pfnComparator);
		return;
	}

	*ppIns = pNode;

	pNode->pParent	= pParent;
	pNode->ppRefl	= ppIns;
	pNode->pLeft	= NULL;
	pNode->pRight	= NULL;
	pNode->uHeight	= 1;

	if(ppIns == &(pParent->pLeft)){
		MCF_AVL_NODE_HEADER *const pPrev = pParent->pPrev;
		pNode->pPrev = pPrev;
		pNode->pNext = pParent;
		pParent->pPrev = pNode;
		if(pPrev){
			pPrev->pNext = pNode;
		}
	} else {
		MCF_AVL_NODE_HEADER *const pNext = pParent->pNext;
		pNode->pPrev = pParent;
		pNode->pNext = pNext;
		if(pNext){
			pNext->pPrev = pNode;
		}
		pParent->pNext = pNode;
	}

	UpdateRecur(pParent);
}

void MCF_AvlDetach(
	const MCF_AVL_NODE_HEADER *pNode
){
	MCF_AVL_NODE_HEADER *const pParent = pNode->pParent;
	MCF_AVL_NODE_HEADER **const ppRefl = pNode->ppRefl;
	MCF_AVL_NODE_HEADER *const pLeft = pNode->pLeft;
	MCF_AVL_NODE_HEADER *const pRight = pNode->pRight;

	if(!pLeft){
		/*---------+------*\
		| node     | right |
		|    \     >       |
		|    right |       |
		\*---------+------*/

		*ppRefl = pRight;

		if(pRight){
			pRight->pParent = pParent;
			pRight->ppRefl = ppRefl;
		}

		if(pParent){
			UpdateRecur(pParent);
		}
	} else {
		MCF_AVL_NODE_HEADER *pMaxBefore = pLeft;
		for(;;){
			MCF_AVL_NODE_HEADER *const pNext = pMaxBefore->pRight;
			if(!pNext){
				break;
			}
			pMaxBefore = pNext;
		}
		if(pMaxBefore == pLeft){
			/*--------------+------------*\
			|     node      |   left      |
			|     /  \      |   /  \      |
			|  left  right  > ll   right  |
			|  /         \  |          \  |
			| ll         rr |          rr |
			\*--------------+------------*/

			*ppRefl = pLeft;

			if(pRight){
				pRight->pParent = pLeft;
				pRight->ppRefl = &(pLeft->pRight);
			}

			pLeft->pParent	= pParent;
			pLeft->ppRefl	= ppRefl;
			pLeft->pRight	= pRight;
			pLeft->uHeight	= pNode->uHeight;

			UpdateRecur(pLeft);
		} else {
			/*--------------+---------------*\
			|     node      |     maxbf      |
			|     /  \      |     /   \      |
			|  mbfp  right  |  mbfp   right  |
			|  /  \      \  >  /  \       \  |
			| ll  maxbf  rr | ll  mbl     rr |
			|     /         |                |
			|   mbl         |                |
			\*--------------+---------------*/

			MCF_AVL_NODE_HEADER *const pMaxBfParent = pMaxBefore->pParent;
			MCF_AVL_NODE_HEADER **const ppMaxBfRefl = pMaxBefore->ppRefl;
			MCF_AVL_NODE_HEADER *const pMaxBfLeft = pMaxBefore->pLeft;

			ASSERT(pMaxBfParent);

			*ppRefl = pMaxBefore;

			pMaxBefore->pParent	= pParent;
			pMaxBefore->ppRefl	= ppRefl;
			pMaxBefore->pLeft	= pLeft;
			pMaxBefore->pRight	= pRight;
			pMaxBefore->uHeight	= pNode->uHeight;

			pLeft->pParent = pMaxBefore;
			pLeft->ppRefl = &(pMaxBefore->pLeft);

			if(pRight){
				pRight->pParent = pMaxBefore;
				pRight->ppRefl = &(pMaxBefore->pRight);
			}

			*ppMaxBfRefl = pMaxBfLeft;

			if(pMaxBfLeft){
				pMaxBfLeft->pParent = pMaxBfParent;
				pMaxBfLeft->ppRefl = ppMaxBfRefl;
			}

			if(pMaxBfParent != pNode){
				UpdateRecur(pMaxBfParent);
			}
		}
	}

	MCF_AVL_NODE_HEADER *const pPrev = pNode->pPrev;
	MCF_AVL_NODE_HEADER *const pNext = pNode->pNext;
	if(pPrev){
		pPrev->pNext = pNext;
	}
	if(pNext){
		pNext->pPrev = pPrev;
	}
}

MCF_AVL_NODE_HEADER *MCF_AvlLowerBound(
	const MCF_AVL_ROOT *ppRoot,
	intptr_t nOther,
	MCF_AVL_COMPARATOR_NODE_OTHER pfnComparatorNodeOther
){
	const MCF_AVL_NODE_HEADER *pRet = NULL;
	const MCF_AVL_NODE_HEADER *pCur = *ppRoot;
	while(pCur){
		if((*pfnComparatorNodeOther)(pCur, nOther)){
			pCur = pCur->pRight;
		} else {
			pRet = pCur;
			pCur = pCur->pLeft;
		}
	}
	return (MCF_AVL_NODE_HEADER *)pRet;
}

MCF_AVL_NODE_HEADER *MCF_AvlUpperBound(
	const MCF_AVL_ROOT *ppRoot,
	intptr_t nOther,
	MCF_AVL_COMPARATOR_OTHER_NODE pfnComparatorOtherNode
){
	const MCF_AVL_NODE_HEADER *pRet = NULL;
	const MCF_AVL_NODE_HEADER *pCur = *ppRoot;
	while(pCur){
		if(!(*pfnComparatorOtherNode)(nOther, pCur)){
			pCur = pCur->pRight;
		} else {
			pRet = pCur;
			pCur = pCur->pLeft;
		}
	}
	return (MCF_AVL_NODE_HEADER *)pRet;
}

MCF_AVL_NODE_HEADER *MCF_AvlFind(
	const MCF_AVL_ROOT *ppRoot,
	intptr_t nOther,
	MCF_AVL_COMPARATOR_NODE_OTHER pfnComparatorNodeOther,
	MCF_AVL_COMPARATOR_OTHER_NODE pfnComparatorOtherNode
){
	const MCF_AVL_NODE_HEADER *pCur = *ppRoot;
	while(pCur){
		if((*pfnComparatorNodeOther)(pCur, nOther)){
			pCur = pCur->pRight;
		} else if((*pfnComparatorOtherNode)(nOther, pCur)){
			pCur = pCur->pLeft;
		} else {
			break;
		}
	}
	return (MCF_AVL_NODE_HEADER *)pCur;
}

void MCF_AvlEqualRange(
	MCF_AVL_NODE_HEADER **ppBegin,
	MCF_AVL_NODE_HEADER **ppEnd,
	const MCF_AVL_ROOT *ppRoot,
	intptr_t nOther,
	MCF_AVL_COMPARATOR_NODE_OTHER pfnComparatorNodeOther,
	MCF_AVL_COMPARATOR_OTHER_NODE pfnComparatorOtherNode
){
	const MCF_AVL_NODE_HEADER *const pTop = MCF_AvlFind(
		ppRoot,
		nOther,
		pfnComparatorNodeOther,
		pfnComparatorOtherNode
	);
	if(!pTop){
		*ppBegin = NULL;
		*ppEnd = NULL;
	} else {
		const MCF_AVL_NODE_HEADER *pCur = pTop;
		for(;;){
			const MCF_AVL_NODE_HEADER *const pLower = pCur->pLeft;
			if(!pLower || (*pfnComparatorNodeOther)(pLower, nOther)){
				break;
			}
			pCur = pLower;
		}
		*ppBegin = (MCF_AVL_NODE_HEADER *)pCur;

		pCur = pTop;
		for(;;){
			const MCF_AVL_NODE_HEADER *const pUpper = pCur->pRight;
			if(!pUpper || (*pfnComparatorOtherNode)(nOther, pUpper)){
				break;
			}
			pCur = pUpper;
		}
		*ppEnd = (MCF_AVL_NODE_HEADER *)(pCur ? pCur->pNext : NULL);
	}
}
