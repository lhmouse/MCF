// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

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
	unsigned long x_ulCode;
	RefCountingNtmbs x_rcsMsg;

public:
	Exception(const char *pszFile, unsigned long ulLine, unsigned long ulCode, RefCountingNtmbs rcsMsg) noexcept
		: std::exception()
		, x_pszFile(pszFile), x_ulLine(ulLine), x_ulCode(ulCode), x_rcsMsg(std::move(rcsMsg))
	{
	}
	~Exception() override;

public:
	const char *what() const noexcept override {
		return x_rcsMsg;
	}

	const char *GetFile() const noexcept {
		return x_pszFile;
	}
	unsigned long GetLine() const noexcept {
		return x_ulLine;
	}
	unsigned long GetCode() const noexcept {
		return x_ulCode;
	}
	const char *GetMsg() const noexcept {
		return x_rcsMsg;
	}
};

class SystemError : public Exception {
public:
	SystemError(const char *pszFile, unsigned long ulLine, unsigned long ulCode, RefCountingNtmbs rcsFunc) noexcept
		: Exception(pszFile, ulLine, ulCode, std::move(rcsFunc))
	{
	}
	SystemError(const char *pszFile, unsigned long ulLine, RefCountingNtmbs rcsFunc) noexcept
		: SystemError(pszFile, ulLine, ::MCF_CRT_GetWin32LastError(), std::move(rcsFunc))
	{
	}
	~SystemError() override;
};

}

#define DEBUG_THROW(etype_, ...)	\
	(throw [&](const char *file_, unsigned long line_){ return etype_(file_, line_, __VA_ARGS__); }(__FILE__, __LINE__))

#define DEBUG_MAKE_EXCEPTION_PTR(etype_, ...)	\
	(::std::make_exception_ptr([&](const char *file_, unsigned long line_){ return etype_(file_, line_, __VA_ARGS__); }(__FILE__, __LINE__)))

#endif
