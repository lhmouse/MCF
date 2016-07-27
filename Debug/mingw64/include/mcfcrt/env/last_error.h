// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_LAST_ERROR_H_
#define __MCFCRT_ENV_LAST_ERROR_H_

#include "_crtdef.h"
#include <winerror.h>

_MCFCRT_EXTERN_C_BEGIN

extern unsigned long _MCFCRT_GetLastWin32Error(void) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SetLastWin32Error(unsigned long __ulErrorCode) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD size_t _MCFCRT_GetWin32ErrorDescription(const wchar_t **__ppwszStr, unsigned long __ulErrorCode) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
