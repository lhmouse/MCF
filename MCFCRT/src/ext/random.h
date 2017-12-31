// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_RANDOM_H_
#define __MCFCRT_EXT_RANDOM_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

// [0, UINT32_MAX]
extern _MCFCRT_STD uint32_t _MCFCRT_GetRandom_uint32(void) _MCFCRT_NOEXCEPT;
// [0, UINT64_MAX]
extern _MCFCRT_STD uint64_t _MCFCRT_GetRandom_uint64(void) _MCFCRT_NOEXCEPT;
// [0.0, 1.0)
extern double _MCFCRT_GetRandom_double(void) _MCFCRT_NOEXCEPT;
// [0.0, 1.0l)
extern long double _MCFCRT_GetRandom_long_double(void) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
