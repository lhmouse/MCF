// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_INTRUSIVE_PTR_HPP_
#define MCF_CORE_INTRUSIVE_PTR_HPP_

#include "DefaultDeleter.hpp"
#include "../Utilities/Assert.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

template<typename T, class DeleterT = DefaultDeleter<std::remove_cv_t<T>>>
class IntrusivePtr;

namespace Impl {
	class IntrusiveSentry {
	private:
		void (*xm_pfnCallback)(std::intptr_t);
		std::intptr_t xm_nContext;

	public:
		explicit IntrusiveSentry(void (*pfnCallback)(std::intptr_t), std::intptr_t nContext) noexcept
			: xm_pfnCallback(pfnCallback), xm_nContext(nContext)
		{
		}
		IntrusiveSentry(IntrusiveSentry &&rhs) noexcept
			: xm_pfnCallback(std::exchange(rhs.xm_pfnCallback, nullptr)), xm_nContext(rhs.xm_nContext)
		{
		}
		~IntrusiveSentry(){
			if(xm_pfnCallback){
				(*xm_pfnCallback)(xm_nContext);
			}
		}

		IntrusiveSentry(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(IntrusiveSentry &&) = delete;
	};

	template<typename DstT, typename SrcT, typename = void>
	struct IntrusiveCastHelper {
		DstT *operator()(SrcT *pSrc) const noexcept {
			return dynamic_cast<DstT *>(pSrc);
		}
	};
	template<typename DstT, typename SrcT>
	struct IntrusiveCastHelper<DstT, SrcT,
		decltype(static_cast<DstT *>(std::declval<SrcT *>()), (void)0)>
	{
		constexpr DstT *operator()(SrcT *pSrc) const noexcept {
			return static_cast<DstT *>(pSrc);
		}
	};

	template<typename T, class DeleterT>
	class IntrusiveBase {
	private:
		mutable volatile std::size_t xm_uRefCount;

	protected:
		IntrusiveBase() noexcept {
			__atomic_store_n(&xm_uRefCount, 1, __ATOMIC_RELAXED);
		}

	public:
		IntrusiveBase(const IntrusiveBase &) noexcept
			: IntrusiveBase() // 默认构造。
		{
		}
		IntrusiveBase(IntrusiveBase &&) noexcept
			: IntrusiveBase() // 同上。
		{
		}
		IntrusiveBase &operator=(const IntrusiveBase &) noexcept {
			return *this; // 无操作。
		}
		IntrusiveBase &operator=(IntrusiveBase &&) noexcept {
			return *this; // 同上。
		}

	public:
		void AddRef() const volatile noexcept {
			ASSERT(__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) != 0);

			__atomic_add_fetch(&xm_uRefCount, 1, __ATOMIC_RELEASE);
		}
		IntrusiveSentry DropRef() const volatile noexcept {
			ASSERT(__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) != 0);

			if(__atomic_sub_fetch(&xm_uRefCount, 1, __ATOMIC_ACQUIRE) != 0){
				return IntrusiveSentry(nullptr, 0);
			}
			return IntrusiveSentry(
				[](std::intptr_t nThis){ DeleterT()(reinterpret_cast<T *>(nThis)); },
				reinterpret_cast<std::intptr_t>(Get()));
		}

		template<typename U = T>
		auto Get() const volatile noexcept {
			return IntrusiveCastHelper<const volatile U, const volatile IntrusiveBase>()(this);
		}
		template<typename U = T>
		auto Get() const noexcept {
			return IntrusiveCastHelper<const U, const IntrusiveBase>()(this);
		}
		template<typename U = T>
		auto Get() volatile noexcept {
			return IntrusiveCastHelper<volatile U, volatile IntrusiveBase>()(this);
		}
		template<typename U = T>
		auto Get() noexcept {
			return IntrusiveCastHelper<U, IntrusiveBase>()(this);
		}

		template<typename U = T>
		IntrusivePtr<const volatile U, DeleterT> Fork() const volatile noexcept;
		template<typename U = T>
		IntrusivePtr<const U, DeleterT> Fork() const noexcept;
		template<typename U = T>
		IntrusivePtr<volatile U, DeleterT> Fork() volatile noexcept;
		template<typename U = T>
		IntrusivePtr<U, DeleterT> Fork() noexcept;
	};
}

