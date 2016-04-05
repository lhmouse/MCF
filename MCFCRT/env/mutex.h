// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_MUTEX_H_
#define __MCFCRT_ENV_MUTEX_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

// 初始化为 { 0 } 即可。
typedef struct _MCFCRT_tagMutex {
	_MCFCRT_STD uintptr_t __u;
} volatile _MCFCRT_Mutex;

#ifdef __cplusplus
#	define _MCFCRT_MUTEX_INITIALIZER    (        _MCFCRT_tagMutex { 0 })
#else
#	define _MCFCRT_MUTEX_INITIALIZER    ((struct _MCFCRT_tagMutex){ 0 })
#endif

extern bool _MCFCRT_WaitForMutex(_MCFCRT_Mutex *__pMutex, _MCFCRT_STD size_t __uMaxSpinCount, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_WaitForMutexForever(_MCFCRT_Mutex *__pMutex, _MCFCRT_STD size_t __uMaxSpinCount) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SignalMutex(_MCFCRT_Mutex *__pMutex) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
