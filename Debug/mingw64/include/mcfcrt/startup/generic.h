// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STARTUP_GENERIC_H_
#define __MCFCRT_STARTUP_GENERIC_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_TlsCallbackGeneric(void *__pInstance, unsigned __uReason, bool __bDynamic) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
