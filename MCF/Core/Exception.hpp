// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_EXCEPTION_HPP_
#define MCF_EXCEPTION_HPP_

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

extern WideString GetWin32ErrorDesc(unsigned long ulErrorCode);

struct Exception : public std::exception {
	const char *pszFunction;
	unsigned long ulLine;
	unsigned long ulErrorCode;
	const wchar_t *pwszMessage;

	Exception(
		const char *pszFunction_,
		unsigned long ulLine_,
		unsigned long ulErrorCode_,
		const wchar_t *pwszMessage_ = L""
	) noexcept
		: pszFunction	(pszFunction_)
		, ulLine		(ulLine_)
		, ulErrorCode	(ulErrorCode_)
		, pwszMessage	(pwszMessage_)
	{
	}

	virtual const char *what() const noexcept {
		return "MCF::Exception";
	}
};

}

#define MCF_THROW(...)	\
	(throw ::MCF::Exception(__PRETTY_FUNCTION__, __LINE__, __VA_ARGS__))

#define MCF_MAKE_EXCEPTION_PTR(...)	\
	(::std::make_exception_ptr(::MCF::Exception(__PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)))

#endif
