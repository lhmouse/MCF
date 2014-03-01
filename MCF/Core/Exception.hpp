// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EXCEPTION_HPP__
#define __MCF_EXCEPTION_HPP__

#include "../../MCFCRT/env/last_error.h"
#include <exception>
#include <cstddef>

namespace MCF {

inline unsigned long GetWin32LastError() noexcept {
	return ::__MCF_CRT_GetWin32LastError();
}
inline void SetWin32LastError(unsigned long ulErrorCode) noexcept {
	::__MCF_CRT_SetWin32LastError(ulErrorCode);
}

struct Exception {
	const char *m_pszFunction;
	unsigned long m_ulCode;
	const wchar_t *m_pwszMessage;

	constexpr Exception(
		const char *pszFunction,
		unsigned long ulCode,
		const wchar_t *pwszMessage = L""
	) noexcept
		: m_pszFunction(pszFunction)
		, m_ulCode(ulCode)
		, m_pwszMessage(pwszMessage)
	{
	}
};

}

#define MCF_THROW(...)	\
	throw ::MCF::Exception(__PRETTY_FUNCTION__, __VA_ARGS__)

#define MCF_MAKE_EXCEPTION_PTR(...)	\
	::std::make_exception_ptr(::MCF::Exception(__PRETTY_FUNCTION__, __VA_ARGS__))

#endif
