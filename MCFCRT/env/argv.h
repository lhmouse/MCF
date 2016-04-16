// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_ARGV_H_
#define __MCFCRT_ENV_ARGV_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

typedef struct _MCFCRT_tagArgItem {
	const wchar_t *__pwszStr;
	_MCFCRT_STD size_t __uLen;
} _MCFCRT_ArgItem;

extern const _MCFCRT_ArgItem *_MCFCRT_AllocArgv(_MCFCRT_STD size_t *__pArgc, const wchar_t *__pwszCommandLine) _MCFCRT_NOEXCEPT;
extern const _MCFCRT_ArgItem *_MCFCRT_AllocArgvFromCommandLine(_MCFCRT_STD size_t *__pArgc) _MCFCRT_NOEXCEPT; // 根据当前 GetCommandLineW() 的返回值创建。
extern void _MCFCRT_FreeArgv(const _MCFCRT_ArgItem *__pArgItems) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
