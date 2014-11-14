// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_THREAD_H_
#define MCF_CRT_THREAD_H_

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern bool __MCF_CRT_TlsEnvInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_TlsEnvUninit(void) MCF_NOEXCEPT;

extern void __stdcall __MCF_CRT_TlsCallback(void *hModule, unsigned long ulReason, void *pReserved) MCF_NOEXCEPT;

// 失败返回 NULL。
extern void *MCF_CRT_AtThreadExit(void (__cdecl *pfnProc)(MCF_STD intptr_t), MCF_STD intptr_t nContext) MCF_NOEXCEPT;
extern bool MCF_CRT_RemoveAtThreadExit(void *pTlsKey) MCF_NOEXCEPT;

// 失败返回 NULL。
extern void *MCF_CRT_TlsAllocKey(void (__cdecl *pfnCallback)(MCF_STD intptr_t)) MCF_NOEXCEPT;
extern bool MCF_CRT_TlsFreeKey(void *pTlsKey) MCF_NOEXCEPT;

extern bool MCF_CRT_TlsGet(void *pTlsKey, MCF_STD intptr_t *pnValue) MCF_NOEXCEPT;
// 触发回调。
extern bool MCF_CRT_TlsReset(void *pTlsKey, MCF_STD intptr_t nNewValue) MCF_NOEXCEPT;

typedef enum tagMCFTlsExchangeResult {
	MCF_TLSXCH_FAILED,				// 失败（例如 key 无效）。
	MCF_TLSXCH_OLD_VAL_RETURNED,	// *pnOldValue 返回旧值。
	MCF_TLSXCH_NEW_VAL_SET,			// 旧值未设定。
} MCF_TlsExchangeResult;

// 不触发回调。pnOldValue 不得为空。
extern MCF_TlsExchangeResult MCF_CRT_TlsExchange(void *pTlsKey,
	MCF_STD intptr_t *pnOldValue, MCF_STD intptr_t nNewValue) MCF_NOEXCEPT;

// 删除所有 Tls。
extern void MCF_CRT_TlsClearAll() MCF_NOEXCEPT;

// 返回的是 HANDLE。
extern void *MCF_CRT_CreateThread(unsigned (*pfnThreadProc)(MCF_STD intptr_t), MCF_STD intptr_t nParam,
	bool bSuspended, unsigned long *pulThreadId) MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
