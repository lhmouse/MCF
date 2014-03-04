// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_BAIL_H__
#define __MCF_CRT_BAIL_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

#ifdef NDEBUG
#	define	__MCF_NORETURN_IF_NDEBUG	__attribute__((noreturn))
#else
#	define	__MCF_NORETURN_IF_NDEBUG
#endif

extern __MCF_NORETURN_IF_NDEBUG void __MCF_CRT_Bail(const wchar_t *pwszDescription) __MCF_NOEXCEPT;
extern __MCF_NORETURN_IF_NDEBUG void __MCF_CRT_BailF(const wchar_t *pwszFormat, ...) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
