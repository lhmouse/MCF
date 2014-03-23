// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EXCEPTION_HPP__
#define __MCF_EXCEPTION_HPP__

#include "../../MCFCRT/env/last_error.h"
#include "String.hpp"
#include <exception>
#include <cstddef>

namespace MCF {

inline unsigned long GetWin32LastError() noexcept {
	return ::__MCF_CRT_GetWin32LastError();
}
inline void SetWin32LastError(unsigned long ulErrorCode) noexcept {
	::__MCF_CRT_SetWin32LastError(ulErrorCode);
}

extern Utf16String GetWin32ErrorDesc(unsigned long ulErrorCode);

struct Exception {
	const char *pszFunction;
	unsigned long ulErrorCode;
	const wchar_t *pwszMessage;

	constexpr Exception(
		const char *pszFunctionParam,
		unsigned long ulErrorCodeParam,
		const wchar_t *pwszMessageParam = L""
	) noexcept
		: pszFunction(pszFunctionParam)
		, ulErrorCode(ulErrorCodeParam)
		, pwszMessage(pwszMessageParam)
	{
	}
};

}

#define MCF_THROW(...)	\
	do {	\
		::MCF::Exception __MCF_TEMP_EXCEPTION__(__PRETTY_FUNCTION__, __VA_ARGS__);	\
		throw __MCF_TEMP_EXCEPTION__;	\
	} while(false)

#define MCF_MAKE_EXCEPTION_PTR(...)	\
	::std::make_exception_ptr(::MCF::Exception(__PRETTY_FUNCTION__, __VA_ARGS__))

#endif
