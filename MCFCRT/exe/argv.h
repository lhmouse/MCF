// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ARGV_H__
#define __MCF_CRT_ARGV_H__

#include "../env/_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern __MCF_STD size_t __MCF_GetArgV(const wchar_t *const **pppwszArgV) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
