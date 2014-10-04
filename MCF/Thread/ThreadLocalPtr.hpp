// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_LOCAL_PTR_HPP_
#define MCF_THREAD_THREAD_LOCAL_PTR_HPP_

#include "../../MCFCRT/env/thread.h"
#include "../Utilities/NoCopy.hpp"
#include "../Utilities/TupleHelpers.hpp"
#include "../Core/UniqueHandle.hpp"
#include <tuple>
#include <exception>
#include <cstddef>
#include <cstdint>

namespace MCF {

namespace Impl {
	template<bool>
	struct ExceptionWrapper {
		typedef std::exception_ptr ExceptionPtr;

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
		typedef std::nullptr_t ExceptionPtr;

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
class ThreadLocalPtr : NO_COPY {
private:
	using xExceptionWrapper = Impl::ExceptionWrapper<
		std::is_nothrow_constructible<ObjectT, InitParamsT &&...>::value>;
	using xExceptionPtr = typename xExceptionWrapper::ExceptionPtr;

	struct TlsKeyDeleter {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *pKey) const noexcept {
			::MCF_CRT_TlsFreeKey(pKey);
		}
	};
	using xTlsIndex = UniqueHandle<TlsKeyDeleter>;

private:
	static void xTlsCallback(std::intptr_t nValue) noexcept {
		delete (ObjectT *)nValue;
	}

private:
	const xTlsIndex xm_nTlsIndex;
	const std::tuple<InitParamsT...> xm_vInitParams;

public:
	explicit constexpr ThreadLocalPtr(InitParamsT &&...vInitParams)
		: xm_nTlsIndex		(::MCF_CRT_TlsAllocKey(&xTlsCallback))
		, xm_vInitParams	(std::forward<InitParamsT>(vInitParams)...)
	{
	}

private:
	ObjectT *xDoGetPtr() const noexcept {
		ObjectT *pObject;
		{
			std::intptr_t nValue;
			if(::MCF_CRT_TlsGet(xm_nTlsIndex.Get(), &nValue)){
				pObject = (ObjectT *)nValue;
			} else {
				pObject = nullptr;
			}
		}
		return pObject;
	}
	ObjectT *xDoAllocPtr() const {
		ObjectT *pObject;
		{
			std::intptr_t nValue;
			if(::MCF_CRT_TlsGet(xm_nTlsIndex.Get(), &nValue)){
				pObject = (ObjectT *)nValue;
			} else {
				pObject = nullptr;
			}
		}
		if(!pObject){
			auto pNewObject = MakeUniqueFromTuple<ObjectT>(xm_vInitParams);
			if(!::MCF_CRT_TlsReset(xm_nTlsIndex.Get(), (std::intptr_t)pNewObject.get())){
				throw std::bad_alloc();
			}
			pObject = pNewObject.release();
		}
		return pObject;
	}
	void xDoFreePtr() const noexcept {
		::MCF_CRT_TlsReset(xm_nTlsIndex.Get(), (std::intptr_t)(ObjectT *)nullptr);
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
