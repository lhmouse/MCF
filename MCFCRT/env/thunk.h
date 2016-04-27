// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_THUNK_H_
#define __MCFCRT_ENV_THUNK_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern const void *_MCFCRT_AllocateThunk(const void *__pInit, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_DeallocateThunk(const void *__pThunk, bool __bToPoisvn) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
