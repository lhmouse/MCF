// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CRT_MODULE_H_
#define __MCFCRT_ENV_CRT_MODULE_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

typedef void (*_MCFCRT_AtCrtModuleExitCallback)(_MCFCRT_STD intptr_t __nContext);

extern bool _MCFCRT_AtCrtModuleQuickExit(_MCFCRT_AtCrtModuleExitCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

extern void __MCFCRT_DiscardCrtModuleQuickExitCallbacks(void) _MCFCRT_NOEXCEPT;

__attribute__((__noreturn__))
extern void _MCFCRT_ImmediateExit(unsigned __uExitCode) _MCFCRT_NOEXCEPT;
__attribute__((__noreturn__))
extern void _MCFCRT_QuickExit(unsigned __uExitCode) _MCFCRT_NOEXCEPT;
__attribute__((__noreturn__))
extern void _MCFCRT_Exit(unsigned __uExitCode) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
