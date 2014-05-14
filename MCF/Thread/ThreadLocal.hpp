// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_LOCAL_HPP_
#define MCF_THREAD_LOCAL_HPP_

#include "../../MCFCRT/env/thread.h"
#include "../../MCFCRT/env/bail.h"
#include "../Core/Utilities.hpp"
#include <tuple>
#include <type_traits>
#include <exception>
#include <cstddef>

namespace MCF {

template<class Object_t, class... InitParams_t>
class ThreadLocal {
	static_assert(std::is_nothrow_destructible<Object_t>::value, "Object_t must be nothrow destructible.");

private:
	template<typename Test_t = int>
	static std::nullptr_t xGetCurrentException(
		typename std::enable_if<
			std::is_nothrow_constructible<Object_t, InitParams_t...>::value,
			Test_t
		>::type = 0
	){
		return nullptr;
	}
	template<typename Test_t = int>
	static std::exception_ptr xGetCurrentException(
		typename std::enable_if<
			!std::is_nothrow_constructible<Object_t, InitParams_t...>::value,
			Test_t
		>::type = 0
	){
		return std::current_exception();
	}

	typedef decltype(xGetCurrentException(0)) xExceptionPtr;
	typedef std::pair<const ThreadLocal *, xExceptionPtr> xCtorWrapperContext;

private:
	template<class... Unpacked_t>
	static void xConstructFromTuple(
		typename std::enable_if<sizeof...(Unpacked_t) < sizeof...(InitParams_t), void>::type *pObj,
		const std::tuple<InitParams_t...> &vTuple,
		const Unpacked_t &...vUnpacked
	) noexcept(std::is_nothrow_constructible<Object_t, InitParams_t...>::value){
		xConstructFromTuple(pObj, vTuple, vUnpacked..., std::get<sizeof...(Unpacked_t)>(vTuple));
	}
	template<class... Unpacked_t>
	static void xConstructFromTuple(
		typename std::enable_if<sizeof...(Unpacked_t) == sizeof...(InitParams_t), void>::type *pObj,
		const std::tuple<InitParams_t...> &,
		const Unpacked_t &...vUnpacked
	) noexcept(std::is_nothrow_constructible<Object_t, InitParams_t...>::value){
		Construct<Object_t>(pObj, vUnpacked...);
	}

	static int xCtorWrapper(void *pObj, std::intptr_t nParam) noexcept {
		auto *const pContext = (xCtorWrapperContext *)nParam;
		try {
			xConstructFromTuple(pObj, pContext->first->xm_vInitParams);
			return -1;
		} catch(...){
			pContext->second = xGetCurrentException();
			return 0;
		}
	}
	static void xDtorWrapper(void *pObj) noexcept {
		Destruct((Object_t *)pObj);
	}

private:
	std::tuple<InitParams_t...> xm_vInitParams;

public:
	explicit constexpr ThreadLocal(InitParams_t &&... vInitParams)
		: xm_vInitParams(std::forward<InitParams_t>(vInitParams)...)
	{
	}
	~ThreadLocal() noexcept {
		Release();
	}

public:
	Object_t *GetPtr() const
		noexcept(std::is_nothrow_constructible<Object_t, InitParams_t...>::value)
	{
		xCtorWrapperContext vContext(this, xExceptionPtr());
		const auto pRet = (Object_t *)::MCF_CRT_RetrieveTls((std::intptr_t)this, sizeof(Object_t), &xCtorWrapper, (std::intptr_t)&vContext, &xDtorWrapper);
		if(vContext.second){
			std::rethrow_exception(vContext.second);
		}
		return pRet;
	}
	Object_t *GetSafePtr() const {
		const auto pRet = GetPtr();
		if(!pRet){
			// 如果这不是由于系统内存不足造成的，请确保不要在静态对象的构造函数或析构函数中访问 TLS。
			throw std::bad_alloc();
		}
		return pRet;
	}

	const Object_t &Get() const {
		return *GetSafePtr();
	}
	Object_t &Get(){
		return *GetSafePtr();
	}

	void Release() noexcept {
		::MCF_CRT_DeleteTls((std::intptr_t)this);
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
