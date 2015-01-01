// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_BAIL_H_
#define MCF_CRT_BAIL_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

#ifdef NDEBUG
#	define MCF_CRT_NORETURN_IF_NDEBUG	__attribute__((__noreturn__))
#else
#	define MCF_CRT_NORETURN_IF_NDEBUG
#endif

extern MCF_CRT_NORETURN_IF_NDEBUG void MCF_CRT_Bail(const wchar_t *pwszDescription) MCF_NOEXCEPT;
extern MCF_CRT_NORETURN_IF_NDEBUG void MCF_CRT_BailF(const wchar_t *pwszFormat, ...) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
