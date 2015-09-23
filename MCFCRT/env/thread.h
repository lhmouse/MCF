// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ENV_THREAD_H_
#define __MCF_CRT_ENV_THREAD_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern bool __MCF_CRT_ThreadEnvInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_ThreadEnvUninit(void) MCF_NOEXCEPT;

extern void __MCF_CRT_TlsThreadCleanup() MCF_NOEXCEPT;

// 失败返回 nullptr。
extern void *MCF_CRT_TlsAllocKey(void (*__pfnCallback)(MCF_STD intptr_t)) MCF_NOEXCEPT;
extern bool MCF_CRT_TlsFreeKey(void *__pTlsKey) MCF_NOEXCEPT;

extern void (*MCF_CRT_TlsGetCallback(void *__pTlsKey))(MCF_STD intptr_t) MCF_NOEXCEPT;
extern bool MCF_CRT_TlsGet(void *__pTlsKey, bool *restrict __pbHasValue, MCF_STD intptr_t *restrict __pnValue) MCF_NOEXCEPT;
// 触发回调。
extern bool MCF_CRT_TlsReset(void *__pTlsKey, MCF_STD intptr_t __nNewValue) MCF_NOEXCEPT;
// 不触发回调，__pnOldValue 不得为空。
extern bool MCF_CRT_TlsExchange(void *__pTlsKey, bool *restrict __pbHasOldValue, MCF_STD intptr_t *restrict __pnOldValue, MCF_STD intptr_t __nNewValue) MCF_NOEXCEPT;

extern int MCF_CRT_AtEndThread(void (*__pfnProc)(MCF_STD intptr_t), MCF_STD intptr_t __nContext);

extern void *MCF_CRT_CreateThread(unsigned (*__pfnThreadProc)(MCF_STD intptr_t), MCF_STD intptr_t __nParam, bool __bSuspended, unsigned long *restrict __pulThreadId) MCF_NOEXCEPT;
extern void MCF_CRT_CloseThread(void *__hThread) MCF_NOEXCEPT;

extern unsigned long MCF_CRT_GetCurrentThreadId(void) MCF_NOEXCEPT;

// 被 APC 打断返回 true，超时返回 false。
extern bool MCF_CRT_Sleep(MCF_STD uint64_t __u64MilliSeconds, bool __bAlertable) MCF_NOEXCEPT;
extern void MCF_CRT_SleepInfinitely(bool __bAlertable) MCF_NOEXCEPT;

extern long MCF_CRT_SuspendThread(void *__hThread) MCF_NOEXCEPT;
extern long MCF_CRT_ResumeThread(void *__hThread) MCF_NOEXCEPT;

// 线程结束返回 true，超时返回 false。
extern bool MCF_CRT_WaitForThread(void *__hThread, MCF_STD uint64_t __u64MilliSeconds) MCF_NOEXCEPT;
extern void MCF_CRT_WaitForThreadInfinitely(void *__hThread) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
