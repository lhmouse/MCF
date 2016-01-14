// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_MODULE_H_
#define __MCFCRT_ENV_MODULE_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

// __MCFCRT_BeginModule() 在所有 CRT 功能都初始化成功后返回 true。
extern bool __MCFCRT_BeginModule(void);
extern void __MCFCRT_EndModule(void);

extern bool MCFCRT_AtEndModule(void (*__pfnProc)(MCFCRT_STD intptr_t), MCFCRT_STD intptr_t __nContext) MCFCRT_NOEXCEPT;

extern void *MCFCRT_GetModuleBase(void) MCFCRT_NOEXCEPT;
// 如果回调函数返回 true 则循环继续，直到遍历完毕，此时返回 true；否则返回 false，GetLastError() 返回 ERROR_SUCCESS。失败返回 false。
extern bool MCFCRT_TraverseModuleSections(
	bool (*__pfnCallback)(MCFCRT_STD intptr_t /* context */, const char /* name */ [8], void * /* base */, MCFCRT_STD size_t /* size */),
	MCFCRT_STD intptr_t __nContext) MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
