// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_MUTEX_H_
#define __MCFCRT_ENV_MUTEX_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

typedef volatile _MCFCRT_STD uintptr_t _MCFCRT_Mutex;

static_assert(sizeof(_MCFCRT_Mutex) >= 4,  "Oops.");
static_assert(alignof(_MCFCRT_Mutex) >= 4, "Oops.");

extern bool _MCFCRT_TryMutex(_MCFCRT_Mutex *__pMutex, _MCFCRT_STD size_t __uMaxSpinCount, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_LockMutex(_MCFCRT_Mutex *__pMutex, _MCFCRT_STD size_t __uMaxSpinCount) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_UnlockMutex(_MCFCRT_Mutex *__pMutex) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
