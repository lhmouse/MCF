// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "avl.h"
#include <assert.h>

static inline size_t GetHeight(const AVL_NODE_HEADER *pWhere){
	return (pWhere == NULL) ? 0 : pWhere->uHeight;
}
static inline size_t Max(size_t lhs, size_t rhs){
	return (lhs > rhs) ? lhs : rhs;
}
static void UpdateRecur(AVL_NODE_HEADER *pWhere){
	assert(pWhere != NULL);

	AVL_NODE_HEADER *pNode = pWhere;
	size_t uLeftHeight = GetHeight(pWhere->pLeft);
	size_t uRightHeight = GetHeight(pWhere->pRight);
	for(;;){
		const size_t uOldHeight = pNode->uHeight;

		AVL_NODE_HEADER *const pParent = pNode->pParent;
		AVL_NODE_HEADER **const ppRefl = pNode->ppRefl;
		AVL_NODE_HEADER *const pLeft = pNode->pLeft;
		AVL_NODE_HEADER *const pRight = pNode->pRight;

		if(uLeftHeight > uRightHeight){
			assert(uLeftHeight - uRightHeight <= 2);

			if(uLeftHeight - uRightHeight <= 1){
				pNode->uHeight = uLeftHeight + 1;
			} else {
				assert(pLeft != NULL);

				AVL_NODE_HEADER *const pLL = pLeft->pLeft;
				AVL_NODE_HEADER *const pLR = pLeft->pRight;

				const size_t uLLHeight = GetHeight(pLL);
				const size_t uLRHeight = GetHeight(pLR);
				if(uLLHeight >= uLRHeight){
					assert(pLL != NULL);

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
					assert(pLR != NULL);

					/*-------------+--------------------*\
					|     node     |      __lr__         |
					|     /  \     |     /      \        |
					|  left  right |  left      node     |
					|  /  \        >  /  \      /  \     |
					| ll  lr       | ll  lrl  lrr  right |
					|    /  \      |                     |
					|  lrl  lrr    |                     |
					\*-------------+--------------------*/

					AVL_NODE_HEADER *const pLRL = pLR->pLeft;
					AVL_NODE_HEADER *const pLRR = pLR->pRight;

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
			assert(uRightHeight - uLeftHeight <= 2);

			if(uRightHeight - uLeftHeight <= 1){
				pNode->uHeight = uRightHeight + 1;
			} else {
				assert(pRight != NULL);
				assert(GetHeight(pRight) - GetHeight(pNode->pLeft) == 2);

				AVL_NODE_HEADER *const pRR = pRight->pRight;
				AVL_NODE_HEADER *const pRL = pRight->pLeft;

				const size_t uRRHeight = GetHeight(pRR);
				const size_t uRLHeight = GetHeight(pRL);
				if(uRRHeight >= uRLHeight){
					assert(pRR != NULL);

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
					assert(pRL != NULL);

					AVL_NODE_HEADER *const pRLR = pRL->pRight;
					AVL_NODE_HEADER *const pRLL = pRL->pLeft;

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

__MCF_CRT_EXTERN int AVLAttach(
	AVL_NODE_HEADER **ppRoot,
	intptr_t nKey,
	AVL_NODE_HEADER *pNode
){
	AVL_NODE_HEADER *pParent = NULL;
	AVL_NODE_HEADER **ppIns = ppRoot;
	for(;;){
		AVL_NODE_HEADER *const pNode = *ppIns;
		if(pNode == NULL){
			break;
		}
		if(nKey < pNode->nKey){
			pParent = pNode;
			ppIns = &(pNode->pLeft);
		} else if(nKey > pNode->nKey){
			pParent = pNode;
			ppIns = &(pNode->pRight);
		} else {
			return 0;
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

	return -1;
}
__MCF_CRT_EXTERN void AVLDetach(
	const AVL_NODE_HEADER *pWhere
){
	AVL_NODE_HEADER *const pParent = pWhere->pParent;
	AVL_NODE_HEADER **const ppRefl = pWhere->ppRefl;
	AVL_NODE_HEADER *const pLeft = pWhere->pLeft;
	AVL_NODE_HEADER *const pRight = pWhere->pRight;

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
		AVL_NODE_HEADER *pMaxBefore = pLeft;
		for(;;){
			AVL_NODE_HEADER *const pNext = pMaxBefore->pRight;
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

			AVL_NODE_HEADER *const pMaxBfParent = pMaxBefore->pParent;
			AVL_NODE_HEADER **const ppMaxBfRefl = pMaxBefore->ppRefl;
			AVL_NODE_HEADER *const pMaxBfLeft = pMaxBefore->pLeft;

			assert(pMaxBfParent != NULL);

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
__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLFind(
	const AVL_NODE_HEADER *pRoot,
	intptr_t nKey
){
	const AVL_NODE_HEADER *pCur = pRoot;
	for(;;){
		if(pCur == NULL){
			return NULL;
		} else if(nKey < pCur->nKey){
			pCur = pCur->pLeft;
		} else if(nKey > pCur->nKey){
			pCur = pCur->pRight;
		} else {
			return (AVL_NODE_HEADER *)pCur;
		}
	}
}

__MCF_CRT_EXTERN void AVLSwap(
	AVL_NODE_HEADER **ppRoot1,
	AVL_NODE_HEADER **ppRoot2
){
	AVL_NODE_HEADER *const pRoot1 = *ppRoot1;
	AVL_NODE_HEADER *const pRoot2 = *ppRoot2;

	*ppRoot2 = pRoot1;
	if(pRoot1 != NULL){
		pRoot1->ppRefl = ppRoot2;
	}

	*ppRoot1 = pRoot2;
	if(pRoot2 != NULL){
		pRoot2->ppRefl = ppRoot1;
	}
}

__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLBegin(
	const AVL_NODE_HEADER *pRoot
){
	const AVL_NODE_HEADER *pCur = pRoot;
	if(pCur == NULL){
		return NULL;
	}
	for(;;){
		const AVL_NODE_HEADER *const pLeft = pCur->pLeft;
		if(pLeft == NULL){
			return (AVL_NODE_HEADER *)pCur;
		}
		pCur = pLeft;
	}
}
__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLRBegin(
	const AVL_NODE_HEADER *pRoot
){
	const AVL_NODE_HEADER *pCur = pRoot;
	if(pCur == NULL){
		return NULL;
	}
	for(;;){
		const AVL_NODE_HEADER *const pRight = pCur->pRight;
		if(pRight == NULL){
			return (AVL_NODE_HEADER *)pCur;
		}
		pCur = pRight;
	}
}
__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLNext(
	const AVL_NODE_HEADER *pWhere
){
	const AVL_NODE_HEADER *pCur = pWhere;

	const AVL_NODE_HEADER *const pRight = pCur->pRight;
	if(pRight != NULL){
		pCur = pRight;
		for(;;){
			const AVL_NODE_HEADER *const pLeft = pCur->pLeft;
			if(pLeft == NULL){
				return (AVL_NODE_HEADER *)pCur;
			}
			pCur = pLeft;
		}
	}

	for(;;){
		const AVL_NODE_HEADER *const pParent = pCur->pParent;
		if(pParent == NULL){
			return NULL;
		}
		if(pCur->ppRefl == &(pParent->pLeft)){
			return (AVL_NODE_HEADER *)pParent;
		}
		pCur = pParent;
	}
}
__MCF_CRT_EXTERN AVL_NODE_HEADER *AVLPrev(
	const AVL_NODE_HEADER *pWhere
){
	const AVL_NODE_HEADER *pCur = pWhere;

	const AVL_NODE_HEADER *const pLeft = pCur->pLeft;
	if(pLeft != NULL){
		pCur = pLeft;
		for(;;){
			const AVL_NODE_HEADER *const pRight = pCur->pRight;
			if(pRight == NULL){
				return (AVL_NODE_HEADER *)pCur;
			}
			pCur = pRight;
		}
	}

	for(;;){
		const AVL_NODE_HEADER *const pParent = pCur->pParent;
		if(pParent == NULL){
			return NULL;
		}
		if(pCur->ppRefl == &(pParent->pRight)){
			return (AVL_NODE_HEADER *)pParent;
		}
		pCur = pParent;
	}
}

__MCF_CRT_EXTERN int AVLTraverse(
	const AVL_NODE_HEADER *pRoot,
	int (*pfnCallback)(AVL_NODE_HEADER *, intptr_t),
	intptr_t nParam
){
	if(pRoot != NULL){
		AVL_NODE_HEADER *const pLeft = pRoot->pLeft;
		AVL_NODE_HEADER *const pRight = pRoot->pRight;

		if(AVLTraverse(pLeft, pfnCallback, nParam) == 0){
			return 0;
		}
		if((*pfnCallback)((AVL_NODE_HEADER *)pRoot, nParam) == 0){
			return 0;
		}
		if(AVLTraverse(pRight, pfnCallback, nParam) == 0){
			return 0;
		}
	}
	return -1;
}
