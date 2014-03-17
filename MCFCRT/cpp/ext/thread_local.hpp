// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_THREAD_LOCAL_HPP__
#define __MCF_CRT_THREAD_LOCAL_HPP__

#include "../../env/thread.h"
#include "../../env/bail.h"
#include <cstddef>
#include <new>
#include <tuple>
#include <type_traits>

namespace MCF {

template<class Object_t>
class TheadLocal {
private:
	static void xCtorWrapper(void *pObj, std::intptr_t nParam){
		new(pObj) Object_t(((const TheadLocal *)nParam)->xm_vTemplate);
	}
	static void xDtorWrapper(void *pObj) noexcept {
		((Object_t *)pObj)->~Object_t();
	}

private:
	Object_t xm_vTemplate;

public:
	template<class... Params_t>
	constexpr TheadLocal(Params_t ...vParams) noexcept(noexcept(Object_t(std::move(vParams)...)))
		: xm_vTemplate(std::move(vParams)...)
	{
	}
	~TheadLocal(){
		Release();
	}

public:
	Object_t *Get() const noexcept {
		const auto pRet = (Object_t *)::__MCF_CRT_RetrieveTls((std::intptr_t)this, sizeof(Object_t), &xCtorWrapper, (std::intptr_t)this, &xDtorWrapper);
		if(!pRet){
			::__MCF_CRT_Bail(
				L"__MCF_CRT_RetrieveTls() 返回了一个空指针。\n"
				"如果这不是由于系统内存不足造成的，请确保不要在静态对象的构造函数或析构函数中访问 TLS。"
			);
		}
		return pRet;
	}
	void Release() const noexcept {
		::__MCF_CRT_DeleteTls((std::intptr_t)this);
	}

public:
	Object_t *operator->() const {
		return Get();
	}
	operator Object_t *() const {
		return Get();
	}
};

}

#endif
