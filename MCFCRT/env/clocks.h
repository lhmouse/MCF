// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ENV_CLOCKS_H_
#define __MCF_CRT_ENV_CLOCKS_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

static inline MCF_STD uint32_t MCF_ReadTimestampCounter32() MCF_NOEXCEPT {
	MCF_STD uint32_t __u32Ret;
	__asm__ __volatile__(
		"rdtsc \n"
		: "=a"(__u32Ret) : : "dx"
		);
	return __u32Ret;
}
static inline MCF_STD uint64_t MCF_ReadTimestampCounter64() MCF_NOEXCEPT {
	MCF_STD uint64_t __u64Ret;
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

extern MCF_STD uint64_t MCF_GetUtcTime() MCF_NOEXCEPT;
extern MCF_STD uint64_t MCF_GetLocalTime() MCF_NOEXCEPT;

extern MCF_STD uint64_t MCF_GetFastMonoClock() MCF_NOEXCEPT;
extern double MCF_GetHiResMonoClock() MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
