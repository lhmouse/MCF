// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_EXT_RANDOM_H_
#define __MCF_CRT_EXT_RANDOM_H_

#include "../env/_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

// [0, UINT32_MAX]
extern MCF_STD uint32_t MCF_GetRandomUint32(void) MCF_NOEXCEPT;
// [0, UINT64_MAX]
extern MCF_STD uint64_t MCF_GetRandomUint64(void) MCF_NOEXCEPT;
// [0, 1.0)
extern double MCF_GetRandomDouble(void) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
