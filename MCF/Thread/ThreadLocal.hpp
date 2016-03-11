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
#include <cstddef>
#include <cstdint>

namespace MCF {

namespace Impl_ThreadLocal {
	struct TlsKeyDeleter {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *pKey) const noexcept {
			::MCFCRT_TlsFreeKey(pKey);
		}
	};

	using TlsCleanupCallback = void (*)(std::intptr_t);

	template<class ElementT, bool kCanBeStoredAsIntPtr>
	struct ElementManipulator {
		static TlsCleanupCallback GetCleanupCallback() noexcept {
			return [](std::intptr_t nValue){ delete reinterpret_cast<ElementT *>(nValue); };
		}
		static bool ExtractValue(ElementT &vDst, std::intptr_t nValue){
			const auto pSrc = reinterpret_cast<const ElementT *>(nValue);
			if(!pSrc){
				return false;
			}
			vDst = *pSrc;
			return true;
		}
		static std::intptr_t PackValue(ElementT &&vSrc){
			return reinterpret_cast<std::intptr_t>(new ElementT(std::move(vSrc)));
		}
	};
	template<class ElementT>
	struct ElementManipulator<ElementT, true> {
		static constexpr TlsCleanupCallback GetCleanupCallback() noexcept {
			return nullptr;
		}
		static bool ExtractValue(ElementT &vDst, std::intptr_t nValue) noexcept {
			__builtin_memcpy(&vDst, &nValue, sizeof(vDst));
			return true;
		}
		static std::intptr_t PackValue(ElementT &&vSrc) noexcept {
			std::intptr_t nValue;
			__builtin_memcpy(&nValue, &vSrc, sizeof(vSrc));
			return nValue;
		}
	};

	template<class ElementT>
	using ElementTraits = ElementManipulator<ElementT,
		std::is_trivial<ElementT>::value && !std::is_array<ElementT>::value && (sizeof(ElementT) <= sizeof(std::intptr_t))>;
}

template<class ElementT>
class ThreadLocal : MCF_NONCOPYABLE {
private:
	const ElementT x_vDefault;
	UniqueHandle<Impl_ThreadLocal::TlsKeyDeleter> x_pTlsKey;

public:
	explicit ThreadLocal(ElementT vDefault = ElementT())
		: x_vDefault(std::move(vDefault))
	{
		const auto pfnCleanupCallback = Impl_ThreadLocal::ElementTraits<ElementT>::GetCleanupCallback();
		if(!x_pTlsKey.Reset(::MCFCRT_TlsAllocKey(pfnCleanupCallback))){
			MCF_THROW(Exception, ::MCFCRT_GetLastWin32Error(), Rcntws::View(L"MCFCRT_TlsAllocKey() 失败。"));
		}
	}

public:
	ElementT Get() const {
		bool bHasValue = false;
		std::intptr_t nValue = 0;
		if(!::MCFCRT_TlsGet(x_pTlsKey.Get(), &bHasValue, &nValue)){
			ASSERT_MSG(false, L"MCFCRT_TlsGet() 失败。");
		}
		if(bHasValue){
			ElementT vElement;
			if(Impl_ThreadLocal::ElementTraits<ElementT>::ExtractValue(vElement, nValue)){
				return std::move(vElement);
			}
		}
		return x_vDefault;
	}
	void Set(ElementT vElement){
		const auto pfnCleanupCallback = Impl_ThreadLocal::ElementTraits<ElementT>::GetCleanupCallback();
		std::intptr_t nValue = Impl_ThreadLocal::ElementTraits<ElementT>::PackValue(std::move(vElement));
		if(!::MCFCRT_TlsReset(x_pTlsKey.Get(), nValue)){ // noexcept
			if(pfnCleanupCallback){
				(*pfnCleanupCallback)(nValue);
			}
			MCF_THROW(Exception, ::MCFCRT_GetLastWin32Error(), Rcntws::View(L"MCFCRT_TlsReset() 失败。"));
		}
	}
};

}

#endif
