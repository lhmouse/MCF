// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_ONCE_FLAG_H_
#define __MCFCRT_ENV_ONCE_FLAG_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

// 初始化为 { 0 } 即可。
typedef struct __MCFCRT_tagOnceFlag {
	_MCFCRT_STD uintptr_t __u;
} volatile _MCFCRT_OnceFlag;

typedef enum __MCFCRT_tagOnceResult {
	_MCFCRT_kOnceResultTimedOut = 1,
	_MCFCRT_kOnceResultInitial  = 2,
	_MCFCRT_kOnceResultFinished = 3,
} _MCFCRT_OnceResult;

static inline void _MCFCRT_InitializeOnceFlag(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT {
	__atomic_store_n(&(__pOnceFlag->__u), 0, __ATOMIC_RELEASE);
}

extern _MCFCRT_OnceResult _MCFCRT_WaitForOnceFlag(_MCFCRT_OnceFlag *__pOnceFlag, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern _MCFCRT_OnceResult _MCFCRT_WaitForOnceFlagForever(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SignalOnceFlagAsFinished(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SignalOnceFlagAsAborted(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
