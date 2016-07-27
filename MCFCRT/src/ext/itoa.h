// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_ITOA_H_
#define __MCFCRT_EXT_ITOA_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern char *_MCFCRT_itoa_d(char *__pchBuffer, _MCFCRT_STD  intptr_t __nValue) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_itoa_u(char *__pchBuffer, _MCFCRT_STD uintptr_t __uValue) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_itoa_x(char *__pchBuffer, _MCFCRT_STD uintptr_t __uValue) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_itoa_X(char *__pchBuffer, _MCFCRT_STD uintptr_t __uValue) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_itoa0d(char *__pchBuffer, _MCFCRT_STD  intptr_t __nValue, unsigned __uMinDigits) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_itoa0u(char *__pchBuffer, _MCFCRT_STD uintptr_t __uValue, unsigned __uMinDigits) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_itoa0x(char *__pchBuffer, _MCFCRT_STD uintptr_t __uValue, unsigned __uMinDigits) _MCFCRT_NOEXCEPT;
extern char *_MCFCRT_itoa0X(char *__pchBuffer, _MCFCRT_STD uintptr_t __uValue, unsigned __uMinDigits) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
