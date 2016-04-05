// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_LOCAL_HPP_
#define MCF_THREAD_THREAD_LOCAL_HPP_

#include "../../MCFCRT/env/thread.h"
#include "../../MCFCRT/env/last_error.h"
#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

namespace Impl_ThreadLocal {
	struct TlsKeyDeleter {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *pKey) const noexcept {
			::_MCFCRT_TlsFreeKey(pKey);
		}
	};
}

template<class ElementT>
class ThreadLocal {
	static_assert(std::is_nothrow_default_constructible<ElementT>::value, "The default constructor of ElementT shall not throw exceptions.");
	static_assert(alignof(ElementT) <= alignof(std::max_align_t),         "ElementT is over-aligned.");

private:
	UniqueHandle<Impl_ThreadLocal::TlsKeyDeleter> x_pTlsKey;

public:
	explicit ThreadLocal(){
		if(!x_pTlsKey.Reset(::_MCFCRT_TlsAllocKey(sizeof(ElementT),
			[](std::intptr_t, void *p){ Construct(static_cast<ElementT *>(p)); return static_cast<unsigned long>(0); },
			[](std::intptr_t, void *p){ Destruct (static_cast<ElementT *>(p)); },
			0)))
		{
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_TlsAllocKey() 失败。"));
		}
	}

public:
	const ElementT *Get() const noexcept {
		void *pStorage;
		const bool bResult = ::_MCFCRT_TlsGet(x_pTlsKey.Get(), &pStorage);
		MCF_ASSERT_MSG(bResult, L"_MCFCRT_TlsGet() 失败。");
		return static_cast<const ElementT *>(pStorage);
	}
	ElementT *Get() noexcept {
		void *pStorage;
		const bool bResult = ::_MCFCRT_TlsGet(x_pTlsKey.Get(), &pStorage);
		MCF_ASSERT_MSG(bResult, L"_MCFCRT_TlsGet() 失败。");
		return static_cast<ElementT *>(pStorage);
	}

	const ElementT *Open() const {
		void *pStorage;
		const bool bResult = ::_MCFCRT_TlsRequire(x_pTlsKey.Get(), &pStorage);
		if(!bResult){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_TlsRequire() 失败。"));
		}
		return static_cast<const ElementT *>(pStorage);
	}
	ElementT *Open(){
		void *pStorage;
		const bool bResult = ::_MCFCRT_TlsRequire(x_pTlsKey.Get(), &pStorage);
		if(!bResult){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_TlsRequire() 失败。"));
		}
		return static_cast<ElementT *>(pStorage);
	}

	template<typename ParamT>
	void Set(ParamT &&vParam){
		Open()[0] = std::forward<ParamT>(vParam);
	}

	void Swap(ThreadLocal &rhs) noexcept {
		using std::swap;
		swap(x_pTlsKey, rhs.x_pTlsKey);
	}

public:
	friend void swap(ThreadLocal &lhs, ThreadLocal &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
