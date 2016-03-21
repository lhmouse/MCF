// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_BAIL_H_
#define __MCFCRT_ENV_BAIL_H_

#include "_crtdef.h"

#ifdef __cplusplus
#	include <cstdarg>
#else
#	include <stdarg.h>
#endif

__MCFCRT_EXTERN_C_BEGIN

extern __attribute__((__noreturn__))
void _MCFCRT_Bail(const wchar_t *__pwszDescription) _MCFCRT_NOEXCEPT;

extern __attribute__((__noreturn__))
void _MCFCRT_BailV(const wchar_t *__pwszFormat, _MCFCRT_STD va_list __pArgs) _MCFCRT_NOEXCEPT;

static inline __attribute__((__noreturn__ /*, __format__(__printf__, 1, 2) */))
void _MCFCRT_BailF(const wchar_t *__pwszFormat, ...) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD va_list __pArgs;
	va_start(__pArgs, __pwszFormat);
	_MCFCRT_BailV(__pwszFormat, __pArgs);
	va_end(__pArgs);
}

__MCFCRT_EXTERN_C_END

#endif
