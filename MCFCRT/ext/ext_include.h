// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_EXT_INCLUDE_H_
#define MCF_CRT_EXT_INCLUDE_H_

#include "unref_param.h"
#include "assert.h"
#include "offset_of.h"
#include "expect.h"

#include "../env/_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern char *MCF_strcpyout(char *restrict dst, const char *restrict src) MCF_NOEXCEPT;
extern wchar_t *MCF_wcscpyout(wchar_t *restrict dst, const wchar_t *restrict src) MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
