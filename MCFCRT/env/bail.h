// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ENV_BAIL_H_
#define __MCF_CRT_ENV_BAIL_H_

#include "_crtdef.h"

#ifdef __cplusplus
#	include <cstdarg>
#else
#	include <stdarg.h>
#endif

__MCF_CRT_EXTERN_C_BEGIN

extern __attribute__((__noreturn__))
void MCF_CRT_Bail(const wchar_t *__pwszDescription) MCF_NOEXCEPT;

extern __attribute__((__noreturn__))
void MCF_CRT_BailV(const wchar_t *__pwszFormat, MCF_STD va_list __pArgs) MCF_NOEXCEPT;

static inline __attribute__((__noreturn__ /*, __format__(__printf__, 1, 2) */))
void MCF_CRT_BailF(const wchar_t *__pwszFormat, ...) MCF_NOEXCEPT {
	MCF_STD va_list __pArgs;
	va_start(__pArgs, __pwszFormat);
	MCF_CRT_BailV(__pwszFormat, __pArgs);
	va_end(__pArgs);
}

__MCF_CRT_EXTERN_C_END

#endif
