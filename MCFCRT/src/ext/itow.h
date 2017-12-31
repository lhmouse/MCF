// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_ITOW_H_
#define __MCFCRT_EXT_ITOW_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern wchar_t *_MCFCRT_itow_d(wchar_t *__buffer, _MCFCRT_STD  intptr_t __value) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow_u(wchar_t *__buffer, _MCFCRT_STD uintptr_t __value) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow_x(wchar_t *__buffer, _MCFCRT_STD uintptr_t __value) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow_X(wchar_t *__buffer, _MCFCRT_STD uintptr_t __value) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow0d(wchar_t *__buffer, _MCFCRT_STD  intptr_t __value, unsigned __min_digits) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow0u(wchar_t *__buffer, _MCFCRT_STD uintptr_t __value, unsigned __min_digits) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow0x(wchar_t *__buffer, _MCFCRT_STD uintptr_t __value, unsigned __min_digits) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_itow0X(wchar_t *__buffer, _MCFCRT_STD uintptr_t __value, unsigned __min_digits) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
