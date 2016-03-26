// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_LOCAL_HPP_
#define MCF_THREAD_THREAD_LOCAL_HPP_

#include "../../MCFCRT/env/thread.h"
#include "../../MCFCRT/env/last_error.h"
#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
#include <type_traits>
#include <cstring>
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

	template<class ElementT, bool kCanBeStoredAsIntPtrT>
	struct ElementManipulator {
		static ::_MCFCRT_TlsCallback GetCleanupCallback() noexcept {
			return [](std::intptr_t *pnValue){ delete ExtractValue(*pnValue); };
		}
		static ElementT *ExtractValue(std::intptr_t &nValue){
			return reinterpret_cast<ElementT *>(nValue);
		}
		static std::intptr_t PackValue(ElementT &&vSrc){
			return reinterpret_cast<std::intptr_t>(new ElementT(std::move(vSrc)));
		}
	};
	template<class ElementT>
	struct ElementManipulator<ElementT, true> {
		static constexpr ::_MCFCRT_TlsCallback GetCleanupCallback() noexcept {
			return nullptr;
		}
		static ElementT *ExtractValue(std::intptr_t &nValue){
			return reinterpret_cast<ElementT *>(&nValue);
		}
		static std::intptr_t PackValue(ElementT &&vSrc){
			std::intptr_t nRet;
			std::memcpy(&nRet, AddressOf(vSrc), sizeof(vSrc));
			return nRet;
		}
	};

	template<class ElementT>
	using ElementTraits = ElementManipulator<ElementT,
		std::is_trivial<ElementT>::value && !std::is_array<ElementT>::value && (sizeof(ElementT) <= sizeof(std::intptr_t))>;
}

template<class ElementT>
class ThreadLocal : MCF_NONCOPYABLE {
private:
	UniqueHandle<Impl_ThreadLocal::TlsKeyDeleter> x_pTlsKey;

public:
	explicit ThreadLocal(){
		const auto pfnCleanupCallback = Impl_ThreadLocal::ElementTraits<ElementT>::GetCleanupCallback();
		if(!x_pTlsKey.Reset(::_MCFCRT_TlsAllocKey(pfnCleanupCallback))){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_TlsAllocKey() 失败。"));
		}
	}

public:
	ElementT *Get() const noexcept {
		std::intptr_t *pnValue;
		if(!::_MCFCRT_TlsGet(x_pTlsKey.Get(), &pnValue)){
			MCF_ASSERT_MSG(false, L"_MCFCRT_TlsGet() 失败。");
		}
		return Impl_ThreadLocal::ElementTraits<ElementT>::ExtractValue(*pnValue);
	}
	void Set(ElementT vElement){
		std::intptr_t *pnValue;
		if(!::_MCFCRT_TlsRequire(x_pTlsKey.Get(), &pnValue, reinterpret_cast<std::intptr_t>(static_cast<ElementT *>(nullptr)))){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_TlsRequire() 失败。"));
		}
		const auto nNewValue = Impl_ThreadLocal::ElementTraits<ElementT>::PackValue(std::move(vElement));
		const auto pfnCleanupCallback = Impl_ThreadLocal::ElementTraits<ElementT>::GetCleanupCallback();
		if(pfnCleanupCallback){
			(*pfnCleanupCallback)(pnValue);
		}
		*pnValue = nNewValue;
	}
};

}

#endif
