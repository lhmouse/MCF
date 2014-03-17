// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_THREAD_H__
#define __MCF_CRT_THREAD_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern unsigned long __MCF_CRT_TlsEnvInitialize(void) __MCF_NOEXCEPT;
extern void __MCF_CRT_TlsEnvUninitialize(void) __MCF_NOEXCEPT;

extern unsigned long __MCF_CRT_ThreadInitialize(void) __MCF_NOEXCEPT;
extern void __MCF_CRT_ThreadUninitialize(void) __MCF_NOEXCEPT;

extern void *__MCF_CRT_CreateThread(
	unsigned int (*pfnProc)(__MCF_STD intptr_t),
	__MCF_STD intptr_t nParam,
	unsigned long ulFlags,
	unsigned long *pulThreadId
) __MCF_NOEXCEPT;

extern int __MCF_CRT_AtThreadExit(
	void (*pfnProc)(__MCF_STD intptr_t),
	__MCF_STD intptr_t nContext
) __MCF_NOEXCEPT;

extern void *__MCF_CRT_RetrieveTls(
	__MCF_STD intptr_t nKey,
	__MCF_STD size_t uSizeToAlloc,
	void (*pfnConstructor)(void *, __MCF_STD intptr_t),
	__MCF_STD intptr_t nParam,
	void (*pfnDestructor)(void *)
) __MCF_NOEXCEPT;

extern void __MCF_CRT_DeleteTls(
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
