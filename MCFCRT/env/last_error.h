// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_LAST_ERROR_H__
#define __MCF_CRT_LAST_ERROR_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern unsigned long __MCF_CRT_GetLastError(void);
extern void __MCF_CRT_SetLastError(unsigned long ulErrorCode);

__MCF_EXTERN_C_END

#ifndef ERROR_SUCCESS
#	define ERROR_SUCCESS	0
#endif

#endif
