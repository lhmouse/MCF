// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_DLL_DECL_H_
#define __MCFCRT_DLL_DECL_H_

#include "../env/_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

__attribute__((__weak__))
extern bool MCFDll_OnProcessAttach(void *__hDll, bool __bDynamic) MCF_NOEXCEPT;

__attribute__((__weak__))
extern void MCFDll_OnProcessDetach(void *__hDll, bool __bDynamic) MCF_NOEXCEPT;

__attribute__((__weak__))
extern void MCFDll_OnThreadAttach(void *__hDll) MCF_NOEXCEPT;

__attribute__((__weak__))
extern void MCFDll_OnThreadDetach(void *__hDll) MCF_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
