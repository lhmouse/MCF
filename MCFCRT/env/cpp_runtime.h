// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CPP_RUNTIME_H_
#define __MCFCRT_ENV_CPP_RUNTIME_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_CppRuntimeInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_CppRuntimeUninit(void) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
