// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CRT_MODULE_H_
#define __MCFCRT_ENV_CRT_MODULE_H_

#include "../pre/module.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool _MCFCRT_AtCrtModuleExit(_MCFCRT_AtModuleExitCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

extern bool __MCFCRT_IsQuickExitInProgress(void) _MCFCRT_NOEXCEPT;
__attribute__((__noreturn__))
extern bool __MCFCRT_ExitProcess(int __nExitCode, bool __bIsQuick) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
