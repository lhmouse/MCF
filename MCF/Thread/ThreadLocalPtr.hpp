// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_LOCAL_PTR_HPP_
#define MCF_THREAD_THREAD_LOCAL_PTR_HPP_

#include "../../MCFCRT/env/thread.h"
#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/TupleHelpers.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
#include <tuple>
#include <exception>
#include <cstddef>
#include <cstdint>

namespace MCF {

namespace Impl {
	template<bool>
	struct ExceptionWrapper {
		using ExceptionPtr = std::exception_ptr;

		static ExceptionPtr GetCurrentException() noexcept {
			return std::current_exception();
		}
		[[noreturn]]
		static void RethrowException(ExceptionPtr pException){
			std::rethrow_exception(std::move(pException));
		}
	};
	template<>
	struct ExceptionWrapper<true> {
		using ExceptionPtr = std::nullptr_t;

		static ExceptionPtr GetCurrentException() noexcept {
			return nullptr;
		}
		[[noreturn]]
		static void RethrowException(ExceptionPtr) noexcept {
			__builtin_trap();
		}
	};
}

template<class ObjectT, class ...InitParamsT>
class ThreadLocalPtr : NONCOPYABLE {
private:
	using xExceptionWrapper = Impl::ExceptionWrapper<
		std::is_nothrow_constructible<ObjectT, InitParamsT &&...>::value>;
	using xExceptionPtr = typename xExceptionWrapper::ExceptionPtr;

	struct xTlsKeyDeleter {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *pKey) const noexcept {
			::MCF_CRT_TlsFreeKey(pKey);
		}
	};

private:
	const std::tuple<InitParamsT...> xm_vInitParams;
	UniqueHandle<xTlsKeyDeleter> xm_pTlsKey;

public:
	explicit constexpr ThreadLocalPtr(InitParamsT &&...vInitParams)
		: xm_vInitParams(std::forward<InitParamsT>(vInitParams)...)
	{
		if(!xm_pTlsKey.Reset(::MCF_CRT_TlsAllocKey(
			[](std::intptr_t nValue) noexcept { delete reinterpret_cast<ObjectT *>(nValue); })))
		{
			DEBUG_THROW(SystemError, "MCF_CRT_TlsAllocKey");
		}
	}

private:
	ObjectT *xDoGetPtr() const noexcept {
		ObjectT *pObject = nullptr;
		std::intptr_t nValue;
		if(::MCF_CRT_TlsGet(xm_pTlsKey.Get(), &nValue)){
			pObject = reinterpret_cast<ObjectT *>(nValue);
		}
		return pObject;
	}
	ObjectT *xDoAllocPtr() const {
		auto pObject = xDoGetPtr();
		if(!pObject){
			auto pNewObject = MakeUniqueFromTuple<ObjectT>(xm_vInitParams);
			if(!::MCF_CRT_TlsReset(xm_pTlsKey.Get(), reinterpret_cast<std::intptr_t>(pNewObject.get()))){
				DEBUG_THROW(SystemError, "MCF_CRT_TlsReset");
			}
			pObject = pNewObject.release();
		}
		return pObject;
	}
	void xDoFreePtr() const noexcept {
		::MCF_CRT_TlsReset(xm_pTlsKey.Get(), reinterpret_cast<std::intptr_t>(static_cast<ObjectT *>(nullptr)));
	}

public:
	const ObjectT *GetPtr() const noexcept {
		return xDoGetPtr();
	}
	ObjectT *GetPtr() noexcept {
		return xDoGetPtr();
	}
	const ObjectT *GetSafePtr() const {
		return xDoAllocPtr();
	}
	ObjectT *GetSafePtr(){
		return xDoAllocPtr();
	}

	const ObjectT &Get() const {
		return *GetSafePtr();
	}
	ObjectT &Get(){
		return *GetSafePtr();
	}

	void Release() noexcept {
		xDoFreePtr();
	}

public:
	explicit operator const ObjectT *() const {
		return GetPtr();
	}
	explicit operator ObjectT *(){
		return GetPtr();
	}
	const ObjectT *operator->() const {
		return GetSafePtr();
	}
	ObjectT *operator->(){
		return GetSafePtr();
	}
	const ObjectT &operator*() const{
		return Get();
	}
	ObjectT &operator*(){
		return Get();
	}
};

}

#endif
