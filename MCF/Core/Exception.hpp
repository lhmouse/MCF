// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include <exception>
#include <typeinfo>
#include <utility>
#include "Rcnts.hpp"

namespace MCF {

class Exception : public std::exception {
public:
	static void RethrowCurrentNestedExceptionIfExists();

private:
	const char *x_pszFile;
	unsigned long x_ulLine;
	unsigned long x_ulErrorCode;
	std::exception_ptr x_pNestedException;

	Rcntws x_rcwsErrorMessage;

public:
	Exception(const char *pszFile, unsigned long ulLine, unsigned long ulErrorCode, std::exception_ptr pNestedException,
		Rcntws rcwsErrorMessage) noexcept
		: x_pszFile(pszFile), x_ulLine(ulLine), x_ulErrorCode(ulErrorCode), x_pNestedException(std::move(pNestedException))
		, x_rcwsErrorMessage(std::move(rcwsErrorMessage))
	{
	}
	~Exception() override;

public:
	const char *what() const noexcept override {
		return typeid(*this).name();
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
	const std::exception_ptr &GetNestedException() const noexcept {
		return x_pNestedException;
	}
	[[noreturn]] RethrowNestedException() const {
		if(!x_pNestedException){
			std::terminate();
		}
		std::rethrow_exception(x_pNestedException);
	}

	const Rcntws &GetErrorMessage() const noexcept {
		return x_rcwsErrorMessage;
	}
};

namespace Impl_Exception {
	struct DummyReturnType {
		template<typename T>
		[[noreturn]] operator T &() const && noexcept {
			std::terminate();
		}
		template<typename T>
		[[noreturn]] operator T &&() const && noexcept {
			std::terminate();
		}
	};

	// 这个返回类型允许在三目运算符中调用该函数模板。
	template<typename ExceptionT, typename ...ParamsT>
	[[noreturn]] DummyReturnType DebugThrow(const char *pszFile, unsigned long ulLine, unsigned long ulErrorCode, std::exception_ptr pNestedException,
		ParamsT &&...vParams)
	{
		throw ExceptionT(pszFile, ulLine, ulErrorCode, std::move(pNestedException),
			std::forward<ParamsT>(vParams)...);
	}
	template<typename ExceptionT, typename ...ParamsT>
	std::exception_ptr DebugMakeExceptionPtr(const char *pszFile, unsigned long ulLine, unsigned long ulErrorCode, std::exception_ptr pNestedException,
		ParamsT &&...vParams)
	{
		return std::make_exception_ptr(ExceptionT(pszFile, ulLine, ulErrorCode, std::move(pNestedException),
			std::forward<ParamsT>(vParams)...));
	}
}

}

#define DEBUG_THROW(etype_, code_, ...)	\
	(::MCF::Impl_Exception::DebugThrow<etype_>(__FILE__, __LINE__, (code_), (::std::exception_ptr()),	\
		__VA_ARGS__))

#define DEBUG_THROW_NESTED(etype_, code_, ...)	\
	(::MCF::Impl_Exception::DebugThrow<etype_>(__FILE__, __LINE__, (code_), (::std::current_exception()),	\
		__VA_ARGS__))

#define DEBUG_MAKE_EXCEPTION_PTR(etype_, code_, ...)	\
	(::MCF::Impl_Exception::DebugMakeExceptionPtr<etype_>(__FILE__, __LINE__, (code_), (::std::exception_ptr()),	\
		__VA_ARGS__))

#define DEBUG_MAKE_NESTED_EXCEPTION_PTR(etype_, code_, ...)	\
	(::MCF::Impl_Exception::DebugMakeExceptionPtr<etype_>(__FILE__, __LINE__, (code_), (::std::current_exception()),	\
		__VA_ARGS__))

#endif
