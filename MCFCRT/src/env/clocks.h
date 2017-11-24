// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CLOCKS_H_
#define __MCFCRT_ENV_CLOCKS_H_

#include "_crtdef.h"

#ifndef __MCFCRT_CLOCKS_INLINE_OR_EXTERN
#  define __MCFCRT_CLOCKS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__attribute__((__artificial__))
__MCFCRT_CLOCKS_INLINE_OR_EXTERN _MCFCRT_STD uint32_t _MCFCRT_ReadTimeStampCounter32(void) _MCFCRT_NOEXCEPT {
	return (_MCFCRT_STD uint32_t)__builtin_ia32_rdtsc();
}
__attribute__((__artificial__))
__MCFCRT_CLOCKS_INLINE_OR_EXTERN _MCFCRT_STD uint64_t _MCFCRT_ReadTimeStampCounter64(void) _MCFCRT_NOEXCEPT {
	return __builtin_ia32_rdtsc();
}

extern _MCFCRT_STD uint64_t _MCFCRT_GetUtcClock(void) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD uint64_t _MCFCRT_GetLocalClock(void) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD uint64_t _MCFCRT_GetUtcClockFromLocal(_MCFCRT_STD uint64_t __local) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD uint64_t _MCFCRT_GetLocalClockFromUtc(_MCFCRT_STD uint64_t __utc) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD uint64_t _MCFCRT_GetFastMonoClock(void) _MCFCRT_NOEXCEPT;
extern double _MCFCRT_GetHiResMonoClock(void) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
