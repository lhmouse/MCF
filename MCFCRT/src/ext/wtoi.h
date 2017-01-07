// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_WTOI_H_
#define __MCFCRT_EXT_WTOI_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

typedef enum __MCFCRT_tag_wtoi_result {
	_MCFCRT_wtoi_result_success        = 0,
	_MCFCRT_wtoi_result_no_digit       = 1,
	_MCFCRT_wtoi_result_would_overflow = 2,
} _MCFCRT_wtoi_result;

extern wchar_t *_MCFCRT_wtoi_d(_MCFCRT_wtoi_result *__peResult, _MCFCRT_STD  intptr_t *__pnValue, const wchar_t *__pwcBuffer) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_wtoi_u(_MCFCRT_wtoi_result *__peResult, _MCFCRT_STD uintptr_t *__puValue, const wchar_t *__pwcBuffer) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_wtoi_x(_MCFCRT_wtoi_result *__peResult, _MCFCRT_STD uintptr_t *__puValue, const wchar_t *__pwcBuffer) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_wtoi_X(_MCFCRT_wtoi_result *__peResult, _MCFCRT_STD uintptr_t *__puValue, const wchar_t *__pwcBuffer) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_wtoi0d(_MCFCRT_wtoi_result *__peResult, _MCFCRT_STD  intptr_t *__pnValue, const wchar_t *__pwcBuffer, unsigned __uMaxDigits) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_wtoi0u(_MCFCRT_wtoi_result *__peResult, _MCFCRT_STD uintptr_t *__puValue, const wchar_t *__pwcBuffer, unsigned __uMaxDigits) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_wtoi0x(_MCFCRT_wtoi_result *__peResult, _MCFCRT_STD uintptr_t *__puValue, const wchar_t *__pwcBuffer, unsigned __uMaxDigits) _MCFCRT_NOEXCEPT;
extern wchar_t *_MCFCRT_wtoi0X(_MCFCRT_wtoi_result *__peResult, _MCFCRT_STD uintptr_t *__puValue, const wchar_t *__pwcBuffer, unsigned __uMaxDigits) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
