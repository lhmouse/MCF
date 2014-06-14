// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_MODULE_H_
#define MCF_CRT_MODULE_H_

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern bool __MCF_CRT_BeginModule(void);
extern void __MCF_CRT_EndModule(void);

extern int MCF_CRT_AtEndModule(void (*pfnProc)(MCF_STD intptr_t), MCF_STD intptr_t nContext);

extern void *MCF_CRT_GetModuleBase(void);

__MCF_EXTERN_C_END

#endif
