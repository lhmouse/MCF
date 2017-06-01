// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_HEAP_DBG_H_
#define __MCFCRT_ENV_HEAP_DBG_H_

#include "_crtdef.h"

/*
	__MCFCRT_HEAPDBG_LEVEL

	0 禁用。
	1 对 GetLastError() 投毒。
	2 在 1 的基础上，捕获部分越界的内存操作。
	3 在 2 的基础上，捕获全部越界的内存操作及内存泄漏。
	4 在 3 的基础上，添加临界区检查，向新分配和已释放的内存投毒。
*/

#ifndef __MCFCRT_HEAPDBG_LEVEL
#  ifdef NDEBUG
#    define __MCFCRT_HEAPDBG_LEVEL  2
#  else
#    define __MCFCRT_HEAPDBG_LEVEL  4
#  endif
#endif

#define __MCFCRT_REQUIRE_HEAPDBG_LEVEL(lv_)    ((__MCFCRT_HEAPDBG_LEVEL + 0) >= (lv_))

#include "avl_tree.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_HeapDbgInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapDbgUninit(void) _MCFCRT_NOEXCEPT;

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)

extern _MCFCRT_STD size_t __MCFCRT_HeapDbgGetRawSize(_MCFCRT_STD size_t __uContentSize) _MCFCRT_NOEXCEPT;

#  if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(3)

typedef struct __MCFCRT_tagHeapDbgBlockInfo {
	_MCFCRT_AvlNodeHeader __avlhNodeByAddress;

	void *__pAddress;
	_MCFCRT_STD size_t __uSize;
	const void *__pRetAddr;
} __MCFCRT_HeapDbgBlockInfo;

// 失败返回空指针。
extern __MCFCRT_HeapDbgBlockInfo *__MCFCRT_HeapDbgAllocateBlockInfo(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapDbgDeallocateBlockInfo(__MCFCRT_HeapDbgBlockInfo *__pBlockInfo) _MCFCRT_NOEXCEPT;

// 返回 __pAddress，后续的 __MCFCRT_HeapDbgValidateBlock() 使用这个返回值。
extern unsigned char *__MCFCRT_HeapDbgRegisterBlockInfo(__MCFCRT_HeapDbgBlockInfo *__pBlockInfo, unsigned char *__pRaw, _MCFCRT_STD size_t __uContentSize, const void *__pRetAddr) _MCFCRT_NOEXCEPT;
extern __MCFCRT_HeapDbgBlockInfo *__MCFCRT_HeapDbgValidateBlock(unsigned char **__ppRaw, unsigned char *__pAddress, const void *__pRetAddr) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapDbgUnregisterBlockInfo(__MCFCRT_HeapDbgBlockInfo *__pBlockInfo) _MCFCRT_NOEXCEPT;

#  else

extern unsigned char *__MCFCRT_HeapDbgAddBlockGuardsBasic(unsigned char *__pRaw) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapDbgValidateBlockBasic(unsigned char **__ppRaw, unsigned char *__pAddress, const void *__pRetAddr) _MCFCRT_NOEXCEPT;

#  endif

#endif

_MCFCRT_EXTERN_C_END

#endif