template<typename T, class DeleterT = DefaultDeleter<std::remove_cv_t<T>>>
using IntrusiveBase = Impl::IntrusiveBase<std::remove_cv_t<T>, DeleterT>;

template<typename T, class DeleterT>
class IntrusivePtr {
public:
	using Buddy = IntrusiveBase<T, DeleterT>;

private:
	const volatile Buddy *xm_pBuddy;

public:
	constexpr explicit IntrusivePtr(T *pObject = nullptr) noexcept
		: xm_pBuddy(pObject)
	{
	}
	IntrusivePtr(const IntrusivePtr &rhs) noexcept
		: IntrusivePtr()
	{
		Reset(rhs);
	}
	IntrusivePtr(IntrusivePtr &&rhs) noexcept
		: IntrusivePtr()
	{
		Reset(std::move(rhs));
	}
	template<typename U, std::enable_if_t<std::is_convertible<U *, T *>::value, int> = 0>
	explicit IntrusivePtr(const IntrusivePtr<U, DeleterT> &rhs) noexcept
		: IntrusivePtr()
	{
		Reset(rhs);
	}
	template<typename U, std::enable_if_t<std::is_convertible<U *, T *>::value, int> = 0>
	explicit IntrusivePtr(IntrusivePtr<U, DeleterT> &&rhs) noexcept
		: IntrusivePtr()
	{
		Reset(std::move(rhs));
	}
	IntrusivePtr &operator=(const IntrusivePtr &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	IntrusivePtr &operator=(IntrusivePtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	template<typename U, std::enable_if_t<std::is_convertible<U *, T *>::value, int> = 0>
	IntrusivePtr &operator=(const IntrusivePtr<U, DeleterT> &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	template<typename U, std::enable_if_t<std::is_convertible<U *, T *>::value, int> = 0>
	IntrusivePtr &operator=(IntrusivePtr<U, DeleterT> &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~IntrusivePtr(){
		Reset();
	}

public:
	bool IsGood() const noexcept {
		return Get() != nullptr;
	}
	T *Get() const noexcept {
		if(!xm_pBuddy){
			return nullptr;
		}
		return const_cast<T *>(xm_pBuddy->Get());
	}
	T *Release() noexcept {
		const auto pBuddy = std::exchange(xm_pBuddy, nullptr);
		if(!pBuddy){
			return nullptr;
		}
		return const_cast<T *>(pBuddy->Get());
	}
	IntrusivePtr Fork() const noexcept {
		return IntrusivePtr(*this);
	}

	IntrusivePtr &Reset(T *pObject = nullptr) noexcept {
		const auto pOld = std::exchange(xm_pBuddy, pObject);
		if(pOld){
			pOld->DropRef();
		}
		return *this;
	}
	template<typename U, std::enable_if_t<std::is_convertible<U *, T *>::value, int> = 0>
	IntrusivePtr &Reset(const IntrusivePtr<U, DeleterT> &rhs) noexcept {
		Reset(rhs.Get());
		if(xm_pBuddy){
			xm_pBuddy->AddRef();
		}
		return *this;
	}
	template<typename U, std::enable_if_t<std::is_convertible<U *, T *>::value, int> = 0>
	IntrusivePtr &Reset(IntrusivePtr<U, DeleterT> &&rhs) noexcept {
		return Reset(rhs.Release());
	}

	void Swap(IntrusivePtr &rhs) noexcept {
		std::swap(xm_pBuddy, rhs.xm_pBuddy);
	}

public:
	explicit operator bool() const noexcept {
		return IsGood();
	}
	explicit operator T *() const noexcept {
		return Get();
	}

	T &operator*() const noexcept {
		return *(this->operator->());
	}
	T *operator->() const noexcept {
		const auto pRet = Get();
		ASSERT_MSG(pRet, L"试图解引用空指针。");
		return pRet;
	}
};

namespace Impl {
	template<typename T, class DeleterT>
		template<typename U>
	IntrusivePtr<const volatile U, DeleterT> IntrusiveBase<T, DeleterT>::Fork() const volatile noexcept {
		const auto pForked = Get<const volatile U>();
		if(!pForked){
			return nullptr;
		}
		AddRef();
		return IntrusivePtr<const volatile U, DeleterT>(pForked);
	}
	template<typename T, class DeleterT>
		template<typename U>
	IntrusivePtr<const U, DeleterT> IntrusiveBase<T, DeleterT>::Fork() const noexcept {
		const auto pForked = Get<const U>();
		if(!pForked){
			return nullptr;
		}
		AddRef();
		return IntrusivePtr<const U, DeleterT>(pForked);
	}
	template<typename T, class DeleterT>
		template<typename U>
	IntrusivePtr<volatile U, DeleterT> IntrusiveBase<T, DeleterT>::Fork() volatile noexcept {
		const auto pForked = Get<volatile U>();
		if(!pForked){
			return nullptr;
		}
		AddRef();
		return IntrusivePtr<const volatile U, DeleterT>(pForked);
	}
	template<typename T, class DeleterT>
		template<typename U>
	IntrusivePtr<U, DeleterT> IntrusiveBase<T, DeleterT>::Fork() noexcept {
		const auto pForked = Get<U>();
		if(!pForked){
			return nullptr;
		}
		AddRef();
		return IntrusivePtr<U, DeleterT>(pForked);
	}
}

template<typename T, class DeleterT>
bool operator==(const IntrusivePtr<T, DeleterT> &lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs.Get() == rhs.Get();
}
template<typename T, class DeleterT>
bool operator==(const IntrusivePtr<T, DeleterT> &lhs, T *rhs) noexcept {
	return lhs.Get() == rhs;
}
template<typename T, class DeleterT>
bool operator==(T *lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs == rhs.Get();
}

template<typename T, class DeleterT>
bool operator!=(const IntrusivePtr<T, DeleterT> &lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs.Get() != rhs.Get();
}
template<typename T, class DeleterT>
bool operator!=(const IntrusivePtr<T, DeleterT> &lhs, T *rhs) noexcept {
	return lhs.Get() != rhs;
}
template<typename T, class DeleterT>
bool operator!=(T *lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs != rhs.Get();
}

template<typename T, class DeleterT>
bool operator<(const IntrusivePtr<T, DeleterT> &lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs.Get() < rhs.Get();
}
template<typename T, class DeleterT>
bool operator<(const IntrusivePtr<T, DeleterT> &lhs, T *rhs) noexcept {
	return lhs.Get() < rhs;
}
template<typename T, class DeleterT>
bool operator<(T *lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs < rhs.Get();
}

template<typename T, class DeleterT>
bool operator>(const IntrusivePtr<T, DeleterT> &lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs.Get() > rhs.Get();
}
template<typename T, class DeleterT>
bool operator>(const IntrusivePtr<T, DeleterT> &lhs, T *rhs) noexcept {
	return lhs.Get() > rhs;
}
template<typename T, class DeleterT>
bool operator>(T *lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs > rhs.Get();
}

template<typename T, class DeleterT>
bool operator<=(const IntrusivePtr<T, DeleterT> &lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs.Get() <= rhs.Get();
}
template<typename T, class DeleterT>
bool operator<=(const IntrusivePtr<T, DeleterT> &lhs, T *rhs) noexcept {
	return lhs.Get() <= rhs;
}
template<typename T, class DeleterT>
bool operator<=(T *lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs <= rhs.Get();
}

template<typename T, class DeleterT>
bool operator>=(const IntrusivePtr<T, DeleterT> &lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs.Get() >= rhs.Get();
}
template<typename T, class DeleterT>
bool operator>=(const IntrusivePtr<T, DeleterT> &lhs, T *rhs) noexcept {
	return lhs.Get() >= rhs;
}
template<typename T, class DeleterT>
bool operator>=(T *lhs, const IntrusivePtr<T, DeleterT> &rhs) noexcept {
	return lhs >= rhs.Get();
}

template<typename T, class DeleterT>
void swap(IntrusivePtr<T, DeleterT> &lhs, IntrusivePtr<T, DeleterT> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
