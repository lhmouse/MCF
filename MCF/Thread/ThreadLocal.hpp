// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_LOCAL_HPP_
#define MCF_THREAD_THREAD_LOCAL_HPP_

#include "../../MCFCRT/env/thread_env.h"
#include "../../MCFCRT/env/last_error.h"
#include "../Utilities/Assert.hpp"
#include "../Utilities/AlignedStorage.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/ReconstructOrAssign.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

namespace Impl_ThreadLocal {
}

template<typename ElementT>
class ThreadLocal {
	static_assert(alignof(ElementT) <= alignof(std::max_align_t), "ElementT is over-aligned.");

private:
	struct X_TlsKeyDeleter {
		constexpr ::_MCFCRT_TlsKeyHandle operator()() const noexcept {
			return nullptr;
		}
		void operator()(::_MCFCRT_TlsKeyHandle hTlsKey) const noexcept {
			::_MCFCRT_TlsFreeKey(hTlsKey);
		}
	};

	struct X_TlsContainer {
		AlignedStorage<0, ElementT> vStorage;
		bool bConstructed;
	};

private:
	static UniqueHandle<X_TlsKeyDeleter> X_AllocateTlsKey(){
		auto hTemp = ::_MCFCRT_TlsAllocKey(sizeof(X_TlsContainer),
			[](std::intptr_t, void *pStorage) noexcept -> unsigned long {
				const auto pContainer = static_cast<X_TlsContainer *>(pStorage);
				pContainer->bConstructed = false;
				return 0;
			},
			[](std::intptr_t, void *pStorage) noexcept -> void {
				const auto pContainer = static_cast<X_TlsContainer *>(pStorage);
				if(!pContainer->bConstructed){
					return;
				}
				Destruct(reinterpret_cast<ElementT *>(&(pContainer->vStorage)));
			},
			0);
		if(!hTemp){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_TlsAllocKey() 失败。"));
		}
		UniqueHandle<X_TlsKeyDeleter> hTlsKey(hTemp);

		return hTlsKey;
	}

private:
	UniqueHandle<X_TlsKeyDeleter> x_hTlsKey;

public:
	explicit ThreadLocal()
		: x_hTlsKey(X_AllocateTlsKey())
	{
	}

public:
	const ElementT *Get() const noexcept {
		void *pStorage;
		const bool bResult = ::_MCFCRT_TlsGet(x_hTlsKey.Get(), &pStorage);
		MCF_ASSERT_MSG(bResult, L"_MCFCRT_TlsGet() 失败。");
		const auto pContainer = static_cast<const X_TlsContainer *>(pStorage);
		if(!pContainer){
			return nullptr;
		}

		if(!pContainer->bConstructed){
			return nullptr;
		}
		const auto pElement = reinterpret_cast<const ElementT *>(&(pContainer->vStorage));
		return pElement;
	}
	ElementT *Get() noexcept {
		void *pStorage;
		const bool bResult = ::_MCFCRT_TlsGet(x_hTlsKey.Get(), &pStorage);
		MCF_ASSERT_MSG(bResult, L"_MCFCRT_TlsGet() 失败。");
		const auto pContainer = static_cast<X_TlsContainer *>(pStorage);
		if(!pContainer){
			return nullptr;
		}

		if(!pContainer->bConstructed){
			return nullptr;
		}
		const auto pElement = reinterpret_cast<ElementT *>(&(pContainer->vStorage));
		return pElement;
	}

	const ElementT *Open() const {
		void *pStorage;
		const bool bResult = ::_MCFCRT_TlsRequire(x_hTlsKey.Get(), &pStorage);
		if(!bResult){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_TlsRequire() 失败。"));
		}
		const auto pContainer = static_cast<X_TlsContainer *>(pStorage);
		MCF_ASSERT(pContainer);

		const auto pElement = reinterpret_cast<ElementT *>(&(pContainer->vStorage));
		if(!pContainer->bConstructed){
			Construct(pElement);
			pContainer->bConstructed = true;
		}
		return pElement;
	}
	ElementT *Open(){
		void *pStorage;
		const bool bResult = ::_MCFCRT_TlsRequire(x_hTlsKey.Get(), &pStorage);
		if(!bResult){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_TlsRequire() 失败。"));
		}
		const auto pContainer = static_cast<X_TlsContainer *>(pStorage);
		MCF_ASSERT(pContainer);

		const auto pElement = reinterpret_cast<ElementT *>(&(pContainer->vStorage));
		if(!pContainer->bConstructed){
			Construct(pElement);
			pContainer->bConstructed = true;
		}
		return pElement;
	}

	template<typename ...ParamsT>
	void Set(ParamsT &&...vParams){
		void *pStorage;
		const bool bResult = ::_MCFCRT_TlsRequire(x_hTlsKey.Get(), &pStorage);
		if(!bResult){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_TlsRequire() 失败。"));
		}
		const auto pContainer = static_cast<X_TlsContainer *>(pStorage);
		MCF_ASSERT(pContainer);

		const auto pElement = reinterpret_cast<ElementT *>(&(pContainer->vStorage));
		if(!pContainer->bConstructed){
			Construct(pElement, std::forward<ParamsT>(vParams)...);
			pContainer->bConstructed = true;
		} else {
			ReconstructOrAssign(pElement, std::forward<ParamsT>(vParams)...);
		}
	}

	void Swap(ThreadLocal &rhs) noexcept {
		using std::swap;
		swap(x_hTlsKey, rhs.x_hTlsKey);
	}

public:
	friend void swap(ThreadLocal &lhs, ThreadLocal &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
