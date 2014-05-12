// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_THREAD_H_
#define MCF_CRT_THREAD_H_

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern unsigned long __MCF_CRT_TlsEnvInitialize(void) MCF_NOEXCEPT;
extern void __MCF_CRT_TlsEnvUninitialize(void) MCF_NOEXCEPT;

extern unsigned long __MCF_CRT_ThreadInitialize(void) MCF_NOEXCEPT;
extern void __MCF_CRT_ThreadUninitialize(void) MCF_NOEXCEPT;

extern void *MCF_CRT_CreateThread(
	unsigned int (*pfnProc)(MCF_STD intptr_t),
	MCF_STD intptr_t nParam,
	unsigned long ulFlags,
	unsigned long *pulThreadId
) MCF_NOEXCEPT;

extern int MCF_CRT_AtThreadExit(
	void (*pfnProc)(MCF_STD intptr_t),
	MCF_STD intptr_t nContext
) MCF_NOEXCEPT;

extern void *MCF_CRT_RetrieveTls(
	MCF_STD intptr_t nKey,
	MCF_STD size_t uSizeToAlloc,
	int (*pfnConstructor)(void *, MCF_STD intptr_t),
	MCF_STD intptr_t nParam,
	void (*pfnDestructor)(void *)
) MCF_NOEXCEPT;

extern void MCF_CRT_DeleteTls(
	MCF_STD intptr_t nKey
) MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
