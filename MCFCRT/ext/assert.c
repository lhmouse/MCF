// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.h"
#include "assert.h"
#include "../env/bail.h"

__attribute__((__noreturn__))
int __MCFCRT_OnAssertionFailure(const wchar_t *pwszExpression, const char *pszFile, unsigned long ulLine, const wchar_t *pwszMessage){
	_MCFCRT_BailF(L"调试断言失败。\n\n表达式：%ls\n文件　：%hs\n行号　：%lu\n描述　：%ls", pwszExpression, pszFile, ulLine, pwszMessage);
}
