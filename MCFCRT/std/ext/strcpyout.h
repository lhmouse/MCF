// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_STRCPYOUT_H_
#define MCF_CRT_STRCPYOUT_H_

#include "../../env/_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern char *_strcpyout(char *restrict dst, const char *restrict src) __MCF_NOEXCEPT;
extern wchar_t *_wcscpyout(wchar_t *restrict dst, const wchar_t *restrict src) __MCF_NOEXCEPT;

#ifndef _UNICODE
#	define _tcscpyout	_strcpyout
#else
#	define _tcscpyout	_wcscpyout
#endif

#endif
