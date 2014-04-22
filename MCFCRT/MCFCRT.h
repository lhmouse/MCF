// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_MCFCRT_H_
#define MCF_CRT_MCFCRT_H_

#include "c/c_include.h"
#include "env/env_include.h"

#ifdef __cplusplus
#	include "cpp/cpp_include.hpp"
#endif

#if !defined(NDEBUG) && defined(__cplusplus)
#	define _GLIBCXX_DEBUG	1
#endif

__MCF_EXTERN_C_BEGIN

extern unsigned long __MCF_CRT_Begin(void);
extern void __MCF_CRT_End(void);

extern void *MCF_GetModuleBase(void);

extern int MCF_AtCRTEnd(void (*pfnProc)(__MCF_STD intptr_t), __MCF_STD intptr_t nContext);

__MCF_EXTERN_C_END

#endif
