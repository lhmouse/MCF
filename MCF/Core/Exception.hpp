// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EXCEPTION_HPP__
#define __MCF_EXCEPTION_HPP__

#include "String.hpp"
#include "../../MCFCRT/env/last_error.h"
#include <cstddef>

namespace MCF {

inline unsigned long GetLastError() noexcept {
	return ::__MCF_CRT_GetLastError();
}
inline void SetLastError(unsigned long ulErrorCode) noexcept {
	::__MCF_CRT_SetLastError(ulErrorCode);
}

struct Exception {
	const char *pszFunction;
	unsigned long ulCode;
	const wchar_t *pwszMessage;
};

[[noreturn]]
inline void Throw(
	const char *pszFunction,
	unsigned long ulCode,
	const wchar_t *pwszMessage = L""
){
	throw ::MCF::Exception{pszFunction, ulCode, pwszMessage};
}

}

#define MCF_THROW(...)	::MCF::Throw(__PRETTY_FUNCTION__, __VA_ARGS__)

#endif
