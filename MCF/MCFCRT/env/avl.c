// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "avl.h"
#include "../c/ext/assert.h"

static inline size_t GetHeight(const __MCF_AVL_NODE_HEADER *pWhere){
	return (pWhere == NULL) ? 0 : pWhere->uHeight;
}
static inline size_t Max(size_t lhs, size_t rhs){
	return (lhs > rhs) ? lhs : rhs;
}
static void UpdateRecur(__MCF_AVL_NODE_HEADER *pWhere){
	ASSERT(pWhere != NULL);

	__MCF_AVL_NODE_HEADER *pNode = pWhere;
	size_t uLeftHeight = GetHeight(pWhere->pLeft);
	size_t uRightHeight = GetHeight(pWhere->pRight);
	for(;;){
		const size_t uOldHeight = pNode->uHeight;

		__MCF_AVL_NODE_HEADER *const pParent = pNode->pParent;
		__MCF_AVL_NODE_HEADER **const ppRefl = pNode->ppRefl;
		__MCF_AVL_NODE_HEADER *const pLeft = pNode->pLeft;
		__MCF_AVL_NODE_HEADER *const pRight = pNode->pRight;

		if(uLeftHeight > uRightHeight){
			ASSERT(uLeftHeight - uRightHeight <= 2);

			if(uLeftHeight - uRightHeight <= 1){
				pNode->uHeight = uLeftHeight + 1;
			} else {
				ASSERT(pLeft != NULL);

				__MCF_AVL_NODE_HEADER *const pLL = pLeft->pLeft;
				__MCF_AVL_NODE_HEADER *const pLR = pLeft->pRight;

				const size_t uLLHeight = GetHeight(pLL);
				const size_t uLRHeight = GetHeight(pLR);
				if(uLLHeight >= uLRHeight){
					ASSERT(pLL != NULL);

					/*-------------+-------------*\
					|     node     |  left        |
					|     /  \     |  /  \        |
					|  left  right > ll  node     |
					|  /  \        |     /  \     |
					| ll  lr       |    lr  right |
					\*-------------+-------------*/

					if(pLR != NULL){
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
					ASSERT(pLR != NULL);

					/*-------------+--------------------*\
					|     node     |      __lr__         |
					|     /  \     |     /      \        |
					|  left  right |  left      node     |
					|  /  \        >  /  \      /  \     |
					| ll  lr       | ll  lrl  lrr  right |
					|    /  \      |                     |
					|  lrl  lrr    |                     |
					\*-------------+--------------------*/

					__MCF_AVL_NODE_HEADER *const pLRL = pLR->pLeft;
					__MCF_AVL_NODE_HEADER *const pLRR = pLR->pRight;

					size_t uLRLHeight = 0;
					if(pLRL != NULL){
						pLRL->pParent = pLeft;
						pLRL->ppRefl = &(pLeft->pRight);
						uLRLHeight = pLRL->uHeight;
					}

					size_t uLRRHeight = 0;
					if(pLRR != NULL){
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
				ASSERT(pRight != NULL);
				ASSERT(GetHeight(pRight) - GetHeight(pNode->pLeft) == 2);

				__MCF_AVL_NODE_HEADER *const pRR = pRight->pRight;
				__MCF_AVL_NODE_HEADER *const pRL = pRight->pLeft;

				const size_t uRRHeight = GetHeight(pRR);
				const size_t uRLHeight = GetHeight(pRL);
				if(uRRHeight >= uRLHeight){
					ASSERT(pRR != NULL);

					if(pRL != NULL){
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
					ASSERT(pRL != NULL);

					__MCF_AVL_NODE_HEADER *const pRLR = pRL->pRight;
					__MCF_AVL_NODE_HEADER *const pRLL = pRL->pLeft;

					size_t uRLRHeight = 0;
					if(pRLR != NULL){
						pRLR->pParent = pRight;
						pRLR->ppRefl = &(pRight->pLeft);
						uRLRHeight = pRLR->uHeight;
					}

					size_t uRLLHeight = 0;
					if(pRLL != NULL){
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
		if(pParent == NULL){
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

__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLAttach(
	__MCF_AVL_NODE_HEADER **ppRoot,
	intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode
){
	__MCF_AVL_NODE_HEADER *pParent = NULL;
	__MCF_AVL_NODE_HEADER **ppIns = ppRoot;
	for(;;){
		__MCF_AVL_NODE_HEADER *const pCur = *ppIns;
		if(pCur == NULL){
			break;
		}
		if(nKey < pCur->nKey){
			pParent = pCur;
			ppIns = &(pCur->pLeft);
		} else if(pCur->nKey < nKey){
			pParent = pCur;
			ppIns = &(pCur->pRight);
		} else {
			return pCur;
		}
	}
	*ppIns = pNode;

	pNode->nKey		= nKey;
	pNode->pParent	= pParent;
	pNode->ppRefl	= ppIns;
	pNode->pLeft	= NULL;
	pNode->pRight	= NULL;
	pNode->uHeight	= 1;

	if(pParent != NULL){
		UpdateRecur(pParent);
	}
	return NULL;
}
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLAttachCustomComp(
	__MCF_AVL_NODE_HEADER **ppRoot,
	intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
){
	__MCF_AVL_NODE_HEADER *pParent = NULL;
	__MCF_AVL_NODE_HEADER **ppIns = ppRoot;
	for(;;){
		__MCF_AVL_NODE_HEADER *const pCur = *ppIns;
		if(pCur == NULL){
			break;
		}
		if((*pfnKeyComparer)(nKey, pCur->nKey) != 0){
			pParent = pCur;
			ppIns = &(pCur->pLeft);
		} else if((*pfnKeyComparer)(pCur->nKey, nKey) != 0){
			pParent = pCur;
			ppIns = &(pCur->pRight);
		} else {
			return pCur;
		}
	}
	*ppIns = pNode;

	pNode->nKey		= nKey;
	pNode->pParent	= pParent;
	pNode->ppRefl	= ppIns;
	pNode->pLeft	= NULL;
	pNode->pRight	= NULL;
	pNode->uHeight	= 1;

	if(pParent != NULL){
		UpdateRecur(pParent);
	}
	return NULL;
}
__MCF_CRT_EXTERN void __MCF_AVLDetach(
	const __MCF_AVL_NODE_HEADER *pWhere
){
	__MCF_AVL_NODE_HEADER *const pParent = pWhere->pParent;
	__MCF_AVL_NODE_HEADER **const ppRefl = pWhere->ppRefl;
	__MCF_AVL_NODE_HEADER *const pLeft = pWhere->pLeft;
	__MCF_AVL_NODE_HEADER *const pRight = pWhere->pRight;

	if(pLeft == NULL){
		/*----------+------*\
		| where     | right |
		|     \     >       |
		|     right |       |
		\*----------+------*/

		*ppRefl = pRight;

		if(pRight != NULL){
			pRight->pParent = pParent;
			pRight->ppRefl = ppRefl;
		}

		if(pParent != NULL){
			UpdateRecur(pParent);
		}
	} else {
		__MCF_AVL_NODE_HEADER *pMaxBefore = pLeft;
		for(;;){
			__MCF_AVL_NODE_HEADER *const pNext = pMaxBefore->pRight;
			if(pNext == NULL){
				break;
			}
			pMaxBefore = pNext;
		}
		if(pMaxBefore == pLeft){
			/*---------------+------------*\
			|     where      |   left      |
			|     /   \      |   /  \      |
			|  left   right  > ll   right  |
			|  /          \  |          \  |
			| ll          rr |          rr |
			\*---------------+------------*/

			*ppRefl = pLeft;

			if(pRight != NULL){
				pRight->pParent = pLeft;
				pRight->ppRefl = &(pLeft->pRight);
			}

			pLeft->pParent = pParent;
			pLeft->ppRefl = ppRefl;
			pLeft->pRight = pRight;
			pLeft->uHeight = pWhere->uHeight;

			UpdateRecur(pLeft);
		} else {
			/*---------------+---------------*\
			|     where      |     maxbf      |
			|     /   \      |     /   \      |
			|  mbfp   right  |  mbfp   right  |
			|  /  \       \  >  /  \       \  |
			| ll  maxbf   rr | ll  mbl     rr |
			|     /          |                |
			|   mbl          |                |
			\*---------------+---------------*/

			__MCF_AVL_NODE_HEADER *const pMaxBfParent = pMaxBefore->pParent;
			__MCF_AVL_NODE_HEADER **const ppMaxBfRefl = pMaxBefore->ppRefl;
			__MCF_AVL_NODE_HEADER *const pMaxBfLeft = pMaxBefore->pLeft;

			ASSERT(pMaxBfParent != NULL);

			*ppRefl = pMaxBefore;

			pMaxBefore->pParent = pParent;
			pMaxBefore->ppRefl = ppRefl;
			pMaxBefore->pLeft = pLeft;
			pMaxBefore->pRight = pRight;
			pMaxBefore->uHeight = pWhere->uHeight;

			pLeft->pParent = pMaxBefore;
			pLeft->ppRefl = &(pMaxBefore->pLeft);

			if(pRight != NULL){
				pRight->pParent = pMaxBefore;
				pRight->ppRefl = &(pMaxBefore->pRight);
			}

			*ppMaxBfRefl = pMaxBfLeft;

			if(pMaxBfLeft != NULL){
				pMaxBfLeft->pParent = pMaxBfParent;
				pMaxBfLeft->ppRefl = ppMaxBfRefl;
			}

			if(pMaxBfParent != pWhere){
				UpdateRecur(pMaxBfParent);
			}
		}
	}
}
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLFind(
	const __MCF_AVL_NODE_HEADER *pRoot,
	intptr_t nKey
){
	const __MCF_AVL_NODE_HEADER *pCur = pRoot;
	for(;;){
		if(pCur == NULL){
			return NULL;
		}
		if(nKey < pCur->nKey){
			pCur = pCur->pLeft;
		} else if(pCur->nKey < nKey){
			pCur = pCur->pRight;
		} else {
			return (__MCF_AVL_NODE_HEADER *)pCur;
		}
	}
}
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLFindCustomComp(
	const __MCF_AVL_NODE_HEADER *pRoot,
	intptr_t nKey,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
){
	const __MCF_AVL_NODE_HEADER *pCur = pRoot;
	for(;;){
		if(pCur == NULL){
			return NULL;
		}
		if((*pfnKeyComparer)(nKey, pCur->nKey) != 0){
			pCur = pCur->pLeft;
		} else if((*pfnKeyComparer)(pCur->nKey, nKey) != 0){
			pCur = pCur->pRight;
		} else {
			return (__MCF_AVL_NODE_HEADER *)pCur;
		}
	}
}

__MCF_CRT_EXTERN void __MCF_AVLSwap(
	__MCF_AVL_NODE_HEADER **ppRoot1,
	__MCF_AVL_NODE_HEADER **ppRoot2
){
	__MCF_AVL_NODE_HEADER *const pRoot1 = *ppRoot1;
	__MCF_AVL_NODE_HEADER *const pRoot2 = *ppRoot2;

	*ppRoot2 = pRoot1;
	if(pRoot1 != NULL){
		pRoot1->ppRefl = ppRoot2;
	}

	*ppRoot1 = pRoot2;
	if(pRoot2 != NULL){
		pRoot2->ppRefl = ppRoot1;
	}
}

__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLBegin(
	const __MCF_AVL_NODE_HEADER *pRoot
){
	const __MCF_AVL_NODE_HEADER *pCur = pRoot;
	if(pCur == NULL){
		return NULL;
	}
	for(;;){
		const __MCF_AVL_NODE_HEADER *const pLeft = pCur->pLeft;
		if(pLeft == NULL){
			return (__MCF_AVL_NODE_HEADER *)pCur;
		}
		pCur = pLeft;
	}
}
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLRBegin(
	const __MCF_AVL_NODE_HEADER *pRoot
){
	const __MCF_AVL_NODE_HEADER *pCur = pRoot;
	if(pCur == NULL){
		return NULL;
	}
	for(;;){
		const __MCF_AVL_NODE_HEADER *const pRight = pCur->pRight;
		if(pRight == NULL){
			return (__MCF_AVL_NODE_HEADER *)pCur;
		}
		pCur = pRight;
	}
}
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLNext(
	const __MCF_AVL_NODE_HEADER *pWhere
){
	const __MCF_AVL_NODE_HEADER *pCur = pWhere;

	const __MCF_AVL_NODE_HEADER *const pRight = pCur->pRight;
	if(pRight != NULL){
		pCur = pRight;
		for(;;){
			const __MCF_AVL_NODE_HEADER *const pLeft = pCur->pLeft;
			if(pLeft == NULL){
				return (__MCF_AVL_NODE_HEADER *)pCur;
			}
			pCur = pLeft;
		}
	}

	for(;;){
		const __MCF_AVL_NODE_HEADER *const pParent = pCur->pParent;
		if(pParent == NULL){
			return NULL;
		}
		if(pCur->ppRefl == &(pParent->pLeft)){
			return (__MCF_AVL_NODE_HEADER *)pParent;
		}
		pCur = pParent;
	}
}
__MCF_CRT_EXTERN __MCF_AVL_NODE_HEADER *__MCF_AVLPrev(
	const __MCF_AVL_NODE_HEADER *pWhere
){
	const __MCF_AVL_NODE_HEADER *pCur = pWhere;

	const __MCF_AVL_NODE_HEADER *const pLeft = pCur->pLeft;
	if(pLeft != NULL){
		pCur = pLeft;
		for(;;){
			const __MCF_AVL_NODE_HEADER *const pRight = pCur->pRight;
			if(pRight == NULL){
				return (__MCF_AVL_NODE_HEADER *)pCur;
			}
			pCur = pRight;
		}
	}

	for(;;){
		const __MCF_AVL_NODE_HEADER *const pParent = pCur->pParent;
		if(pParent == NULL){
			return NULL;
		}
		if(pCur->ppRefl == &(pParent->pRight)){
			return (__MCF_AVL_NODE_HEADER *)pParent;
		}
		pCur = pParent;
	}
}

__MCF_CRT_EXTERN int __MCF_AVLTraverse(
	const __MCF_AVL_NODE_HEADER *pRoot,
	int (*pfnCallback)(__MCF_AVL_NODE_HEADER *, intptr_t),
	intptr_t nParam
){
	if(pRoot != NULL){
		__MCF_AVL_NODE_HEADER *const pLeft = pRoot->pLeft;
		__MCF_AVL_NODE_HEADER *const pRight = pRoot->pRight;

		if(__MCF_AVLTraverse(pLeft, pfnCallback, nParam) == 0){
			return 0;
		}
		if((*pfnCallback)((__MCF_AVL_NODE_HEADER *)pRoot, nParam) == 0){
			return 0;
		}
		if(__MCF_AVLTraverse(pRight, pfnCallback, nParam) == 0){
			return 0;
		}
	}
	return -1;
}
