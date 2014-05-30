// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_EXCEPTION_HPP_
#define MCF_EXCEPTION_HPP_

#include "../../MCFCRT/env/last_error.h"
#include "String.hpp"
#include <exception>
#include <utility>
#include <cstddef>

namespace MCF {

inline unsigned long GetWin32LastError() noexcept {
	return ::__MCF_CRT_GetWin32LastError();
}
inline void SetWin32LastError(unsigned long ulErrorCode) noexcept {
	::__MCF_CRT_SetWin32LastError(ulErrorCode);
}

extern WideString GetWin32ErrorDesc(unsigned long ulErrorCode);

class Exception : public std::exception {
public:
	const char *m_pszFunction;
	unsigned long m_ulLine;
	unsigned long m_ulErrorCode;
	WideString m_wcsMessage;

private:
	void xMakeMessage() noexcept {
	}
	void xMakeMessage(const WideStringObserver &wsoMessage){
		m_wcsMessage = wsoMessage;
	}
	void xMakeMessage(WideString wcsMessage){
		m_wcsMessage = std::move(wcsMessage);
	}

public:
	// 确保参数的传递都不影响 ::GetLastError() 的返回值。
	template<typename... Params_t>
	Exception(
		const char *pszFunction,
		unsigned long ulLine,
		unsigned long ulErrorCode,
		Params_t &&... vParams
	) noexcept
		: m_pszFunction	(pszFunction)
		, m_ulLine		(ulLine)
		, m_ulErrorCode	(ulErrorCode)
	{
		xMakeMessage(std::forward<Params_t>(vParams)...);
	}

public:
	virtual const char *what() const noexcept {
		return "MCF::Exception";
	}
};

}

#define MCF_THROW(code, ...)	\
	throw ::MCF::Exception(__PRETTY_FUNCTION__, __LINE__, code, __VA_ARGS__);	\

#define MCF_MAKE_EXCEPTION_PTR(code, ...)	\
	out_ptr = ::std::make_exception_ptr(::MCF::Exception(__PRETTY_FUNCTION__, __LINE__, code, __VA_ARGS__));	\

#endif
