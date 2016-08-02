// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CRT_MODULE_H_
#define __MCFCRT_ENV_CRT_MODULE_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

typedef void (*_MCFCRT_AtCrtModuleExitCallback)(_MCFCRT_STD intptr_t __nContext);

extern bool _MCFCRT_AtCrtModuleExit(_MCFCRT_AtCrtModuleExitCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_AtCrtModuleQuickExit(_MCFCRT_AtCrtModuleExitCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

extern void __MCFCRT_DiscardCrtModuleQuickExitCallbacks(void) _MCFCRT_NOEXCEPT;

typedef enum __MCFCRT_tagExitType {
	_MCFCRT_kExitTypeNormal    = 0, // exit
	_MCFCRT_kExitTypeImmediate = 1, // _Exit
	_MCFCRT_kExitTypeQuick     = 2, // quick_exit
} _MCFCRT_ExitType;

__attribute__((__noreturn__))
extern void _MCFCRT_ExitProcess(unsigned __uExitCode, _MCFCRT_ExitType __eExitType) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
