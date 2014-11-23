// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_THUNK_H_
#define MCF_CRT_THUNK_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern void *MCF_CRT_AllocateThunk(const void *pInit, MCF_STD size_t uSize) MCF_NOEXCEPT;
extern void MCF_CRT_DeallocateThunk(void *pThunk, bool bToPoison) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
