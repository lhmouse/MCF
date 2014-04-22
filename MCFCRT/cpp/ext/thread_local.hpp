// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_THREAD_LOCAL_HPP_
#define MCF_CRT_THREAD_LOCAL_HPP_

#include "../../env/thread.h"
#include "../../env/bail.h"
#include <cstddef>
#include <new>
#include <tuple>
#include <type_traits>

namespace MCF {

template<class Object_t, class... InitParams_t>
class TheadLocal {
private:
	template<class... Unpacked_t>
	static void TupleConstruct(
		typename std::enable_if<sizeof...(Unpacked_t) < sizeof...(InitParams_t), void>::type *pObj,
		const std::tuple<InitParams_t...> &vTuple,
		const Unpacked_t &...vUnpacked
	){
		TupleConstruct(pObj, vTuple, vUnpacked..., std::get<sizeof...(Unpacked_t)>(vTuple));
	}
	template<class... Unpacked_t>
	static void TupleConstruct(
		typename std::enable_if<sizeof...(Unpacked_t) == sizeof...(InitParams_t), void>::type *pObj,
		const std::tuple<InitParams_t...> &,
		const Unpacked_t &...vUnpacked
	){
		new(pObj) Object_t(vUnpacked...);
	}

	static void xCtorWrapper(void *pObj, std::intptr_t nParam){
		TupleConstruct(pObj, ((const TheadLocal *)nParam)->xm_vInitParams);
	}
	static void xDtorWrapper(void *pObj) noexcept {
		((Object_t *)pObj)->~Object_t();
	}

private:
	std::tuple<InitParams_t...> xm_vInitParams;

public:
	explicit constexpr TheadLocal(InitParams_t &&... vInitParams)
		: xm_vInitParams(std::forward<InitParams_t>(vInitParams)...)
	{
	}
	~TheadLocal(){
		Release();
	}

private:
	Object_t *xGetPtr() const noexcept {
		const auto pRet = (Object_t *)::MCF_CRT_RetrieveTls((std::intptr_t)this, sizeof(Object_t), &xCtorWrapper, (std::intptr_t)this, &xDtorWrapper);
		if(!pRet){
			::MCF_CRT_Bail(
				L"MCF_CRT_RetrieveTls() 返回了一个空指针。\n"
				"如果这不是由于系统内存不足造成的，请确保不要在静态对象的构造函数或析构函数中访问 TLS。"
			);
		}
		return pRet;
	}
public:
	const Object_t &Get() const noexcept {
		return *xGetPtr();
	}
	Object_t &Get() noexcept {
		return *xGetPtr();
	}
	void Release() noexcept {
		::MCF_CRT_DeleteTls((std::intptr_t)this);
	}

public:
	explicit operator const Object_t *() const noexcept {
		return xGetPtr();
	}
	explicit operator Object_t *() noexcept {
		return xGetPtr();
	}
	const Object_t *operator->() const noexcept {
		return xGetPtr();
	}
	Object_t *operator->() noexcept {
		return xGetPtr();
	}
	const Object_t &operator*() const noexcept {
		return Get();
	}
	Object_t &operator*() noexcept {
		return Get();
	}
};

template<class Object_t, class... InitParams_t>
TheadLocal<Object_t, typename std::remove_reference<InitParams_t>::type...> MakeThreadLocal(InitParams_t &&... vInitParams){
	return TheadLocal<Object_t, typename std::remove_reference<InitParams_t>::type...>(std::forward<InitParams_t>(vInitParams)...);
}

}

#endif
