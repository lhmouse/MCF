// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_LOCAL_HPP_
#define MCF_THREAD_THREAD_LOCAL_HPP_

#include "../../MCFCRT/env/thread.h"
#include "../Utilities/Noncopyable.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
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

template<class ObjectT>
class ThreadLocal : NONCOPYABLE {
private:
	using xExceptionWrapper = Impl::ExceptionWrapper<std::is_nothrow_copy_constructible<ObjectT>::value>;
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
	const ObjectT x_vTemplate;
	UniqueHandle<xTlsKeyDeleter> x_pTlsKey;

public:
	explicit ThreadLocal(ObjectT vTemplate = ObjectT())
		: x_vTemplate(std::move(vTemplate))
	{
		if(!x_pTlsKey.Reset(::MCF_CRT_TlsAllocKey([](std::intptr_t nValue) noexcept { delete (ObjectT *)nValue; }))){
			DEBUG_THROW(SystemError, "MCF_CRT_TlsAllocKey");
		}
	}

private:
	ObjectT *xDoGetPtr() const noexcept {
		bool bHasValue;
		std::intptr_t nValue;
		if(!::MCF_CRT_TlsGet(x_pTlsKey.Get(), &bHasValue, &nValue)){
			return nullptr;
		}
		if(!bHasValue){
			return nullptr;
		}
		return reinterpret_cast<ObjectT *>(nValue);
	}
	ObjectT *xDoAllocPtr() const {
		auto pObject = xDoGetPtr();
		if(!pObject){
			pObject = new ObjectT(x_vTemplate);
			if(!::MCF_CRT_TlsReset(x_pTlsKey.Get(), (std::intptr_t)pObject)){ // noexcept
				delete pObject;
				DEBUG_THROW(SystemError, "MCF_CRT_TlsReset");
			}
		}
		return pObject;
	}
	void xDoFreePtr() const noexcept {
		::MCF_CRT_TlsReset(x_pTlsKey.Get(), (std::intptr_t)(ObjectT *)nullptr);
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
