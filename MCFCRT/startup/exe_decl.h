// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STARTUP_EXE_DECL_H_
#define __MCFCRT_STARTUP_EXE_DECL_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern unsigned _MCFCRT_Main(void) _MCFCRT_NOEXCEPT;

__attribute__((__weak__))
extern void _MCFCRT_OnCtrlEvent(bool __bIsSigInt) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
