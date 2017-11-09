// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_LAST_ERROR_H_
#define __MCFCRT_ENV_LAST_ERROR_H_

#include "_crtdef.h"
#include <winerror.h>

_MCFCRT_EXTERN_C_BEGIN

extern unsigned long _MCFCRT_GetLastError(void) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SetLastError(unsigned long __ulErrorCode) _MCFCRT_NOEXCEPT;

extern bool _MCFCRT_GetErrorDescription(const wchar_t **_MCFCRT_RESTRICT __ppwszText, _MCFCRT_STD size_t *_MCFCRT_RESTRICT __puLength, unsigned long __ulErrorCode) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
