// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_PRE_TLS_H_
#define __MCFCRT_PRE_TLS_H_

#include "../env/_crtdef.h"
#include "../env/_tls_common.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_TlsInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_TlsUninit(void) _MCFCRT_NOEXCEPT;

extern void __MCFCRT_TlsCleanup(void) _MCFCRT_NOEXCEPT;

extern bool _MCFCRT_TlsGet(_MCFCRT_TlsKeyHandle __hTlsKey, void **_MCFCRT_RESTRICT __ppStorage) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_TlsRequire(_MCFCRT_TlsKeyHandle __hTlsKey, void **_MCFCRT_RESTRICT __ppStorage) _MCFCRT_NOEXCEPT;

extern bool _MCFCRT_AtThreadExit(_MCFCRT_AtThreadExitCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
