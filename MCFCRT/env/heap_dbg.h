// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_HEAP_DBG_H_
#define MCF_CRT_HEAP_DBG_H_

#include "_crtdef.h"

/*
	__MCF_CRT_HEAPDBG_LEVEL

	0	禁用。
	1	添加仅对性能造成轻微影响的检查。
	2	在 1 的基础上，向新分配和已释放的内存投毒。
	3	在 2 的基础上，追踪内存块的分配和释放，捕捉无效的内存操作及内存泄漏。
*/

#ifndef __MCF_CRT_HEAPDBG_LEVEL
#	ifdef NDEBUG
#		define __MCF_CRT_HEAPDBG_LEVEL	1
#	else
#		define __MCF_CRT_HEAPDBG_LEVEL	3
#	endif
#endif

#define __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(lv_)	((__MCF_CRT_HEAPDBG_LEVEL + 0) >= lv_)

#include "avl_tree.h"

__MCF_CRT_EXTERN_C_BEGIN

extern bool __MCF_CRT_HeapDbgInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_HeapDbgUninit(void) MCF_NOEXCEPT;

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)

typedef struct __tagMCF_HeapDbgBlockInfo {
	MCF_AvlNodeHeader vHeader;

	void *pContents;
	MCF_STD size_t uSize;
	const void *pRetAddr;
} __MCF_HeapDbgBlockInfo;

extern MCF_STD size_t __MCF_CRT_HeapDbgGetRawSize(MCF_STD size_t uContentSize) MCF_NOEXCEPT;

// 返回 pContents，后续的 __MCF_CRT_HeapDbgValidate() 使用这个返回值。
extern unsigned char *__MCF_CRT_HeapDbgAddGuardsAndRegister(
	unsigned char *pRaw, MCF_STD size_t uContentSize, const void *pRetAddr) MCF_NOEXCEPT;

extern const __MCF_HeapDbgBlockInfo *__MCF_CRT_HeapDbgValidate(
	unsigned char **ppRaw, unsigned char *pContents, const void *pRetAddr) MCF_NOEXCEPT;

extern void __MCF_CRT_HeapDbgUnregister(const __MCF_HeapDbgBlockInfo *pBlockInfo) MCF_NOEXCEPT;

#endif

__MCF_CRT_EXTERN_C_END

#endif
