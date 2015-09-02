// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_EXT_STPCPY_H_
#define __MCF_CRT_EXT_STPCPY_H_

#include "../env/_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern char *MCF_stpcpy(char *restrict __dst, const char *restrict __src) MCF_NOEXCEPT;
extern char *MCF_stppcpy(char *restrict __dst, char *restrict __end, const char *restrict __src) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
