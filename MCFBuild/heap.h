// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_HEAP_H_
#define MCFBUILD_HEAP_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

// These functions set the last error code to `ERROR_NOT_ENOUGH_MEMORY` on failure.

extern void *MCFBUILD_HeapAlloc(MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;
extern void *MCFBUILD_HeapRealloc(void *pBlockOld, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_HeapFree(void *pBlock) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
