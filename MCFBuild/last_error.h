// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_LAST_ERROR_H_
#define MCFBUILD_LAST_ERROR_H_

#include "common.h"
#include <winerror.h>

MCFBUILD_EXTERN_C_BEGIN

// These are wrappers for `GetLastError()` and `SetLastError()`, respectively.
extern unsigned long MCFBUILD_GetLastError(void) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_SetLastError(unsigned long ulErrorCode) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
