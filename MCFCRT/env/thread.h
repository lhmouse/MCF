// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_THREAD_H_
#define __MCFCRT_ENV_THREAD_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_ThreadEnvInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_ThreadEnvUninit(void) _MCFCRT_NOEXCEPT;

extern void __MCFCRT_TlsThreadCleanup() _MCFCRT_NOEXCEPT;

// 失败返回 nullptr。
extern void *_MCFCRT_TlsAllocKey(void (*__pfnCallback)(_MCFCRT_STD intptr_t)) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_TlsFreeKey(void *__pTlsKey) _MCFCRT_NOEXCEPT;

extern void (*_MCFCRT_TlsGetCallback(void *__pTlsKey))(_MCFCRT_STD intptr_t) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_TlsGet(void *__pTlsKey, bool *restrict __pbHasValue, _MCFCRT_STD intptr_t *restrict __pnValue) _MCFCRT_NOEXCEPT;
// 触发回调。
extern bool _MCFCRT_TlsReset(void *__pTlsKey, _MCFCRT_STD intptr_t __nNewValue) _MCFCRT_NOEXCEPT;
// 不触发回调，__pnOldValue 不得为空。
extern bool _MCFCRT_TlsExchange(void *__pTlsKey, bool *restrict __pbHasOldValue, _MCFCRT_STD intptr_t *restrict __pnOldValue, _MCFCRT_STD intptr_t __nNewValue) _MCFCRT_NOEXCEPT;

extern int _MCFCRT_AtEndThread(void (*__pfnProc)(_MCFCRT_STD intptr_t), _MCFCRT_STD intptr_t __nContext);

extern void *_MCFCRT_CreateNativeThread(unsigned long (*__attribute__((__stdcall__)) __pfnThreadProc)(void *), void *__pParam, bool __bSuspended, _MCFCRT_STD uintptr_t *restrict __puThreadId) _MCFCRT_NOEXCEPT;
extern void *_MCFCRT_CreateThread(unsigned (*__pfnThreadProc)(_MCFCRT_STD intptr_t), _MCFCRT_STD intptr_t __nParam, bool __bSuspended, _MCFCRT_STD uintptr_t *restrict __puThreadId) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_CloseThread(void *__hThread) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD uintptr_t _MCFCRT_GetCurrentThreadId(void) _MCFCRT_NOEXCEPT;

extern void _MCFCRT_Sleep(_MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
// 被 APC 打断返回 true，超时返回 false。
extern bool _MCFCRT_AlertableSleep(_MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_AlertableSleepForever(void) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_YieldThread(void) _MCFCRT_NOEXCEPT;

extern long _MCFCRT_SuspendThread(void *__hThread) _MCFCRT_NOEXCEPT;
extern long _MCFCRT_ResumeThread(void *__hThread) _MCFCRT_NOEXCEPT;

// 线程结束返回 true，超时返回 false。
extern bool _MCFCRT_WaitForThread(void *__hThread, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_WaitForThreadForever(void *__hThread) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
