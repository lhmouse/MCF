// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_BAIL_HPP_
#define MCF_UTILITIES_BAIL_HPP_

#include "../../MCFCRT/env/bail.h"

namespace MCF {

[[noreturn]] inline
void Bail(const wchar_t *pwszDescription) noexcept {
	::_MCFCRT_Bail(pwszDescription);
}

[[noreturn]] inline
void BailV(const wchar_t *pwszFormat, std::va_list pArgs) noexcept {
	::_MCFCRT_BailV(pwszFormat, pArgs);
}

[[noreturn]] inline /* __attribute__((__format__(__printf__, 1, 2))) */
void BailF(const wchar_t *pwszFormat, ...) noexcept {
	std::va_list pArgs;
	va_start(pArgs, pwszFormat);
	::_MCFCRT_BailV(pwszFormat, pArgs);
	va_end(pArgs);
}

}

#endif
