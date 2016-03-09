// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include <exception>
#include <utility>
#include "RefCountingNtmbs.hpp"
#include "../../MCFCRT/env/last_error.h"

namespace MCF {

namespace Impl_Exception {
	struct DummyReturnType {
		template<typename T>
		[[noreturn]] operator T() const noexcept {
			std::terminate();
		}
	};

	template<typename ExceptionT, typename ...ParamsT>
	[[noreturn]] DummyReturnType DebugThrow(const char *pszFile, unsigned long ulLine, ParamsT &&...vParams){
		throw ExceptionT(pszFile, ulLine, std::forward<ParamsT>(vParams)...);
	}
	template<typename ExceptionT, typename ...ParamsT>
	std::exception_ptr DebugMakeExceptionPtr(const char *pszFile, unsigned long ulLine, ParamsT &&...vParams){
		return std::make_exception_ptr(ExceptionT(pszFile, ulLine, std::forward<ParamsT>(vParams)...));
	}
}

class Exception : public std::exception {
private:
	const char *x_pszFile;
	unsigned long x_ulLine;
	unsigned long x_ulErrorCode;
	RefCountingNtmbs x_rcsErrorMessage;

public:
	Exception(const char *pszFile, unsigned long ulLine, unsigned long ulErrorCode, RefCountingNtmbs rcsErrorMessage) noexcept
		: x_pszFile(pszFile), x_ulLine(ulLine), x_ulErrorCode(ulErrorCode), x_rcsErrorMessage(std::move(rcsErrorMessage))
	{
	}
	Exception(const char *pszFile, unsigned long ulLine, long lErrorCode, RefCountingNtmbs rcsErrorMessage) noexcept
		: Exception(pszFile, ulLine, static_cast<unsigned long>(lErrorCode), std::move(rcsErrorMessage))
	{
	}
	~Exception() override;

	Exception(const Exception &) noexcept = default;
	Exception(Exception &&) noexcept = default;
	Exception &operator=(const Exception &) noexcept = default;
	Exception &operator=(Exception &&) noexcept = default;

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
	unsigned long GetErrorCode() const noexcept {
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
	SystemException(const char *pszFile, unsigned long ulLine, long lErrorCode, RefCountingNtmbs rcsErrorMessage) noexcept
		: SystemException(pszFile, ulLine, static_cast<unsigned long>(lErrorCode), std::move(rcsErrorMessage))
	{
	}
	~SystemException() override;

	SystemException(const SystemException &) noexcept = default;
	SystemException(SystemException &&) noexcept = default;
	SystemException &operator=(const SystemException &) noexcept = default;
	SystemException &operator=(SystemException &&) noexcept = default;
};

}

#define DEBUG_THROW(etype_, ...)	\
	(::MCF::Impl_Exception::DebugThrow<etype_>(__FILE__, __LINE__, __VA_ARGS__))

#define DEBUG_MAKE_EXCEPTION_PTR(etype_, ...)	\
	(::MCF::Impl_Exception::DebugMakeExceptionPtr<etype_>(__FILE__, __LINE__, __VA_ARGS__))

#endif
