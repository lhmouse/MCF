// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ARGV_H_
#define __MCF_CRT_ARGV_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

typedef struct tagMCF_ArgItem {
	const wchar_t *pwszStr;
	MCF_STD size_t uLen;
} MCF_ArgItem;

extern const MCF_ArgItem *MCF_CRT_AllocArgv(MCF_STD size_t *pArgc, const wchar_t *pwszCommandLine) MCF_NOEXCEPT;
extern const MCF_ArgItem *MCF_CRT_AllocArgvFromCommandLine(MCF_STD size_t *pArgc) MCF_NOEXCEPT; // 根据当前 GetCommandLineW() 的返回值创建。
extern void MCF_CRT_FreeArgv(const MCF_ArgItem *pArgItems) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
