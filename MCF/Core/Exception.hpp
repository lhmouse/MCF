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
	RefCountingNtmbs x_rcsMessage;

public:
	Exception(const char *pszFile, unsigned long ulLine, unsigned long ulCode, RefCountingNtmbs rcsMessage) noexcept
		: std::exception()
		, x_pszFile(pszFile), x_ulLine(ulLine), x_ulCode(ulCode), x_rcsMessage(std::move(rcsMessage))
	{
	}
	~Exception() override;

public:
	const char *what() const noexcept override {
		return x_rcsMessage;
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
	const char *GetMessage() const noexcept {
		return x_rcsMessage;
	}
};

class SystemError : public Exception {
public:
	SystemError(const char *pszFile, unsigned long ulLine, unsigned long ulCode, RefCountingNtmbs rcsFunction) noexcept
		: Exception(pszFile, ulLine, ulCode, std::move(rcsFunction))
	{
	}
	SystemError(const char *pszFile, unsigned long ulLine, RefCountingNtmbs rcsFunction) noexcept
		: SystemError(pszFile, ulLine, ::MCF_CRT_GetWin32LastError(), std::move(rcsFunction))
	{
	}
	~SystemError() override;
};

}

#define DEBUG_THROW(etype_, ...)	\
	__extension__ ({	\
		etype_ e_ (__FILE__, __LINE__, __VA_ARGS__);	\
		throw e_;	\
	})

#endif
