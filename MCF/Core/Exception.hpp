// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include <exception>
#include "RefCountingNtmbs.hpp"
#include "../../MCFCRT/env/last_error.h"

namespace MCF {

class Exception : public std::exception {
private:
	const char *x_pszFile;
	unsigned long x_ulLine;
	unsigned long x_ulErrorCode;
	RefCountingNtmbs x_rcsErrorMessage;

public:
	Exception(const char *pszFile, unsigned long ulLine, unsigned long ulErrorCode, RefCountingNtmbs rcsErrorMessage) noexcept
		: std::exception()
		, x_pszFile(pszFile), x_ulLine(ulLine), x_ulErrorCode(ulErrorCode), x_rcsErrorMessage(std::move(rcsErrorMessage))
	{
	}
	~Exception() override;

public:
	const char *what() const noexcept override {
		return x_rcsErrorMessage;
	}

	const char *GetFile() const noexcept {
		return x_pszFile;
	}
	unsigned long GetLine() const noexcept {
		return x_ulLine;
	}
	unsigned long GetCode() const noexcept {
		return x_ulErrorCode;
	}
	const char *GetErrorMessage() const noexcept {
		return x_rcsErrorMessage;
	}
};

class SystemException : public Exception {
public:
	SystemException(const char *pszFile, unsigned long ulLine, unsigned long ulErrorCode, RefCountingNtmbs rcsFunction) noexcept
		: Exception(pszFile, ulLine, ulErrorCode, std::move(rcsFunction))
	{
	}
	SystemException(const char *pszFile, unsigned long ulLine, RefCountingNtmbs rcsFunction) noexcept
		: SystemException(pszFile, ulLine, ::MCFCRT_GetWin32LastError(), std::move(rcsFunction))
	{
	}
	~SystemException() override;
};

}

#define DEBUG_THROW(etype_, ...)	\
	([&](const char *file_, unsigned long line_) __attribute__((__noreturn__)) -> char (*)(volatile bool *, volatile int &, double) {	\
		auto &&e_ = etype_(file_, line_, __VA_ARGS__);	\
		throw static_cast<decltype(e_) &&>(e_);	\
	}(__FILE__, __LINE__))

#define DEBUG_MAKE_EXCEPTION_PTR(etype_, ...)	\
	([&](const char *file_, unsigned long line_) -> ::std::exception_ptr {	\
		auto &&e_ = etype_(file_, line_, __VA_ARGS__);	\
		return ::std::make_exception_ptr(static_cast<decltype(e_) &&>(e_));	\
	}(__FILE__, __LINE__))

#endif
