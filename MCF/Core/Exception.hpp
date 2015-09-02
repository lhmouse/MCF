// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include "LastError.hpp"
#include "../../MCFCRT/ext/stpcpy.h"
#include <exception>

namespace MCF {

namespace Impl_Exception {
	class NtsBuffer {
	public:
		enum : std::size_t {
			kMaxTextLength = 1023
		};

	private:
		char x_achText[kMaxTextLength + 1];

	public:
		explicit NtsBuffer(const char *pszText) noexcept {
			::MCF_stppcpy(x_achText, x_achText + sizeof(x_achText), pszText);
		}
		NtsBuffer(const NtsBuffer &rhs) noexcept {
			::MCF_stpcpy(x_achText, rhs.x_achText);
		}
		NtsBuffer &operator=(const NtsBuffer &rhs) noexcept {
			::MCF_stpcpy(x_achText, rhs.x_achText);
			return *this;
		}

	public:
		operator const char *() const noexcept {
			return x_achText;
		}
		operator char *() noexcept {
			return x_achText;
		}
	};
}

class Exception : public std::exception {
private:
	const char *x_pszFile;
	unsigned long x_ulLine;
	unsigned long x_ulCode;
	Impl_Exception::NtsBuffer x_ntsMessage;

public:
	Exception(const char *pszFile, unsigned long ulLine, unsigned long ulCode, const char *pszMessage) noexcept
		: std::exception()
		, x_pszFile(pszFile), x_ulLine(ulLine), x_ulCode(ulCode), x_ntsMessage(pszMessage)
	{
	}
	~Exception() override;

public:
	const char *what() const noexcept override {
		return x_ntsMessage;
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
		return x_ntsMessage;
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
