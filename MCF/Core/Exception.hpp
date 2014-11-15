// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include "../../MCFCRT/env/last_error.h"
#include <stdexcept>

namespace MCF {

class Exception : public std::exception {
private:
	const char *const xm_pszFile;
	const unsigned long xm_ulLine;

	const char *const xm_pszMessage;
	const unsigned long xm_ulCode;

public:
	Exception(const char *pszFile, unsigned long ulLine,
		const char *pszMessage, unsigned long ulCode) noexcept
		: xm_pszFile(pszFile), xm_ulLine(ulLine), xm_pszMessage(pszMessage), xm_ulCode(ulCode)
	{
	}
	~Exception() override;

public:
	const char *what() const noexcept override {
		return xm_pszMessage;
	}

	const char *GetMessage() const noexcept {
		return xm_pszMessage;
	}
	unsigned long GetCode() const noexcept {
		return xm_ulCode;
	}
};

class SystemError : public Exception {
public:
	SystemError(const char *pszFile, unsigned long ulLine,
		const char *pszMessage, unsigned long ulCode = ::MCF_CRT_GetWin32LastError()) noexcept
		: Exception(pszFile, ulLine, pszMessage, ulCode)
	{
	}
	~SystemError() override;
};

}

#define DEBUG_THROW(etype_, ...)	(throw etype_(__FILE__, __LINE__, __VA_ARGS__))

#endif
