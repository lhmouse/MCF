// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ENV_THUNK_H_
#define __MCF_CRT_ENV_THUNK_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern void *MCF_CRT_AllocateThunk(const void *__pInit, MCF_STD size_t __uSize) MCF_NOEXCEPT;
extern void MCF_CRT_DeallocateThunk(void *__pThunk, bool __bToPoisvn) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
