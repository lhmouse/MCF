// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include "../../MCFCRT/env/last_error.h"
#include "String.hpp"
#include <stdexcept>
#include <utility>
#include <cstddef>

namespace MCF {

inline unsigned long GetWin32LastError() noexcept {
	return ::MCF_CRT_GetWin32LastError();
}
inline void SetWin32LastError(unsigned long ulErrorCode) noexcept {
	::MCF_CRT_SetWin32LastError(ulErrorCode);
}

inline WideString GetWin32ErrorDesc(unsigned long ulErrorCode){
	WideString wsRet;
	wchar_t *pwszDesc = nullptr;
	const auto uLen = ::MCF_CRT_AllocWin32ErrorDesc(&pwszDesc, ulErrorCode);
	try {
		wsRet.Append(pwszDesc, uLen);
		::MCF_CRT_FreeWin32ErrorDesc(pwszDesc);
	} catch(...){
		::MCF_CRT_FreeWin32ErrorDesc(pwszDesc);
		throw;
	}
	return std::move(wsRet);
}

class Exception : public std::exception {
public:
	const char *const m_pszFunction;
	const unsigned long m_ulLine;
	const unsigned long m_ulErrorCode;
	const WideString m_wcsMessage;

public:
	// 确保参数的传递都不影响 ::GetLastError() 的返回值。
	template<typename ...ParamsT>
	Exception(const char *pszFunction, unsigned long ulLine, unsigned long ulErrorCode,
		ParamsT &&...vParams)
		: m_pszFunction(pszFunction), m_ulLine(ulLine), m_ulErrorCode(ulErrorCode)
		, m_wcsMessage(std::forward<ParamsT>(vParams)...)
	{
	}

public:
	virtual const char *what() const noexcept {
		return "MCF::Exception";
	}
};

}

#define MCF_THROW(code_, ...)	\
	(throw ::MCF::Exception(__PRETTY_FUNCTION__, __LINE__, code_, __VA_ARGS__))

#define MCF_MAKE_EXCEPTION_PTR(code_, ...)	\
	(::std::make_exception_ptr(::MCF::Exception(__PRETTY_FUNCTION__, __LINE__, code_, __VA_ARGS__)))

#endif
