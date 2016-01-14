// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CLOCKS_H_
#define __MCFCRT_ENV_CLOCKS_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

static inline MCFCRT_STD uint32_t MCFCRT_ReadTimestampCounter32() MCFCRT_NOEXCEPT {
	MCFCRT_STD uint32_t __u32Ret;
	__asm__ __volatile__(
		"rdtsc \n"
		: "=a"(__u32Ret) : : "dx"
		);
	return __u32Ret;
}
static inline MCFCRT_STD uint64_t MCFCRT_ReadTimestampCounter64() MCFCRT_NOEXCEPT {
	MCFCRT_STD uint64_t __u64Ret;
	__asm__ __volatile__(
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

extern MCFCRT_STD uint64_t MCFCRT_GetUtcClock() MCFCRT_NOEXCEPT;
extern MCFCRT_STD uint64_t MCFCRT_GetLocalClock() MCFCRT_NOEXCEPT;

extern MCFCRT_STD uint64_t MCFCRT_GetFastMonoClock() MCFCRT_NOEXCEPT;
extern double MCFCRT_GetHiResMonoClock() MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
