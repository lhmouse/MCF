// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_THREAD_H_
#define __MCFCRT_ENV_THREAD_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_ThreadEnvInit(void) MCF_NOEXCEPT;
extern void __MCFCRT_ThreadEnvUninit(void) MCF_NOEXCEPT;

extern void __MCFCRT_TlsThreadCleanup() MCF_NOEXCEPT;

// 失败返回 nullptr。
extern void *MCFCRT_TlsAllocKey(void (*__pfnCallback)(MCF_STD intptr_t)) MCF_NOEXCEPT;
extern bool MCFCRT_TlsFreeKey(void *__pTlsKey) MCF_NOEXCEPT;

extern void (*MCFCRT_TlsGetCallback(void *__pTlsKey))(MCF_STD intptr_t) MCF_NOEXCEPT;
extern bool MCFCRT_TlsGet(void *__pTlsKey, bool *restrict __pbHasValue, MCF_STD intptr_t *restrict __pnValue) MCF_NOEXCEPT;
// 触发回调。
extern bool MCFCRT_TlsReset(void *__pTlsKey, MCF_STD intptr_t __nNewValue) MCF_NOEXCEPT;
// 不触发回调，__pnOldValue 不得为空。
extern bool MCFCRT_TlsExchange(void *__pTlsKey, bool *restrict __pbHasOldValue, MCF_STD intptr_t *restrict __pnOldValue, MCF_STD intptr_t __nNewValue) MCF_NOEXCEPT;

extern int MCFCRT_AtEndThread(void (*__pfnProc)(MCF_STD intptr_t), MCF_STD intptr_t __nContext);

extern void *MCFCRT_CreateThread(unsigned (*__pfnThreadProc)(MCF_STD intptr_t), MCF_STD intptr_t __nParam, bool __bSuspended, MCF_STD uintptr_t *restrict __puThreadId) MCF_NOEXCEPT;
extern void MCFCRT_CloseThread(void *__hThread) MCF_NOEXCEPT;

extern MCF_STD uintptr_t MCFCRT_GetCurrentThreadId(void) MCF_NOEXCEPT;

extern void MCFCRT_Sleep(MCF_STD uint64_t __u64UntilFastMonoClock) MCF_NOEXCEPT;
// 被 APC 打断返回 true，超时返回 false。
extern bool MCFCRT_AlertableSleep(MCF_STD uint64_t __u64UntilFastMonoClock) MCF_NOEXCEPT;
extern void MCFCRT_AlertableSleepInfinitely(void) MCF_NOEXCEPT;
extern void MCFCRT_YieldThread(void) MCF_NOEXCEPT;

extern long MCFCRT_SuspendThread(void *__hThread) MCF_NOEXCEPT;
extern long MCFCRT_ResumeThread(void *__hThread) MCF_NOEXCEPT;

// 线程结束返回 true，超时返回 false。
extern bool MCFCRT_WaitForThread(void *__hThread, MCF_STD uint64_t __u64UntilFastMonoClock) MCF_NOEXCEPT;
extern void MCFCRT_WaitForThreadInfinitely(void *__hThread) MCF_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
