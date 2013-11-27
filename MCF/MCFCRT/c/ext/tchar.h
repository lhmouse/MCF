// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_TCHAR_H__
#define __MCF_CRT_TCHAR_H__

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN char *_strcpyn(char *__restrict__ dst, size_t cap, const char *__restrict__ src);
__MCF_CRT_EXTERN wchar_t *_wcscpyn(wchar_t *__restrict__ dst, size_t cap, const wchar_t *__restrict__ src);
#ifndef _UNICODE
#	define	_tcscpyn	_strcpyn
#else
#	define	_tcscpyn	_wcscpyn
#endif

__MCF_CRT_EXTERN void _strcatp(char *__restrict__ *pdst, const char *end, const char *__restrict__ src);
__MCF_CRT_EXTERN void _wcscatp(wchar_t *__restrict__ *pdst, const wchar_t *end, const wchar_t *__restrict__ src);
#ifndef _UNICODE
#	define	_tcscatp	_strcatp
#else
#	define	_tcscatp	_wcscatp
#endif

__MCF_CRT_EXTERN char *_strtok_r(char *__restrict__ s1, const char *__restrict__ s2, char **ctx);
__MCF_CRT_EXTERN wchar_t *_wcstok_r(wchar_t *__restrict__ s1, const wchar_t *__restrict__ s2, wchar_t **ctx);
#ifndef _UNICODE
#	define	_tcstok_r	_strtok_r
#else
#	define	_tcstok_r	_wcstok_r
#endif

#endif
