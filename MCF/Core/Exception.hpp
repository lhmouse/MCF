// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include "../../MCFCRT/env/last_error.h"
#include "../../MCFCRT/ext/stpcpy.h"
#include <exception>

namespace MCF {

namespace Impl_Exception {
	class NtsBuffer {
	public:
		enum : std::size_t {
			kMaxTextLength = 159
		};

	private:
		char $achText[kMaxTextLength + 1];

	public:
		explicit NtsBuffer(const char *pszText) noexcept {
			::MCF_stppcpy($achText, $achText + sizeof($achText), pszText);
		}
		NtsBuffer(const NtsBuffer &rhs) noexcept {
			::MCF_stpcpy($achText, rhs.$achText);
		}
		NtsBuffer &operator=(const NtsBuffer &rhs) noexcept {
			::MCF_stpcpy($achText, rhs.$achText);
			return *this;
		}

	public:
		operator const char *() const noexcept {
			return $achText;
		}
		operator char *() noexcept {
			return $achText;
		}
	};
}

class Exception : public std::exception {
private:
	const char *$pszFile;
	unsigned long $ulLine;
	unsigned long $ulCode;
	Impl_Exception::NtsBuffer $ntsMessage;

public:
	Exception(const char *pszFile, unsigned long ulLine, unsigned long ulCode, const char *pszMessage) noexcept
		: std::exception()
		, $pszFile(pszFile), $ulLine(ulLine), $ulCode(ulCode), $ntsMessage(pszMessage)
	{
	}
	~Exception() override;

public:
	const char *what() const noexcept override {
		return $ntsMessage;
	}

	const char *GetFile() const noexcept {
		return $pszFile;
	}
	unsigned long GetLine() const noexcept {
		return $ulLine;
	}
	unsigned long GetCode() const noexcept {
		return $ulCode;
	}
	const char *GetMessage() const noexcept {
		return $ntsMessage;
	}
};

class SystemError : public Exception {
public:
	SystemError(const char *pszFile, unsigned long ulLine, unsigned long ulCode, const char *pszFunction) noexcept
		: Exception(pszFile, ulLine, ulCode, pszFunction)
	{
	}
	SystemError(const char *pszFile, unsigned long ulLine, const char *pszFunction) noexcept
		: SystemError(pszFile, ulLine, ::MCF_CRT_GetWin32LastError(), pszFunction)
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
