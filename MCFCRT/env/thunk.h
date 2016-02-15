// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_THUNK_H_
#define __MCFCRT_ENV_THUNK_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

extern const void *MCFCRT_AllocateThunk(const void *__pInit, MCFCRT_STD size_t __uSize) MCFCRT_NOEXCEPT;
extern void MCFCRT_DeallocateThunk(const void *__pThunk, bool __bToPoisvn) MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
