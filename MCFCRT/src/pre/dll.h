// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_PRE_DLL_H_
#define __MCFCRT_PRE_DLL_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool _MCFCRT_OnDllProcessAttach(void *__pInstance, bool __bDynamic) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_OnDllThreadAttach(void *__pInstance) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_OnDllThreadDetach(void *__pInstance) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_OnDllProcessDetach(void *__pInstance, bool __bDynamic) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
