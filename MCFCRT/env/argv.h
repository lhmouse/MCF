// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_ARGV_H_
#define __MCFCRT_ENV_ARGV_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

typedef struct MCFCRT_tagArgItem {
	const wchar_t *pwszStr;
	MCFCRT_STD size_t uLen;
} MCFCRT_ArgItem;

extern const MCFCRT_ArgItem *MCFCRT_AllocArgv(MCFCRT_STD size_t *__pArgc, const wchar_t *__pwszCommandLine) MCFCRT_NOEXCEPT;
extern const MCFCRT_ArgItem *MCFCRT_AllocArgvFromCommandLine(MCFCRT_STD size_t *__pArgc) MCFCRT_NOEXCEPT; // 根据当前 GetCommandLineW() 的返回值创建。
extern void MCFCRT_FreeArgv(const MCFCRT_ArgItem *__pArgItems) MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
