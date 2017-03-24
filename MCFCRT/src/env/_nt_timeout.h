// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_NT_TIMEOUT_H_
#define __MCFCRT_ENV_NT_TIMEOUT_H_

#include "_crtdef.h"
#include "mcfwin.h"
#include "clocks.h"

#ifndef __MCFCRT_NT_TIMEOUT_INLINE_OR_EXTERN
#	define __MCFCRT_NT_TIMEOUT_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_NT_TIMEOUT_INLINE_OR_EXTERN void __MCFCRT_InitializeNtTimeout(LARGE_INTEGER *__pliTimeout, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uint64_t __u64Now = _MCFCRT_GetFastMonoClock();
	if(__u64UntilFastMonoClock < __u64Now){
		// We should time out immediately.
		__pliTimeout->QuadPart = 0;
		return;
	}
	const _MCFCRT_STD uint64_t __u64Infinite = (1ull << 63) - 1u;
	const _MCFCRT_STD uint64_t __u64DeltaMs = __u64UntilFastMonoClock - __u64Now;
	if(__u64DeltaMs > __u64Infinite / 10000u - 1u){
		// We should never time out.
		__pliTimeout->QuadPart = (_MCFCRT_STD int64_t)__u64Infinite;
		return;
	}
	// If this value is negative, the duration is measured by the absolute value of it, in 100 nanoseconds.
	// An increment of 9999u makes sure we never time out before the time point.
	__pliTimeout->QuadPart = -(_MCFCRT_STD int64_t)(__u64DeltaMs * 10000u + 9999u);
}

_MCFCRT_EXTERN_C_END

#endif
