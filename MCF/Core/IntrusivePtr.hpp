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

namespace MCF {

template<typename T, class DeleterT = DefaultDeleter<std::remove_cv_t<T>>>
class IntrusivePtr;

namespace Impl {
	class IntrusiveSentry {
	private:
		void (*xm_pfnCallback)(void *);
		void *xm_pObject;

	public:
		explicit IntrusiveSentry(void (*pfnCallback)(void *), void *pObject) noexcept
			: xm_pfnCallback(pfnCallback), xm_pObject(pObject)
		{
		}
		IntrusiveSentry(IntrusiveSentry &&rhs) noexcept
			: xm_pfnCallback(std::exchange(rhs.xm_pfnCallback, nullptr)), xm_pObject(rhs.xm_pObject)
		{
		}
		~IntrusiveSentry(){
			if(xm_pfnCallback){
				(*xm_pfnCallback)(xm_pObject);
			}
		}

		IntrusiveSentry(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(IntrusiveSentry &&) = delete;
	};

	template<typename T, class DeleterT>
	struct IntrusiveBase {
	private:
		mutable volatile std::size_t xm_uRefCount;

	public:
		IntrusiveBase() noexcept {
			__atomic_store_n(&xm_uRefCount, 1, __ATOMIC_RELAXED);
		}

		IntrusiveBase(const IntrusiveBase &) = delete;
		void operator=(const IntrusiveBase &) = delete;

	public:
		void AddRef() const volatile noexcept {
			ASSERT(__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) != 0);

			__atomic_add_fetch(&xm_uRefCount, 1, __ATOMIC_RELEASE);
		}
		Impl::IntrusiveSentry DropRef() const volatile noexcept {
			ASSERT(__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) != 0);

			if(__atomic_sub_fetch(&xm_uRefCount, 1, __ATOMIC_ACQUIRE) != 0){
				return Impl::IntrusiveSentry(nullptr, nullptr);
			}
			return Impl::IntrusiveSentry(
				[](void *pThis){ DeleterT()(static_cast<T *>(pThis)); }, const_cast<T *>(Get()));
		}

		auto Get() const volatile noexcept {
			return static_cast<const volatile T *>(this);
		}
		auto Get() const noexcept {
			return static_cast<const T *>(this);
		}
		auto Get() volatile noexcept {
			return static_cast<volatile T *>(this);
		}
		auto Get() noexcept {
			return static_cast<T *>(this);
		}

		IntrusivePtr<const volatile T, DeleterT> Fork() const volatile noexcept;
		IntrusivePtr<const T, DeleterT> Fork() const noexcept;
		IntrusivePtr<volatile T, DeleterT> Fork() volatile noexcept;
		IntrusivePtr<T, DeleterT> Fork() noexcept;
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
		if(xm_pBuddy){
			xm_pBuddy->DropRef();
		}
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

	void Reset(T *pObject = nullptr) noexcept {
		const auto pOld = std::exchange(xm_pBuddy, pObject);
		if(pOld){
			pOld->DropRef();
		}
	}
	template<typename U, std::enable_if_t<std::is_convertible<U *, T *>::value, int> = 0>
	void Reset(const IntrusivePtr<U, DeleterT> &rhs) noexcept {
		Reset(rhs.Get());
		if(xm_pBuddy){
			xm_pBuddy->AddRef();
		}
	}
	template<typename U, std::enable_if_t<std::is_convertible<U *, T *>::value, int> = 0>
	void Reset(IntrusivePtr<U, DeleterT> &&rhs) noexcept {
		Reset(rhs.Release());
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
		const auto pRet = Get();
		ASSERT_MSG(pRet, L"试图解引用空指针。");
		return *pRet;
	}
	T *operator->() const noexcept {
		const auto pRet = Get();
		ASSERT_MSG(pRet, L"试图解引用空指针。");
		return pRet;
	}
};

namespace Impl {
	template<typename T, class DeleterT>
	IntrusivePtr<const volatile T, DeleterT> IntrusiveBase<T, DeleterT>::Fork() const volatile noexcept {
		AddRef();
		return IntrusivePtr<const volatile T, DeleterT>(Get());
	}
	template<typename T, class DeleterT>
	IntrusivePtr<const T, DeleterT> IntrusiveBase<T, DeleterT>::Fork() const noexcept {
		AddRef();
		return IntrusivePtr<const T, DeleterT>(Get());
	}
	template<typename T, class DeleterT>
	IntrusivePtr<volatile T, DeleterT> IntrusiveBase<T, DeleterT>::Fork() volatile noexcept {
		AddRef();
		return IntrusivePtr<volatile T, DeleterT>(Get());
	}
	template<typename T, class DeleterT>
	IntrusivePtr<T, DeleterT> IntrusiveBase<T, DeleterT>::Fork() noexcept {
		AddRef();
		return IntrusivePtr<T, DeleterT>(Get());
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
