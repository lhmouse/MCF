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

extern bool _MCFCRT_AtEndModule(void (*__pfnProc)(_MCFCRT_STD intptr_t), _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

extern void *_MCFCRT_GetModuleBase(void) _MCFCRT_NOEXCEPT;
// 如果回调函数返回 true 则循环继续，直到遍历完毕，此时返回 true；否则返回 false，GetLastError() 返回 ERROR_SUCCESS。失败返回 false。
extern bool _MCFCRT_TraverseModuleSections(
	bool (*__pfnCallback)(_MCFCRT_STD intptr_t /* context */, const char /* name */ [8], void * /* base */, _MCFCRT_STD size_t /* size */),
	_MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
