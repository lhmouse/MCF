// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ENV_MODULE_H_
#define __MCF_CRT_ENV_MODULE_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

// __MCF_CRT_BeginModule() 在所有 CRT 功能都初始化成功后返回 true。
extern bool __MCF_CRT_BeginModule(void);
extern void __MCF_CRT_EndModule(void);

extern bool MCF_CRT_AtEndModule(void (*__pfnProc)(MCF_STD intptr_t), MCF_STD intptr_t __nContext) MCF_NOEXCEPT;

extern void *MCF_CRT_GetModuleBase(void) MCF_NOEXCEPT;
// 如果回调函数返回 true 则循环继续，直到遍历完毕，此时返回 true；否则返回 false，GetLastError() 返回 ERROR_SUCCESS。失败返回 false。
extern bool MCF_CRT_TraverseModuleSections(
	bool (*__pfnCallback)(MCF_STD intptr_t /* context */, const char /* name */ [8], void * /* base */, MCF_STD size_t /* size */),
	MCF_STD intptr_t __nContext) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
