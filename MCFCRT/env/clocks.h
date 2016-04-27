// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CLOCKS_H_
#define __MCFCRT_ENV_CLOCKS_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

static inline _MCFCRT_STD uint32_t _MCFCRT_ReadTimestampCounter32() _MCFCRT_NOEXCEPT {
	return (_MCFCRT_STD uint32_t)__builtin_ia32_rdtsc();
}
static inline _MCFCRT_STD uint64_t _MCFCRT_ReadTimestampCounter64() _MCFCRT_NOEXCEPT {
	return __builtin_ia32_rdtsc();
}

extern _MCFCRT_STD uint64_t _MCFCRT_GetUtcClock() _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD uint64_t _MCFCRT_GetLocalClock() _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD uint64_t _MCFCRT_GetUtcClockFromLocal(_MCFCRT_STD uint64_t __u64LocalClock) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD uint64_t _MCFCRT_GetLocalClockFromUtc(_MCFCRT_STD uint64_t __u64UtcClock) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD uint64_t _MCFCRT_GetFastMonoClock() _MCFCRT_NOEXCEPT;
extern double _MCFCRT_GetHiResMonoClock() _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
