// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_BAIL_H_
#define __MCFCRT_ENV_BAIL_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

extern __attribute__((__noreturn__))
void _MCFCRT_Bail(const wchar_t *__pwszDescription) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
