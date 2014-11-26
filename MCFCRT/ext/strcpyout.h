// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_STRCPYOUT_H_
#define MCF_CRT_STRCPYOUT_H_

#include "../env/_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern char *MCF_strcpyout(char *restrict dst, const char *restrict src) MCF_NOEXCEPT;
extern wchar_t *MCF_wcscpyout(wchar_t *restrict dst, const wchar_t *restrict src) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
