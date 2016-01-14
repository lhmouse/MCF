// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_RANDOM_H_
#define __MCFCRT_EXT_RANDOM_H_

#include "../env/_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

// [0, UINT32_MAX]
extern MCFCRT_STD uint32_t MCFCRT_GetRandomUint32(void) MCFCRT_NOEXCEPT;
// [0, UINT64_MAX]
extern MCFCRT_STD uint64_t MCFCRT_GetRandomUint64(void) MCFCRT_NOEXCEPT;
// [0, 1.0)
extern double MCFCRT_GetRandomDouble(void) MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
