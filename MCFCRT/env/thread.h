// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_THREAD_H_
#define MCF_CRT_THREAD_H_

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern bool __MCF_CRT_TlsEnvInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_TlsEnvUninit(void) MCF_NOEXCEPT;

extern void __MCF_CRT_TlsCleanup(void) MCF_NOEXCEPT;

// 失败返回 0。
extern MCF_STD uintptr_t MCF_CRT_AtThreadExit(void (*pfnProc)(MCF_STD intptr_t), MCF_STD intptr_t nContext) MCF_NOEXCEPT;
extern bool MCF_CRT_RemoveAtThreadExit(MCF_STD uintptr_t uKey) MCF_NOEXCEPT;

// 失败返回 0。
extern MCF_STD uintptr_t MCF_CRT_TlsAllocKey(void (*pfnCallback)(MCF_STD intptr_t)) MCF_NOEXCEPT;
extern bool MCF_CRT_TlsFreeKey(MCF_STD uintptr_t uKey) MCF_NOEXCEPT;

extern bool MCF_CRT_TlsGet(MCF_STD uintptr_t uKey, MCF_STD intptr_t *pnValue) MCF_NOEXCEPT;
// 触发回调。
extern bool MCF_CRT_TlsReset(MCF_STD uintptr_t uKey, MCF_STD intptr_t nNewValue) MCF_NOEXCEPT;
// 不触发回调。pnOldValue 不得为空。
// 返回 0 若失败，返回 1 若新值被设定但旧值未定义，返回 2 若旧值有效且已被替换。
extern int MCF_CRT_TlsExchange(MCF_STD uintptr_t uKey, MCF_STD intptr_t *pnOldValue, MCF_STD intptr_t nNewValue) MCF_NOEXCEPT;

// 返回的是 HANDLE。
extern void *MCF_CRT_CreateThread(
	unsigned int (*pfnThreadProc)(MCF_STD intptr_t),
	MCF_STD intptr_t nParam,
	bool bSuspended,
	unsigned long *pulThreadId
) MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
