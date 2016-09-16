// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_LAST_ERROR_HPP_
#define MCF_CORE_LAST_ERROR_HPP_

#include "../Config.hpp"
#include <MCFCRT/env/last_error.h>
#include "StringView.hpp"

namespace MCF {

inline unsigned long GetLastWin32Error() noexcept {
	return ::_MCFCRT_GetLastWin32Error();
}
inline void SetLastWin32Error(unsigned long ulErrorCode) noexcept {
	::_MCFCRT_SetLastWin32Error(ulErrorCode);
}

inline WideStringView GetWin32ErrorDescription(unsigned long ulErrorCode) noexcept {
	const wchar_t *pwszStr;
	const auto uLength = ::_MCFCRT_GetWin32ErrorDescription(&pwszStr, ulErrorCode);
	return WideStringView(pwszStr, uLength);
}

}

#endif
