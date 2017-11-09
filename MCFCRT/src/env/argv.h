// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_ARGV_H_
#define __MCFCRT_ENV_ARGV_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

typedef struct __MCFCRT_tagArgItem {
	const wchar_t *__pwszStr;
	_MCFCRT_STD size_t __uLen;
} _MCFCRT_ArgItem;

extern const _MCFCRT_ArgItem *_MCFCRT_AllocArgv(_MCFCRT_STD size_t *_MCFCRT_RESTRICT __puArgc, const wchar_t *_MCFCRT_RESTRICT __pwszCommandLine) _MCFCRT_NOEXCEPT;
extern const _MCFCRT_ArgItem *_MCFCRT_AllocArgvFromCommandLine(_MCFCRT_STD size_t *_MCFCRT_RESTRICT __puArgc) _MCFCRT_NOEXCEPT; // 根据当前 GetCommandLineW() 的返回值创建。
extern void _MCFCRT_FreeArgv(const _MCFCRT_ArgItem *__pArgItems) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
