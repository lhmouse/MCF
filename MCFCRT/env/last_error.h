// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_LAST_ERROR_H__
#define __MCF_CRT_LAST_ERROR_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern unsigned long __MCF_CRT_GetWin32LastError(void) __MCF_NOEXCEPT;
extern void __MCF_CRT_SetWin32LastError(unsigned long ulErrorCode) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#ifndef ERROR_SUCCESS
#	define ERROR_SUCCESS	0
#endif

#endif
