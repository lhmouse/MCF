// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_TCHAR_H__
#define __MCF_CRT_TCHAR_H__

#include "../../env/_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern char *_strcpyout(char *restrict dst, const char *restrict src) __MCF_NOEXCEPT;
extern wchar_t *_wcscpyout(wchar_t *restrict dst, const wchar_t *restrict src) __MCF_NOEXCEPT;
#ifndef _UNICODE
#	define	_tcscpyout	_strcpyout
#else
#	define	_tcscpyout	_wcscpyout
#endif

extern char *_strtok_r(char *restrict s1, const char *restrict s2, char **ctx) __MCF_NOEXCEPT;
extern wchar_t *_wcstok_r(wchar_t *restrict s1, const wchar_t *restrict s2, wchar_t **ctx) __MCF_NOEXCEPT;
#ifndef _UNICODE
#	define	_tcstok_r	_strtok_r
#else
#	define	_tcstok_r	_wcstok_r
#endif

__MCF_EXTERN_C_END

#endif
