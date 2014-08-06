// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

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

template<class Object, class... InitParams>
class ThreadLocalPtr {
private:
	typedef Impl::ExceptionWrapper<std::is_nothrow_constructible<Object, InitParams &&...>::value> xExceptionWrapper;
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
		delete (Object *)nValue;
	}

private:
	const xTlsIndex xm_nTlsIndex;
	const std::tuple<InitParams...> xm_vInitParams;

public:
	explicit constexpr ThreadLocalPtr(InitParams &&... vInitParams)
		: xm_nTlsIndex		(::MCF_CRT_TlsAllocKey(&xTlsCallback))
		, xm_vInitParams	(std::forward<InitParams>(vInitParams)...)
	{
	}

private:
	Object *xDoGetPtr() const noexcept {
		Object *pObject;
		{
			std::intptr_t nValue;
			if(::MCF_CRT_TlsGet(xm_nTlsIndex.Get(), &nValue)){
				pObject = (Object *)nValue;
			} else {
				pObject = nullptr;
			}
		}
		return pObject;
	}
	Object *xDoAllocPtr() const {
		Object *pObject;
		{
			std::intptr_t nValue;
			if(::MCF_CRT_TlsGet(xm_nTlsIndex.Get(), &nValue)){
				pObject = (Object *)nValue;
			} else {
				pObject = nullptr;
			}
		}
		if(!pObject){
			auto pNewObject = MakeUniqueFromTuple<Object>(xm_vInitParams);
			if(!::MCF_CRT_TlsReset(xm_nTlsIndex.Get(), (std::intptr_t)pNewObject.get())){
				throw std::bad_alloc();
			}
			pObject = pNewObject.release();
		}
		return pObject;
	}
	void xDoFreePtr() const noexcept {
		::MCF_CRT_TlsReset(xm_nTlsIndex.Get(), (std::intptr_t)(Object *)nullptr);
	}

public:
	const Object *GetPtr() const noexcept {
		return xDoGetPtr();
	}
	Object *GetPtr() noexcept {
		return xDoGetPtr();
	}
	const Object *GetSafePtr() const {
		return xDoAllocPtr();
	}
	Object *GetSafePtr(){
		return xDoAllocPtr();
	}

	const Object &Get() const {
		return *GetSafePtr();
	}
	Object &Get(){
		return *GetSafePtr();
	}

	void Release() noexcept {
		xDoFreePtr();
	}

public:
	explicit operator const Object *() const {
		return GetPtr();
	}
	explicit operator Object *(){
		return GetPtr();
	}
	const Object *operator->() const {
		return GetSafePtr();
	}
	Object *operator->(){
		return GetSafePtr();
	}
	const Object &operator*() const{
		return Get();
	}
	Object &operator*(){
		return Get();
	}
};

}

#endif
