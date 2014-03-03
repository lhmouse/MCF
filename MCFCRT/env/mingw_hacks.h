// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

// 多线程同时写，单线程读，可以不加锁。

#ifndef __MCF_CRT_MINGW_HACKS_H__
#define __MCF_CRT_MINGW_HACKS_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern void __MCF_CRT_RunEmutlsThreadDtors(void) __MCF_NOEXCEPT;
extern unsigned long __MCF_CRT_EmutlsCleanup(void) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
