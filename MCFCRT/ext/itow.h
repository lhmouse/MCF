// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_ITOW_H_
#define __MCFCRT_EXT_ITOW_H_

#include "../env/_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

extern wchar_t *_MCFCRT_itow_d(wchar_t *__pwcBuffer, _MCFCRT_STD  intptr_t  __nValue) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow_u(wchar_t *__pwcBuffer, _MCFCRT_STD uintptr_t  __uValue) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow_x(wchar_t *__pwcBuffer, _MCFCRT_STD uintptr_t  __uValue) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow_X(wchar_t *__pwcBuffer, _MCFCRT_STD uintptr_t  __uValue) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow0d(wchar_t *__pwcBuffer, _MCFCRT_STD  intptr_t  __nValue, unsigned __uMinDigits) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow0u(wchar_t *__pwcBuffer, _MCFCRT_STD uintptr_t  __uValue, unsigned __uMinDigits) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow0x(wchar_t *__pwcBuffer, _MCFCRT_STD uintptr_t  __uValue, unsigned __uMinDigits) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow0X(wchar_t *__pwcBuffer, _MCFCRT_STD uintptr_t  __uValue, unsigned __uMinDigits) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
