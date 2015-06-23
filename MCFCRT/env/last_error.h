// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_LAST_ERROR_H_
#define __MCF_CRT_LAST_ERROR_H_

#include "_crtdef.h"
#include <winerror.h>

__MCF_CRT_EXTERN_C_BEGIN

extern unsigned long MCF_CRT_GetWin32LastError(void) MCF_NOEXCEPT;
extern void MCF_CRT_SetWin32LastError(unsigned long ulErrorCode) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
