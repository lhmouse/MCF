// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_ARGV_H_
#define __MCFCRT_ENV_ARGV_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

typedef struct MCF_tagArgItem {
	const wchar_t *pwszStr;
	MCF_STD size_t uLen;
} MCF_ArgItem;

extern const MCF_ArgItem *MCFCRT_AllocArgv(MCF_STD size_t *__pArgc, const wchar_t *__pwszCommandLine) MCF_NOEXCEPT;
extern const MCF_ArgItem *MCFCRT_AllocArgvFromCommandLine(MCF_STD size_t *__pArgc) MCF_NOEXCEPT; // 根据当前 GetCommandLineW() 的返回值创建。
extern void MCFCRT_FreeArgv(const MCF_ArgItem *__pArgItems) MCF_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
