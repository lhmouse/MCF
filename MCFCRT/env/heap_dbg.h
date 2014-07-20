// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_HEAP_DBG_H_
#define MCF_CRT_HEAP_DBG_H_

#include "_crtdef.h"

#if !defined(__MCF_CRT_HEAPDBG_ON) && !defined(NDEBUG)
#	define __MCF_CRT_HEAPDBG_ON		1
#endif

#ifdef __MCF_CRT_HEAPDBG_ON

#include "avl_tree.h"

__MCF_EXTERN_C_BEGIN

extern bool __MCF_CRT_HeapDbgInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_HeapDbgUninit(void) MCF_NOEXCEPT;

typedef struct MCF_tagHeapDbgBlockInfo {
	MCF_AVL_NODE_HEADER vHeader;

	void *pContents;
	MCF_STD size_t uSize;
	const void *pRetAddr;
} __MCF_HEAPDBG_BLOCK_INFO;

extern MCF_STD size_t __MCF_CRT_HeapDbgGetRawSize(
	MCF_STD size_t uContentSize
) MCF_NOEXCEPT;

extern void __MCF_CRT_HeapDbgAddGuardsAndRegister(
	unsigned char **ppContents,
	unsigned char *pRaw,
	MCF_STD size_t uContentSize,
	const void *pRetAddr
) MCF_NOEXCEPT;

extern const __MCF_HEAPDBG_BLOCK_INFO *__MCF_CRT_HeapDbgValidate(
	unsigned char **ppRaw,
	unsigned char *pContents,
	const void *pRetAddr
) MCF_NOEXCEPT;

extern void __MCF_CRT_HeapDbgUnregister(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
) MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif // __MCF_CRT_HEAPDBG_ON

#endif
