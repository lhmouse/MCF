// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_LAST_ERROR_H_
#define __MCFCRT_ENV_LAST_ERROR_H_

#include "_crtdef.h"
#include <winerror.h>

__MCFCRT_EXTERN_C_BEGIN

extern unsigned long MCFCRT_GetLastWin32Error(void) MCFCRT_NOEXCEPT;
extern void MCFCRT_SetLastWin32Error(unsigned long __ulErrorCode) MCFCRT_NOEXCEPT;

extern MCFCRT_STD size_t MCFCRT_GetWin32ErrorDescription(const wchar_t **__ppwszStr, unsigned long __ulErrorCode) MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
