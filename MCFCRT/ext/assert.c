// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "assert.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include "../env/bail.h"

__attribute__((__noreturn__))
int __MCFCRT_OnAssertionFailure(const wchar_t *pwszExpression, const wchar_t *pwszFile, unsigned long ulLine, const wchar_t *pwszMessage){
	wchar_t awcBuffer[1024];
	wchar_t *pwcWrite = _MCFCRT_wcpcpy(awcBuffer, L"调试断言失败。\n\n表达式：");
	pwcWrite = _MCFCRT_wcppcpy(pwcWrite, awcBuffer + 256, pwszExpression); // 如果表达式太长，在此处截断。
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n文件　：");
	pwcWrite = _MCFCRT_wcppcpy(pwcWrite, awcBuffer + 512, pwszFile);       // 如果文件名太长，在此处截断。
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n行号　：");
	pwcWrite = _MCFCRT_itow_u(pwcWrite, ulLine);
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n描述　：");
	pwcWrite = _MCFCRT_wcppcpy(pwcWrite, awcBuffer + 1024, pwszMessage);
	*pwcWrite = 0;
	_MCFCRT_Bail(awcBuffer);
}
