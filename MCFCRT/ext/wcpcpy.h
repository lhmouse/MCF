// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_WCPCPY_H_
#define __MCFCRT_EXT_WCPCPY_H_

#include "../env/_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

extern wchar_t *_MCFCRT_wcpcpy(wchar_t *restrict __dst, const wchar_t *restrict __src) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_wcppcpy(wchar_t *restrict __dst, wchar_t *restrict __end, const wchar_t *restrict __src) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
