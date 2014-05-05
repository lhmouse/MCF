// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_HEAP_DBG_H_
#define MCF_CRT_HEAP_DBG_H_

#if !defined(__MCF_CRT_HEAPDBG_ON) && !defined(NDEBUG)
#	define __MCF_CRT_HEAPDBG_ON		1
#endif

#ifdef __MCF_CRT_HEAPDBG_ON

#include "_crtdef.h"
#include "avl_tree.h"

__MCF_EXTERN_C_BEGIN

typedef struct tagHeapDbgBlockInfo {
	MCF_AVL_NODE_HEADER vHeader;

	const void *pContents;
	__MCF_STD size_t uSize;
	const void *pRetAddr;
} __MCF_HEAPDBG_BLOCK_INFO;

extern unsigned long __MCF_CRT_HeapDbgInit(void) __MCF_NOEXCEPT;
extern void __MCF_CRT_HeapDbgUninit(void) __MCF_NOEXCEPT;

extern __MCF_STD size_t __MCF_CRT_HeapDbgGetRawSize(
	__MCF_STD size_t uContentSize
) __MCF_NOEXCEPT;

extern void __MCF_CRT_HeapDbgAddGuardsAndRegister(
	unsigned char **ppContents,
	unsigned char *pRaw,
	__MCF_STD size_t uContentSize,
	const void *pRetAddr
) __MCF_NOEXCEPT;

extern const __MCF_HEAPDBG_BLOCK_INFO *__MCF_CRT_HeapDbgValidate(
	unsigned char **ppRaw,
	unsigned char *pContents,
	const void *pRetAddr
) __MCF_NOEXCEPT;

extern void __MCF_CRT_HeapDbgUnregister(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
) __MCF_NOEXCEPT;

typedef void (*MCF_HEAP_CALLBACK)(
	int,				// 0 分配，1 释放
	const void *,		// 内存块地址
	__MCF_STD size_t,	// 内存块大小
	const void *,		// 返回地址
	__MCF_STD intptr_t	// 回调参数
);

extern void MCF_CRT_HeapSetCallback(
	MCF_HEAP_CALLBACK *pfnOldCallback,
	__MCF_STD intptr_t *pnOldContext,
	MCF_HEAP_CALLBACK pfnNewCallback,
	__MCF_STD intptr_t nNewContext
) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif // NDEBUG

#endif
