#include <MCFCRT/MCFCRT.h>
#include <MCFCRT/env/avl_tree.h>
#include <MCF/Core/Thread.hpp>
#include <MCF/Components/EventDriver.hpp>
#include <cstdio>

unsigned int MCFMain(){
	__MCF_AVL_PROOT pRoot = NULL;
	for(int i = 0; i < 10; ++i){
		__MCF_AvlAttach(&pRoot, i % 3, new __MCF_AVL_NODE_HEADER);
	}

	__MCF_AVL_NODE_HEADER *pFrom, *pTo;
	__MCF_AvlEqualRange(&pFrom, &pTo, &pRoot, 0);
	std::printf("from = %p, to = %p\n", pFrom, pTo);
	while(pFrom != pTo){
		std::printf("%d\n", pFrom->nKey);
		pFrom = __MCF_AvlNext(pFrom);
	}

	while(pRoot){
		const auto p = pRoot;
		__MCF_AvlDetach(p);
		delete p;
	}

	return 0;
}
