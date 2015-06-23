// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_MODULE_H_
#define __MCF_CRT_MODULE_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

// __MCF_CRT_BeginModule() 在所有 CRT 功能都初始化成功后返回 true。
// 即使返回 false 也需要调用 __MCF_CRT_EndModule()。
extern bool __MCF_CRT_BeginModule(void);
extern void __MCF_CRT_EndModule(void);

extern int MCF_CRT_AtEndModule(void (__cdecl *pfnProc)(MCF_STD intptr_t), MCF_STD intptr_t nContext);

extern void *MCF_CRT_GetModuleBase(void);

__MCF_CRT_EXTERN_C_END

#endif
