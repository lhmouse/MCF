// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_MEMCCHR_H_
#define MCF_CRT_MEMCCHR_H_

#include "../../env/_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern void *_memcchr(const void *s, int c, __MCF_STD size_t n) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
