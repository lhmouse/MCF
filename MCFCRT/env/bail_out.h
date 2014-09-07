// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_BAIL_H_
#define MCF_CRT_BAIL_H_

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern __MCF_NORETURN_IF_NDEBUG void MCF_CRT_BailOut(const wchar_t *pwszDescription) MCF_NOEXCEPT;
extern __MCF_NORETURN_IF_NDEBUG void MCF_CRT_BailOutF(const wchar_t *pwszFormat, ...) MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
