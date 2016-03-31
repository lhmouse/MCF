// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CLOCKS_H_
#define __MCFCRT_ENV_CLOCKS_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

static inline _MCFCRT_STD uint32_t _MCFCRT_ReadTimestampCounter32() _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __u32Ret;
	__asm__ volatile (
		"rdtsc \n"
		: "=a"(__u32Ret) : : "dx"
		);
	return __u32Ret;
}
static inline _MCFCRT_STD uint64_t _MCFCRT_ReadTimestampCounter64() _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint64_t __u64Ret;
	__asm__ volatile (
		"rdtsc \n"
#ifdef _WIN64
		"shl rdx, 32 \n"
		"or rax, rdx \n"
		: "=a"(__u64Ret) : : "dx"
#else
		: "=A"(__u64Ret) : :
#endif
		);
	return __u64Ret;
}

extern _MCFCRT_STD uint64_t _MCFCRT_GetUtcClock() _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD uint64_t _MCFCRT_GetLocalClock() _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD uint64_t _MCFCRT_GetUtcClockFromLocal(_MCFCRT_STD uint64_t __u64LocalClock) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD uint64_t _MCFCRT_GetLocalClockFromUtc(_MCFCRT_STD uint64_t __u64UtcClock) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD uint64_t _MCFCRT_GetFastMonoClock() _MCFCRT_NOEXCEPT;
extern double _MCFCRT_GetHiResMonoClock() _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
