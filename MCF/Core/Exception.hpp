// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include <exception>
#include <utility>
#include "Rcnts.hpp"
#include "../Utilities/DeclVal.hpp"

namespace MCF {

namespace Impl_Exception {
	class ExceptionContext : public virtual std::exception {
	private:
		const char *x_pszFile;
		unsigned long x_ulLine;
		const char *x_pszFunction;
		std::exception_ptr x_pNestedException;

	public:
		ExceptionContext() noexcept
			: ExceptionContext("<Unknown>", 0, "<Unknown>", std::exception_ptr())
		{
		}
		ExceptionContext(const char *pszFile, unsigned long ulLine, const char *pszFunction, std::exception_ptr pNestedException) noexcept
			: x_pszFile(pszFile), x_ulLine(ulLine), x_pszFunction(pszFunction), x_pNestedException(std::move(pNestedException))
		{
		}
		~ExceptionContext() override;

	public:
		const char *what() const noexcept override {
			return x_pszFunction;
		}

		const char *GetFile() const noexcept {
			return x_pszFile;
		}
		unsigned long GetLine() const noexcept {
			return x_ulLine;
		}
		const char *GetFunction() const noexcept {
			return x_pszFunction;
		}
		const std::exception_ptr &GetNestedException() const noexcept {
			return x_pNestedException;
		}
	};
}

class Exception : public Impl_Exception::ExceptionContext {
private:
	unsigned long x_ulErrorCode;
	Rcntws x_rcwsErrorMessage;

public:
	template<typename CodeT, std::enable_if_t<
		sizeof(static_cast<unsigned long>(DeclVal<const CodeT &>())),
		int> = 0>
	Exception(const CodeT &vCode, Rcntws rcwsErrorMessage) noexcept
		: x_ulErrorCode(static_cast<unsigned long>(vCode)), x_rcwsErrorMessage(std::move(rcwsErrorMessage))
	{
	}
	~Exception() override;

public:
	unsigned long GetErrorCode() const noexcept {
		return x_ulErrorCode;
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
	[[noreturn]]
	DummyReturnType DebugThrow(
		const char *pszFile, unsigned long ulLine, const char *pszFunction, std::exception_ptr pNestedException,
		ParamsT &&...vParams)
	try {
		throw ExceptionT(std::forward<ParamsT>(vParams)...);
	} catch(ExceptionContext &e){
		e = ExceptionContext(pszFile, ulLine, pszFunction, std::move(pNestedException));
		throw;
	}
	template<typename ExceptionT, typename ...ParamsT>
	std::exception_ptr DebugMakeExceptionPtr(
		const char *pszFile, unsigned long ulLine, const char *pszFunction, std::exception_ptr pNestedException,
		ParamsT &&...vParams)
	try {
		DebugThrow(pszFile, ulLine, pszFunction, std::move(pNestedException), std::forward<ParamsT>(vParams)...);
	} catch(...){
		return std::current_exception();
	}
}

}

#define MCF_THROW(etype_, ...)	\
	(::MCF::Impl_Exception::DebugThrow<etype_>(	\
		__FILE__, __LINE__, __PRETTY_FUNCTION__, ::std::exception_ptr(),	\
		__VA_ARGS__))

#define MCF_THROW_NESTED(etype_, ...)	\
	(::MCF::Impl_Exception::DebugThrow<etype_>(	\
		__FILE__, __LINE__, __PRETTY_FUNCTION__, ::std::current_exception(),	\
		__VA_ARGS__))

#define MCF_MAKE_EXCEPTION_PTR(etype_, ...)	\
	(::MCF::Impl_Exception::DebugMakeExceptionPtr<etype_>(	\
		__FILE__, __LINE__, __PRETTY_FUNCTION__, ::std::exception_ptr(),	\
		__VA_ARGS__))

#define MCF_MAKE_NESTED_EXCEPTION_PTR(etype_, ...)	\
	(::MCF::Impl_Exception::DebugMakeExceptionPtr<etype_>(	\
		__FILE__, __LINE__, __PRETTY_FUNCTION__, ::std::current_exception(),	\
		__VA_ARGS__))

#endif
