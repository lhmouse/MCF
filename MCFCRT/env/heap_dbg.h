// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ENV_HEAP_DBG_H_
#define __MCF_CRT_ENV_HEAP_DBG_H_

#include "_crtdef.h"

/*
	__MCF_CRT_HEAPDBG_LEVEL

	0 禁用。
	1 对 GetLastError() 投毒。
	2 在 1 的基础上，捕获部分越界的内存操作。
	3 在 2 的基础上，捕获全部越界的内存操作及内存泄漏。
	4 在 3 的基础上，添加临界区检查，向新分配和已释放的内存投毒。
*/

#ifndef __MCF_CRT_HEAPDBG_LEVEL
#	ifdef NDEBUG
#		define __MCF_CRT_HEAPDBG_LEVEL	2
#	else
#		define __MCF_CRT_HEAPDBG_LEVEL	4
#	endif
#endif

#define __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(lv_)	((__MCF_CRT_HEAPDBG_LEVEL + 0) >= (lv_))

#include "avl_tree.h"

__MCF_CRT_EXTERN_C_BEGIN

extern bool __MCF_CRT_HeapDbgInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_HeapDbgUninit(void) MCF_NOEXCEPT;

#if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(2)

#	if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)

typedef struct __MCF_tagHeapDbgBlockInfo {
	MCF_AvlNodeHeader __vHeader;

	void *__pContents;
	MCF_STD size_t __uSize;
	const void *__pRetAddr;
} __MCF_HeapDbgBlockInfo;

#	endif

extern MCF_STD size_t __MCF_CRT_HeapDbgGetRawSize(MCF_STD size_t __uContentSize) MCF_NOEXCEPT;

#	if __MCF_CRT_REQUIRE_HEAPDBG_LEVEL(3)

// 失败返回空指针。
extern __MCF_HeapDbgBlockInfo *__MCF_CRT_HeapDbgAllocateBlockInfo() MCF_NOEXCEPT;
extern void __MCF_CRT_HeapDbgDeallocateBlockInfo(__MCF_HeapDbgBlockInfo *__pBlockInfo) MCF_NOEXCEPT;

// 返回 __pContents，后续的 __MCF_CRT_HeapDbgValidateBlock() 使用这个返回值。
extern unsigned char *__MCF_CRT_HeapDbgRegisterBlockInfo(__MCF_HeapDbgBlockInfo *__pBlockInfo, unsigned char *__pRaw, MCF_STD size_t __uContentSize, const void *__pRetAddr) MCF_NOEXCEPT;
extern __MCF_HeapDbgBlockInfo *__MCF_CRT_HeapDbgValidateBlock(unsigned char **__ppRaw, unsigned char *__pContents, const void *__pRetAddr) MCF_NOEXCEPT;
extern void __MCF_CRT_HeapDbgUnregisterBlockInfo(__MCF_HeapDbgBlockInfo *__pBlockInfo) MCF_NOEXCEPT;

#	else

extern unsigned char *__MCF_CRT_HeapDbgRegisterBlockInfo(unsigned char *__pRaw, MCF_STD size_t __uContentSize, const void *__pRetAddr) MCF_NOEXCEPT;
extern void __MCF_CRT_HeapDbgValidateBlock(unsigned char **__ppRaw, unsigned char *__pContents, const void *__pRetAddr) MCF_NOEXCEPT;

#	endif

#endif

__MCF_CRT_EXTERN_C_END

#endif
