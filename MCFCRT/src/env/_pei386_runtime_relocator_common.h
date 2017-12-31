// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_PEI386_RELOCATOR_COMMON_H_
#define __MCFCRT_ENV_PEI386_RELOCATOR_COMMON_H_

#include "_crtdef.h"
#include "mcfwin.h"

_MCFCRT_EXTERN_C_BEGIN

extern void __MCFCRT_pei386_relocator_common(HMODULE __hModule, const DWORD *__pdwTableBegin, const DWORD *__pdwTableEnd) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
