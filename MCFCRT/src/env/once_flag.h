// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_ONCE_FLAG_H_
#define __MCFCRT_ENV_ONCE_FLAG_H_

#include "_crtdef.h"

#ifndef __MCFCRT_ONCE_FLAG_INLINE_OR_EXTERN
#	define __MCFCRT_ONCE_FLAG_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

// 初始化为 { 0 } 即可。
typedef struct __MCFCRT_tagOnceFlag {
	_MCFCRT_STD uintptr_t __u;
} volatile _MCFCRT_OnceFlag;

typedef enum __MCFCRT_tagOnceResult {
	_MCFCRT_kOnceResultTimedOut = 1,
	_MCFCRT_kOnceResultInitial  = 2,
	_MCFCRT_kOnceResultFinished = 3,
} _MCFCRT_OnceResult;

__MCFCRT_ONCE_FLAG_INLINE_OR_EXTERN void _MCFCRT_InitializeOnceFlag(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT {
	__atomic_store_n(&(__pOnceFlag->__u), 0, __ATOMIC_RELEASE);
}

extern _MCFCRT_OnceResult __MCFCRT_ReallyWaitForOnceFlag(_MCFCRT_OnceFlag *__pOnceFlag, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern _MCFCRT_OnceResult __MCFCRT_ReallyWaitForOnceFlagForever(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_ReallySignalOnceFlagAsFinished(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_ReallySignalOnceFlagAsAborted(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;

// See Itanium ABI: <https://itanium-cxx-abi.github.io/cxx-abi/abi.html#guards>
// Bytes other than the first byte are used as the counter of trapped threads, in native byte order.
__MCFCRT_ONCE_FLAG_INLINE_OR_EXTERN _MCFCRT_OnceResult _MCFCRT_WaitForOnceFlag(_MCFCRT_OnceFlag *__pOnceFlag, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT {
	volatile unsigned char *const __pbyGuard = (unsigned char *)(void *)&(__pOnceFlag->__u);
	if(__builtin_expect(__pbyGuard[0] != 0, true)){
		return _MCFCRT_kOnceResultFinished;
	}
	return __MCFCRT_ReallyWaitForOnceFlag(__pOnceFlag, __u64UntilFastMonoClock);
}
__MCFCRT_ONCE_FLAG_INLINE_OR_EXTERN _MCFCRT_OnceResult _MCFCRT_WaitForOnceFlagForever(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT {
	volatile unsigned char *const __pbyGuard = (unsigned char *)(void *)&(__pOnceFlag->__u);
	if(__builtin_expect(__pbyGuard[0] != 0, true)){
		return _MCFCRT_kOnceResultFinished;
	}
	return __MCFCRT_ReallyWaitForOnceFlagForever(__pOnceFlag);
}
__MCFCRT_ONCE_FLAG_INLINE_OR_EXTERN void _MCFCRT_SignalOnceFlagAsFinished(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT {
	__MCFCRT_ReallySignalOnceFlagAsFinished(__pOnceFlag);
}
__MCFCRT_ONCE_FLAG_INLINE_OR_EXTERN void _MCFCRT_SignalOnceFlagAsAborted(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT {
	__MCFCRT_ReallySignalOnceFlagAsAborted(__pOnceFlag);
}

_MCFCRT_EXTERN_C_END

#endif
