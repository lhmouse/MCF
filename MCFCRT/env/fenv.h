// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_FENV_H_
#define MCF_CRT_FENV_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern bool __MCF_CRT_FEnvInit(void) MCF_NOEXCEPT;

#define __MCF_CRT_FEnvUninit()	((void)0)

__MCF_CRT_EXTERN_C_END

#endif
