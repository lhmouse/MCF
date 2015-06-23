// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_MINGW_HACKS_H_
#define __MCF_CRT_MINGW_HACKS_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern bool __MCF_CRT_MinGWHacksInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_MinGWHacksUninit(void) MCF_NOEXCEPT;

extern void __MCF_CRT_RunEmutlsDtors(void) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
