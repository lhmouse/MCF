// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_THREAD_H__
#define __MCF_CRT_THREAD_H__

#include "_crtdef.h"

__MCF_CRT_EXTERN void __MCF_CRTTlsEnvInitialize();
__MCF_CRT_EXTERN void __MCF_CRTTlsEnvUninitialize();

__MCF_CRT_EXTERN void __MCF_CRTThreadInitialize();
__MCF_CRT_EXTERN void __MCF_CRTThreadUninitialize();

__MCF_CRT_EXTERN void *__MCF_CreateCRTThread(
	unsigned int (*pfnProc)(__MCF_STD intptr_t),
	__MCF_STD intptr_t nParam,
	unsigned long ulFlags,
	unsigned long *pulThreadID
);

__MCF_CRT_EXTERN int __MCF_AtCRTThreadExit(
	void (*pfnProc)(__MCF_STD intptr_t),
	__MCF_STD intptr_t nContext
);

__MCF_CRT_EXTERN void *__MCF_CRTRetrieveTls(
	__MCF_STD intptr_t nKey,
	__MCF_STD size_t uSizeToAlloc,
	void (*pfnConstructor)(void *, __MCF_STD intptr_t),
	__MCF_STD intptr_t nParam,
	void (*pfnDestructor)(void *)
);
__MCF_CRT_EXTERN void __MCF_CRTDeleteTls(
	__MCF_STD intptr_t nKey
);

#endif
