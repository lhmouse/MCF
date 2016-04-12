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
} _MCFCRT_OnceFlag;

#ifdef __cplusplus
#	define _MCFCRT_ONCE_FLAG_INITIALIZER    (        __MCFCRT_tagOnceFlag { 0 })
#else
#	define _MCFCRT_ONCE_FLAG_INITIALIZER    ((struct __MCFCRT_tagOnceFlag){ 0 })
#endif

typedef enum __MCFCRT_tagOnceResult {
	_MCFCRT_kOnceResultTimedOut = 1,
	_MCFCRT_kOnceResultInitial  = 2,
	_MCFCRT_kOnceResultFinished = 3,
} _MCFCRT_OnceResult;

extern _MCFCRT_OnceResult _MCFCRT_WaitForOnceFlag(_MCFCRT_OnceFlag *__pOnceFlag, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern _MCFCRT_OnceResult _MCFCRT_WaitForOnceFlagForever(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SignalOnceFlagAsFinished(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SignalOnceFlagAsAborted(_MCFCRT_OnceFlag *__pOnceFlag) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
