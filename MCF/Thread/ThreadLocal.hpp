// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_LOCAL_HPP_
#define MCF_THREAD_THREAD_LOCAL_HPP_

#include "../../MCFCRT/env/thread.h"
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
			::MCF_CRT_TlsFreeKey(pKey);
		}
	};

	template<class ObjectT>
	class ThreadLocalTrivialSmallEnough : NONCOPYABLE {
		static_assert(std::is_trivial<ObjectT>::value && (sizeof(ObjectT) <= sizeof(std::intptr_t)), "!");

	private:
		const ObjectT x_vDefault;
		UniqueHandle<TlsKeyDeleter> x_pTlsKey;

	public:
		explicit ThreadLocalTrivialSmallEnough(ObjectT vDefault = ObjectT())
			: x_vDefault(std::move(vDefault))
		{
			if(!x_pTlsKey.Reset(::MCF_CRT_TlsAllocKey(nullptr))){
				DEBUG_THROW(SystemError, "MCF_CRT_TlsAllocKey");
			}
		}

	public:
		ObjectT Get() const noexcept {
			bool bHasValue;
			std::intptr_t nValue;
			if(!::MCF_CRT_TlsGet(x_pTlsKey.Get(), &bHasValue, &nValue)){
				ASSERT_MSG(false, L"MCF_CRT_TlsGet() 失败。");
			}
			if(bHasValue){
				return reinterpret_cast<const ObjectT &>(nValue);
			}
			return x_vDefault;
		}
		void Set(ObjectT vObject){
			if(!::MCF_CRT_TlsReset(x_pTlsKey.Get(), reinterpret_cast<std::intptr_t &>(vObject))){ // noexcept
				DEBUG_THROW(SystemError, "MCF_CRT_TlsReset");
			}
		}
	};

	template<class ObjectT>
	class ThreadLocalGeneric : NONCOPYABLE {
	private:
		const ObjectT x_vDefault;
		UniqueHandle<TlsKeyDeleter> x_pTlsKey;

	public:
		explicit ThreadLocalGeneric(ObjectT vDefault = ObjectT())
			: x_vDefault(std::move(vDefault))
		{
			if(!x_pTlsKey.Reset(::MCF_CRT_TlsAllocKey([](std::intptr_t nValue){ delete reinterpret_cast<ObjectT *>(nValue); }))){
				DEBUG_THROW(SystemError, "MCF_CRT_TlsAllocKey");
			}
		}

	public:
		const ObjectT &Get() const noexcept {
			bool bHasValue;
			std::intptr_t nValue;
			if(!::MCF_CRT_TlsGet(x_pTlsKey.Get(), &bHasValue, &nValue)){
				ASSERT_MSG(false, L"MCF_CRT_TlsGet() 失败。");
			}
			if(bHasValue){
				const auto pObject = reinterpret_cast<const ObjectT *>(nValue);
				if(pObject){
					return *pObject;
				}
			}
			return x_vDefault;
		}
		void Set(ObjectT vObject){
			const auto pObject = new ObjectT(vObject);
			if(!::MCF_CRT_TlsReset(x_pTlsKey.Get(), reinterpret_cast<std::intptr_t>(pObject))){ // noexcept
				delete pObject;
				DEBUG_THROW(SystemError, "MCF_CRT_TlsReset");
			}
		}
	};
}

template<class ObjectT>
using ThreadLocal =
	std::conditional_t<std::is_trivial<ObjectT>::value && (sizeof(ObjectT) <= sizeof(std::intptr_t)),
		Impl_ThreadLocal::ThreadLocalTrivialSmallEnough<ObjectT>,
		Impl_ThreadLocal::ThreadLocalGeneric<ObjectT>
		>;

}

#endif
