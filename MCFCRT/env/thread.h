// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_THREAD_H_
#define __MCF_CRT_THREAD_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern bool __MCF_CRT_TlsEnvInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_TlsEnvUninit(void) MCF_NOEXCEPT;

extern void __stdcall __MCF_CRT_TlsCallback(void *hModule, unsigned long ulReason, void *pReserved) MCF_NOEXCEPT;

// 失败返回 nullptr。
extern void *MCF_CRT_TlsAllocKey(void (__cdecl *pfnCallback)(MCF_STD intptr_t)) MCF_NOEXCEPT;
extern bool MCF_CRT_TlsFreeKey(void *pTlsKey) MCF_NOEXCEPT;

extern void (__cdecl *MCF_CRT_TlsGetCallback(void *pTlsKey))(MCF_STD intptr_t) MCF_NOEXCEPT;
extern bool MCF_CRT_TlsGet(void *pTlsKey, bool *pbHasValue, MCF_STD intptr_t *pnValue) MCF_NOEXCEPT;
// 触发回调。
extern bool MCF_CRT_TlsReset(void *pTlsKey, MCF_STD intptr_t nNewValue) MCF_NOEXCEPT;
// 不触发回调，pnOldValue 不得为空。
extern bool MCF_CRT_TlsExchange(void *pTlsKey, bool *pbHasOldValue, MCF_STD intptr_t *pnOldValue, MCF_STD intptr_t nNewValue) MCF_NOEXCEPT;
// 删除所有 Tls。
extern void MCF_CRT_TlsClearAll() MCF_NOEXCEPT;

extern int MCF_CRT_AtEndThread(void (__cdecl *pfnProc)(MCF_STD intptr_t), MCF_STD intptr_t nContext);

// 返回的是 HANDLE。
extern void *MCF_CRT_CreateThread(unsigned (*pfnThreadProc)(MCF_STD intptr_t), MCF_STD intptr_t nParam, bool bSuspended, unsigned long *pulThreadId) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
