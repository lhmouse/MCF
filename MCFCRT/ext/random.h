// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_RANDOM_H_
#define __MCFCRT_EXT_RANDOM_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

// [0, UINT32_MAX]
extern _MCFCRT_STD uint32_t _MCFCRT_GetRandomUint32(void) _MCFCRT_NOEXCEPT;
// [0, UINT64_MAX]
extern _MCFCRT_STD uint64_t _MCFCRT_GetRandomUint64(void) _MCFCRT_NOEXCEPT;
// [0, 1.0)
extern double _MCFCRT_GetRandomDouble(void) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
