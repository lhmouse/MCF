// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "bail.h"
#include "mcfwin.h"
#include "../ext/wcpcpy.h"
#include <stdarg.h>
#include <wchar.h>

__attribute__((__noreturn__))
static void DoBail(const wchar_t *pwszDescription){
	wchar_t awcBuffer[1024 + 256];
	wchar_t *pwcWrite = MCF_wcpcpy(awcBuffer, L"应用程序异常终止，请联系作者寻求协助。");
	if(pwszDescription){
		pwcWrite = MCF_wcpcpy(pwcWrite, L"\n\n错误描述：\n");

		size_t uLen = wcslen(pwszDescription);
		const size_t uMax = (size_t)(awcBuffer + sizeof(awcBuffer) / sizeof(wchar_t) - pwcWrite) - 64; // 后面还有一些内容，保留一些字符。
		if(uLen > uMax){
			uLen = uMax;
		}
		wmemcpy(pwcWrite, pwszDescription, uLen);
		pwcWrite += uLen;
	}
	pwcWrite = MCF_wcpcpy(pwcWrite, L"\n\n单击“确定”终止应用程序。\n");

	const HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	if(hStdErr != INVALID_HANDLE_VALUE){
		DWORD dwMode;
		if(GetConsoleMode(hStdErr, &dwMode)){
			const wchar_t *pwcRead = awcBuffer;
			for(;;){
				const DWORD dwCharsToWrite = (DWORD)(pwcWrite - pwcRead);
				if(dwCharsToWrite == 0){
					break;
				}
				DWORD dwCharsWritten;
				if(!WriteConsoleW(hStdErr, pwcRead, dwCharsToWrite, &dwCharsWritten, nullptr)){
					break;
				}
				pwcRead += dwCharsWritten;
			}
		}
	}

	pwcWrite -= 2;
	*pwcWrite = 0;

	FatalAppExitW(0, awcBuffer);
	__builtin_trap();
}

__attribute__((__noreturn__))
void MCF_CRT_Bail(const wchar_t *pwszDescription){
	DoBail(pwszDescription);
}
__attribute__((__noreturn__))
void MCF_CRT_BailF(const wchar_t *pwszFormat, ...){
	wchar_t awcBuffer[1024];
	va_list ap;
	va_start(ap, pwszFormat);
	vswprintf(awcBuffer, sizeof(awcBuffer) / sizeof(wchar_t), pwszFormat, ap);
	va_end(ap);
	DoBail(awcBuffer);
}
