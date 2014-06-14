// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_THREAD_H_
#define MCF_CRT_THREAD_H_

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern bool __MCF_CRT_TlsEnvInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_TlsEnvUninit(void) MCF_NOEXCEPT;

extern bool __MCF_CRT_ThreadInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_ThreadUninit(void) MCF_NOEXCEPT;

// 返回的是 HANDLE。
extern void *MCF_CRT_CreateThread(
	unsigned int (*pfnThreadProc)(MCF_STD intptr_t),
	MCF_STD intptr_t nParam,
	bool bSuspended,
	unsigned long *pulThreadId
) MCF_NOEXCEPT;

// 失败返回 -1。
extern MCF_STD intptr_t MCF_CRT_AtThreadExit(void (*pfnProc)(MCF_STD intptr_t), MCF_STD intptr_t nContext) MCF_NOEXCEPT;
extern bool MCF_CRT_RemoveAtThreadExit(MCF_STD intptr_t nKey) MCF_NOEXCEPT;

// 失败返回 -1。
// 这里的 TlsCallback 类似于 FlsCallback，但需注意线程必须使用 __MCF_CRT_ThreadInitialize() 初始化过。
// 在任何情况下都调用回调，即使 Tls 中保存的值是缺省值。
// 使用 MCF_CRT_CreateThread() 创建的线程不需要显式调用 __MCF_CRT_ThreadInitialize()。
extern MCF_STD intptr_t MCF_CRT_TlsAllocKey(void (*pfnTlsCallback)(MCF_STD intptr_t), MCF_STD intptr_t nInitValue) MCF_NOEXCEPT;
extern bool MCF_CRT_TlsFreeKey(MCF_STD intptr_t nKey) MCF_NOEXCEPT;

extern bool MCF_CRT_TlsGetValue(MCF_STD intptr_t nKey, MCF_STD intptr_t *pnValue) MCF_NOEXCEPT;
// 注意这里不触发回调。
extern bool MCF_CRT_TlsSetValue(MCF_STD intptr_t nKey, MCF_STD intptr_t *pnOldValue, MCF_STD intptr_t nNewValue) MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
