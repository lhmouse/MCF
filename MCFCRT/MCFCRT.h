// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_MCFCRT_H__
#define __MCF_CRT_MCFCRT_H__

#include "c/cinclude.h"

#ifdef __cplusplus
#	include "cpp/cppinclude.hpp"
#endif

#include "env/envinclude.h"

__MCF_EXTERN_C_BEGIN

extern unsigned long __MCF_CRT_Begin(void);
extern void __MCF_CRT_End(void);

extern void *__MCF_GetModuleBase();
extern int __MCF_AtCRTEnd(void (*pfnProc)(__MCF_STD intptr_t), __MCF_STD intptr_t nContext);

__MCF_EXTERN_C_END

#endif
