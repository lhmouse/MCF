// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_THREAD_H_
#define __MCFCRT_ENV_THREAD_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_ThreadEnvInit(void) MCFCRT_NOEXCEPT;
extern void __MCFCRT_ThreadEnvUninit(void) MCFCRT_NOEXCEPT;

extern void __MCFCRT_TlsThreadCleanup() MCFCRT_NOEXCEPT;

// 失败返回 nullptr。
extern void *MCFCRT_TlsAllocKey(void (*__pfnCallback)(MCFCRT_STD intptr_t)) MCFCRT_NOEXCEPT;
extern bool MCFCRT_TlsFreeKey(void *__pTlsKey) MCFCRT_NOEXCEPT;

extern void (*MCFCRT_TlsGetCallback(void *__pTlsKey))(MCFCRT_STD intptr_t) MCFCRT_NOEXCEPT;
extern bool MCFCRT_TlsGet(void *__pTlsKey, bool *restrict __pbHasValue, MCFCRT_STD intptr_t *restrict __pnValue) MCFCRT_NOEXCEPT;
// 触发回调。
extern bool MCFCRT_TlsReset(void *__pTlsKey, MCFCRT_STD intptr_t __nNewValue) MCFCRT_NOEXCEPT;
// 不触发回调，__pnOldValue 不得为空。
extern bool MCFCRT_TlsExchange(void *__pTlsKey, bool *restrict __pbHasOldValue, MCFCRT_STD intptr_t *restrict __pnOldValue, MCFCRT_STD intptr_t __nNewValue) MCFCRT_NOEXCEPT;

extern int MCFCRT_AtEndThread(void (*__pfnProc)(MCFCRT_STD intptr_t), MCFCRT_STD intptr_t __nContext);

extern void *MCFCRT_CreateNativeThread(unsigned long (*__attribute__((__stdcall__)) __pfnThreadProc)(void *), void *__pParam, bool __bSuspended, MCFCRT_STD uintptr_t *restrict __puThreadId) MCFCRT_NOEXCEPT;
extern void *MCFCRT_CreateThread(unsigned (*__pfnThreadProc)(MCFCRT_STD intptr_t), MCFCRT_STD intptr_t __nParam, bool __bSuspended, MCFCRT_STD uintptr_t *restrict __puThreadId) MCFCRT_NOEXCEPT;
extern void MCFCRT_CloseThread(void *__hThread) MCFCRT_NOEXCEPT;

extern MCFCRT_STD uintptr_t MCFCRT_GetCurrentThreadId(void) MCFCRT_NOEXCEPT;

extern void MCFCRT_Sleep(MCFCRT_STD uint64_t __u64UntilFastMonoClock) MCFCRT_NOEXCEPT;
// 被 APC 打断返回 true，超时返回 false。
extern bool MCFCRT_AlertableSleep(MCFCRT_STD uint64_t __u64UntilFastMonoClock) MCFCRT_NOEXCEPT;
extern void MCFCRT_AlertableSleepInfinitely(void) MCFCRT_NOEXCEPT;
extern void MCFCRT_YieldThread(void) MCFCRT_NOEXCEPT;

extern long MCFCRT_SuspendThread(void *__hThread) MCFCRT_NOEXCEPT;
extern long MCFCRT_ResumeThread(void *__hThread) MCFCRT_NOEXCEPT;

// 线程结束返回 true，超时返回 false。
extern bool MCFCRT_WaitForThread(void *__hThread, MCFCRT_STD uint64_t __u64UntilFastMonoClock) MCFCRT_NOEXCEPT;
extern void MCFCRT_WaitForThreadInfinitely(void *__hThread) MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
