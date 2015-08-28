// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_DLL_DECL_H_
#define __MCF_CRT_DLL_DECL_H_

#include "../env/_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

__attribute__((__weak__))
extern bool MCFDll_OnProcessAttach(void *hDll, bool bDynamic) MCF_NOEXCEPT;

__attribute__((__weak__))
extern void MCFDll_OnProcessDetach(void *hDll, bool bDynamic) MCF_NOEXCEPT;

__attribute__((__weak__))
extern void MCFDll_OnThreadAttach(void *hDll) MCF_NOEXCEPT;

__attribute__((__weak__))
extern void MCFDll_OnThreadDetach(void *hDll) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
