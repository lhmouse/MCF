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

// 返回的是 HANDLE。
extern void *MCF_CRT_CreateThread(unsigned (*__pfnThreadProc)(MCF_STD intptr_t), MCF_STD intptr_t __nParam, bool __bSuspended, unsigned long *restrict __pulThreadId) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
