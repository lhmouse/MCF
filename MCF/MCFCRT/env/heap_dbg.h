// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_HEAP_DBG_H__
#define __MCF_CRT_HEAP_DBG_H__

#ifndef NDEBUG

#include "_crtdef.h"
#include "avl.h"

#define __MCF_CRT_HEAPDBG_ON	1

typedef struct tagHeapDbgBlockInfo {
	__MCF_AVL_NODE_HEADER __MCF_AVLNodeHeader;

	__MCF_STD size_t uSize;
	const void *pRetAddr;
} __MCF_HEAPDBG_BLOCK_INFO;

__MCF_CRT_EXTERN unsigned long __MCF_CRT_HeapDbgInitContext();
__MCF_CRT_EXTERN void __MCF_CRT_HeapDbgUninitContext();

__MCF_CRT_EXTERN __MCF_STD size_t __MCF_CRT_HeapDbgGetRawSize(
	__MCF_STD size_t uContentSize
);
__MCF_CRT_EXTERN void __MCF_CRT_HeapDbgAddGuardsAndRegister(
	unsigned char **ppContents,
	unsigned char *pRaw,
	__MCF_STD size_t uContentSize,
	const void *pRetAddr
);
__MCF_CRT_EXTERN const __MCF_HEAPDBG_BLOCK_INFO *__MCF_CRT_HeapDbgValidate(
	unsigned char **ppRaw,
	unsigned char *pContents,
	const void *pRetAddr
);
__MCF_CRT_EXTERN const unsigned char *__MCF_CRT_HeapDbgGetContents(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
);
__MCF_CRT_EXTERN void __MCF_CRT_HeapDbgUnregister(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
);

#endif

#endif
