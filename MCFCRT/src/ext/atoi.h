// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_ATOI_H_
#define __MCFCRT_EXT_ATOI_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

typedef enum __MCFCRT_tag_atoi_result {
	_MCFCRT_atoi_result_success        = 0,
	_MCFCRT_atoi_result_no_digit       = 1,
	_MCFCRT_atoi_result_would_overflow = 2,
} _MCFCRT_atoi_result;

extern char *_MCFCRT_atoi_d(_MCFCRT_atoi_result *_MCFCRT_RESTRICT __result_out, _MCFCRT_STD  intptr_t *_MCFCRT_RESTRICT __value_out, const char *_MCFCRT_RESTRICT __buffer) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_atoi_u(_MCFCRT_atoi_result *_MCFCRT_RESTRICT __result_out, _MCFCRT_STD uintptr_t *_MCFCRT_RESTRICT __value_out, const char *_MCFCRT_RESTRICT __buffer) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_atoi_x(_MCFCRT_atoi_result *_MCFCRT_RESTRICT __result_out, _MCFCRT_STD uintptr_t *_MCFCRT_RESTRICT __value_out, const char *_MCFCRT_RESTRICT __buffer) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_atoi_X(_MCFCRT_atoi_result *_MCFCRT_RESTRICT __result_out, _MCFCRT_STD uintptr_t *_MCFCRT_RESTRICT __value_out, const char *_MCFCRT_RESTRICT __buffer) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_atoi0d(_MCFCRT_atoi_result *_MCFCRT_RESTRICT __result_out, _MCFCRT_STD  intptr_t *_MCFCRT_RESTRICT __value_out, const char *_MCFCRT_RESTRICT __buffer, unsigned __max_digits) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_atoi0u(_MCFCRT_atoi_result *_MCFCRT_RESTRICT __result_out, _MCFCRT_STD uintptr_t *_MCFCRT_RESTRICT __value_out, const char *_MCFCRT_RESTRICT __buffer, unsigned __max_digits) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_atoi0x(_MCFCRT_atoi_result *_MCFCRT_RESTRICT __result_out, _MCFCRT_STD uintptr_t *_MCFCRT_RESTRICT __value_out, const char *_MCFCRT_RESTRICT __buffer, unsigned __max_digits) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_atoi0X(_MCFCRT_atoi_result *_MCFCRT_RESTRICT __result_out, _MCFCRT_STD uintptr_t *_MCFCRT_RESTRICT __value_out, const char *_MCFCRT_RESTRICT __buffer, unsigned __max_digits) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
