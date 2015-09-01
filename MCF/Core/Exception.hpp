// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include "LastError.hpp"
#include <exception>
#include <utility>
#include <cstring>

namespace MCF {

class Exception : public std::exception {
private:
	const char *x_pszFile;
	unsigned long x_ulLine;
	unsigned long x_ulCode;
	char x_achMessage[1024];

public:
	Exception(const char *pszFile, unsigned long ulLine, unsigned long ulCode, const char *pszMessage) noexcept
		: std::exception()
		, x_pszFile(pszFile), x_ulLine(ulLine), x_ulCode(ulCode)
	{
		const auto uLen = std::min(std::strlen(pszMessage), sizeof(x_achMessage) - 1);
		std::memcpy(x_achMessage, pszMessage, uLen);
		x_achMessage[uLen] = 0;
	}
	Exception(const Exception &rhs) noexcept
		: std::exception(static_cast<const std::exception &>(rhs))
		, x_pszFile(rhs.x_pszFile), x_ulLine(rhs.x_ulLine), x_ulCode(rhs.x_ulCode)
	{
		std::strcpy(x_achMessage, rhs.x_achMessage);
	}
	Exception &operator=(const Exception &rhs) noexcept {
		std::exception::operator=(static_cast<const std::exception &>(rhs));

		x_pszFile = rhs.x_pszFile;
		x_ulLine  = rhs.x_ulLine;
		x_ulCode  = rhs.x_ulCode;
		std::strcpy(x_achMessage, rhs.x_achMessage);
		return *this;
	}
	~Exception() override;

public:
	const char *what() const noexcept override {
		return x_achMessage;
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
		return x_achMessage;
	}
};

class SystemError : public Exception {
public:
	SystemError(const char *pszFile, unsigned long ulLine, const char *pszFunction, unsigned long ulCode = GetWin32LastError()) noexcept
		: Exception(pszFile, ulLine, ulCode, pszFunction)
	{
	}
	~SystemError() override;
};

}

#define DEBUG_THROW(etype_, ...)	\
	([&]() [[noreturn]] {	\
		etype_ e_ (__FILE__, __LINE__, __VA_ARGS__);	\
		throw e_;	\
	}())

#endif
