// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_LOCAL_PTR_HPP_
#define MCF_THREAD_LOCAL_PTR_HPP_

#include "../../MCFCRT/env/thread.h"
#include "../Core/Utilities.hpp"
#include "../Core/UniqueHandle.hpp"
#include <tuple>
#include <exception>
#include <cstddef>
#include <cstdint>

namespace MCF {

namespace Impl {
	template<bool IS_DUMMY>
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

template<class Object_t, class... InitParams_t>
class ThreadLocalPtr {
private:
	typedef Impl::ExceptionWrapper<std::is_nothrow_constructible<Object_t, InitParams_t &&...>::value> xExceptionWrapper;
	typedef typename xExceptionWrapper::ExceptionPtr xExceptionPtr;

	struct TlsKeyDeleter {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *pKey) const noexcept {
			::MCF_CRT_TlsFreeKey(pKey);
		}
	};
	typedef UniqueHandle<TlsKeyDeleter> xTlsIndex;

private:
	static void xTlsCallback(std::intptr_t nValue) noexcept {
		delete (Object_t *)nValue;
	}

private:
	const xTlsIndex xm_nTlsIndex;
	const std::tuple<InitParams_t...> xm_vInitParams;

public:
	explicit constexpr ThreadLocalPtr(InitParams_t &&... vInitParams)
		: xm_nTlsIndex		(::MCF_CRT_TlsAllocKey(&xTlsCallback))
		, xm_vInitParams	(std::forward<InitParams_t>(vInitParams)...)
	{
	}

private:
	Object_t *xDoGetPtr() const noexcept {
		Object_t *pObject;
		{
			std::intptr_t nValue;
			if(::MCF_CRT_TlsGet(xm_nTlsIndex.Get(), &nValue)){
				pObject = (Object_t *)nValue;
			} else {
				pObject = nullptr;
			}
		}
		return pObject;
	}
	Object_t *xDoAllocPtr() const {
		Object_t *pObject;
		{
			std::intptr_t nValue;
			if(::MCF_CRT_TlsGet(xm_nTlsIndex.Get(), &nValue)){
				pObject = (Object_t *)nValue;
			} else {
				pObject = nullptr;
			}
		}
		if(!pObject){
			auto pNewObject = MakeUniqueFromTuple<Object_t>(xm_vInitParams);
			if(!::MCF_CRT_TlsReset(xm_nTlsIndex.Get(), (std::intptr_t)pNewObject.get())){
				throw std::bad_alloc();
			}
			pObject = pNewObject.release();
		}
		return pObject;
	}
	void xDoFreePtr() const noexcept {
		::MCF_CRT_TlsReset(xm_nTlsIndex.Get(), (std::intptr_t)(Object_t *)nullptr);
	}

public:
	const Object_t *GetPtr() const noexcept {
		return xDoGetPtr();
	}
	Object_t *GetPtr() noexcept {
		return xDoGetPtr();
	}
	const Object_t *GetSafePtr() const {
		return xDoAllocPtr();
	}
	Object_t *GetSafePtr(){
		return xDoAllocPtr();
	}

	const Object_t &Get() const {
		return *GetSafePtr();
	}
	Object_t &Get(){
		return *GetSafePtr();
	}

	void Release() noexcept {
		xDoFreePtr();
	}

public:
	explicit operator const Object_t *() const {
		return GetPtr();
	}
	explicit operator Object_t *(){
		return GetPtr();
	}
	const Object_t *operator->() const {
		return GetSafePtr();
	}
	Object_t *operator->(){
		return GetSafePtr();
	}
	const Object_t &operator*() const{
		return Get();
	}
	Object_t &operator*(){
		return Get();
	}
};

}

#endif
