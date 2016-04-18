// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_DLL_DECL_H_
#define __MCFCRT_DLL_DECL_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

__attribute__((__weak__))
extern bool _MCFCRT_OnDllProcessAttach(void *__hDll, bool __bDynamic) _MCFCRT_NOEXCEPT;
__attribute__((__weak__))
extern void _MCFCRT_OnDllThreadAttach(void *__hDll) _MCFCRT_NOEXCEPT;
__attribute__((__weak__))
extern void _MCFCRT_OnDllThreadDetach(void *__hDll) _MCFCRT_NOEXCEPT;
__attribute__((__weak__))
extern void _MCFCRT_OnDllProcessDetach(void *__hDll, bool __bDynamic) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
