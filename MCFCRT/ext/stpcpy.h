// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_STPCPY_H_
#define __MCFCRT_EXT_STPCPY_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern char *_MCFCRT_stpcpy(char *restrict __dst, const char *restrict __src) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_stppcpy(char *restrict __dst, char *restrict __end, const char *restrict __src) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
