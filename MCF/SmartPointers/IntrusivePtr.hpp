// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_INTRUSIVE_PTR_HPP_
#define MCF_CORE_INTRUSIVE_PTR_HPP_

#include "../Utilities/Assert.hpp"
#include "DefaultDeleter.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class IntrusivePtr;

namespace Impl {
	template<typename ObjectT, class DeleterT>
	class IntrusiveSentry {
	private:
		ObjectT *xm_pToDelete;

	public:
		explicit IntrusiveSentry(ObjectT *pToDelete) noexcept
			: xm_pToDelete(pToDelete)
		{
		}
		IntrusiveSentry(IntrusiveSentry &&rhs) noexcept
			: xm_pToDelete(std::exchange(rhs.xm_pToDelete, nullptr))
		{
		}
		~IntrusiveSentry(){
			if(xm_pToDelete){
				DeleterT()(const_cast<std::remove_cv_t<ObjectT> *>(xm_pToDelete));
			}
		}

		IntrusiveSentry(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(IntrusiveSentry &&) = delete;
	};

	template<typename DstT, typename SrcT, typename = int>
	struct IntrusiveCastHelper {
		DstT *operator()(SrcT *pSrc) const noexcept {
			return dynamic_cast<DstT *>(pSrc);
		}
	};
	template<typename DstT, typename SrcT>
	struct IntrusiveCastHelper<DstT, SrcT,
		decltype(static_cast<DstT *>(std::declval<SrcT *>()), 1)>
	{
		constexpr DstT *operator()(SrcT *pSrc) const noexcept {
			return static_cast<DstT *>(pSrc);
		}
	};

	template<typename ObjectT, class DeleterT>
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
		IntrusiveSentry<ObjectT, DeleterT> DropRef() const volatile noexcept {
			ASSERT(__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) != 0);

			ObjectT *pToDelete = nullptr;
			if(__atomic_sub_fetch(&xm_uRefCount, 1, __ATOMIC_ACQUIRE) == 0){
				pToDelete = const_cast<ObjectT *>(Get());
			}
			return IntrusiveSentry<ObjectT, DeleterT>(pToDelete);
		}

		template<typename OtherT = ObjectT>
		const volatile OtherT *Get() const volatile noexcept {
			return IntrusiveCastHelper<const volatile OtherT, const volatile IntrusiveBase>()(this);
		}
		template<typename OtherT = ObjectT>
		const OtherT *Get() const noexcept {
			return IntrusiveCastHelper<const OtherT, const IntrusiveBase>()(this);
		}
		template<typename OtherT = ObjectT>
		volatile OtherT *Get() volatile noexcept {
			return IntrusiveCastHelper<volatile OtherT, volatile IntrusiveBase>()(this);
		}
		template<typename OtherT = ObjectT>
		OtherT *Get() noexcept {
			return IntrusiveCastHelper<OtherT, IntrusiveBase>()(this);
		}

		template<typename OtherT = ObjectT>
		IntrusivePtr<const volatile OtherT, DeleterT> Fork() const volatile noexcept;

		template<typename OtherT = ObjectT>
		IntrusivePtr<const OtherT, DeleterT> Fork() const noexcept;

		template<typename OtherT = ObjectT>
		IntrusivePtr<volatile OtherT, DeleterT> Fork() volatile noexcept;

		template<typename OtherT = ObjectT>
		IntrusivePtr<OtherT, DeleterT> Fork() noexcept;
	};
}

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
using IntrusiveBase = Impl::IntrusiveBase<std::remove_cv_t<ObjectT>, DeleterT>;

template<typename ObjectT, class DeleterT>
class IntrusivePtr {
public:
	using Buddy = IntrusiveBase<ObjectT, DeleterT>;

private:
	const volatile Buddy *xm_pBuddy;

public:
	constexpr explicit IntrusivePtr(ObjectT *pObject = nullptr) noexcept
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
	template<typename OtherT>
	explicit IntrusivePtr(const IntrusivePtr<OtherT, DeleterT> &rhs) noexcept
		: IntrusivePtr()
	{
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(rhs);
	}
	template<typename OtherT>
	explicit IntrusivePtr(IntrusivePtr<OtherT, DeleterT> &&rhs) noexcept
		: IntrusivePtr()
	{
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

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
	template<typename OtherT>
	IntrusivePtr &operator=(const IntrusivePtr<OtherT, DeleterT> &rhs) noexcept {
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(rhs);
		return *this;
	}
	template<typename OtherT>
	IntrusivePtr &operator=(IntrusivePtr<OtherT, DeleterT> &&rhs) noexcept {
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(std::move(rhs));
		return *this;
	}
	~IntrusivePtr(){
		Reset();
	}

public:
	bool IsValid() const noexcept {
		return Get() != nullptr;
	}
	ObjectT *Get() const noexcept {
		if(!xm_pBuddy){
			return nullptr;
		}
		return const_cast<ObjectT *>(xm_pBuddy->Get());
	}
	ObjectT *Release() noexcept {
		const auto pBuddy = std::exchange(xm_pBuddy, nullptr);
		if(!pBuddy){
			return nullptr;
		}
		return const_cast<ObjectT *>(pBuddy->Get());
	}
	IntrusivePtr Fork() const noexcept {
		return IntrusivePtr(*this);
	}

	IntrusivePtr &Reset(ObjectT *pObject = nullptr) noexcept {
		const auto pOld = std::exchange(xm_pBuddy, pObject);
		if(pOld){
			pOld->DropRef();
		}
		return *this;
	}
	template<typename OtherT>
	IntrusivePtr &Reset(const IntrusivePtr<OtherT, DeleterT> &rhs) noexcept {
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(rhs.Get());
		if(xm_pBuddy){
			xm_pBuddy->AddRef();
		}
		return *this;
	}
	template<typename OtherT>
	IntrusivePtr &Reset(IntrusivePtr<OtherT, DeleterT> &&rhs) noexcept {
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		return Reset(rhs.Release());
	}

	void Swap(IntrusivePtr &rhs) noexcept {
		std::swap(xm_pBuddy, rhs.xm_pBuddy);
	}

public:
	explicit operator bool() const noexcept {
		return IsValid();
	}
	explicit operator ObjectT *() const noexcept {
		return Get();
	}

	ObjectT &operator*() const noexcept {
		ASSERT(IsValid);
		return *Get();
	}
	ObjectT *operator->() const noexcept {
		ASSERT(IsValid);
		return Get();
	}
};

namespace Impl {
	template<typename ObjectT, class DeleterT>
		template<typename OtherT>
	IntrusivePtr<const volatile OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Fork() const volatile noexcept {
		const auto pForked = Get<const volatile OtherT>();
		if(!pForked){
			return nullptr;
		}
		AddRef();
		return IntrusivePtr<const volatile OtherT, DeleterT>(pForked);
	}
	template<typename ObjectT, class DeleterT>
		template<typename OtherT>
	IntrusivePtr<const OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Fork() const noexcept {
		const auto pForked = Get<const OtherT>();
		if(!pForked){
			return nullptr;
		}
		AddRef();
		return IntrusivePtr<const OtherT, DeleterT>(pForked);
	}
	template<typename ObjectT, class DeleterT>
		template<typename OtherT>
	IntrusivePtr<volatile OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Fork() volatile noexcept {
		const auto pForked = Get<volatile OtherT>();
		if(!pForked){
			return nullptr;
		}
		AddRef();
		return IntrusivePtr<const volatile OtherT, DeleterT>(pForked);
	}
	template<typename ObjectT, class DeleterT>
		template<typename OtherT>
	IntrusivePtr<OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Fork() noexcept {
		const auto pForked = Get<OtherT>();
		if(!pForked){
			return nullptr;
		}
		AddRef();
		return IntrusivePtr<OtherT, DeleterT>(pForked);
	}
}

template<typename ObjectT, class DeleterT>
bool operator==(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs.Get() == rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator==(const IntrusivePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() == rhs;
}
template<typename ObjectT, class DeleterT>
bool operator==(ObjectT *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs == rhs.Get();
}

template<typename ObjectT, class DeleterT>
bool operator!=(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs.Get() != rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator!=(const IntrusivePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() != rhs;
}
template<typename ObjectT, class DeleterT>
bool operator!=(ObjectT *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs != rhs.Get();
}

template<typename ObjectT, class DeleterT>
bool operator<(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs.Get() < rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator<(const IntrusivePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() < rhs;
}
template<typename ObjectT, class DeleterT>
bool operator<(ObjectT *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs < rhs.Get();
}

template<typename ObjectT, class DeleterT>
bool operator>(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs.Get() > rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator>(const IntrusivePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() > rhs;
}
template<typename ObjectT, class DeleterT>
bool operator>(ObjectT *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs > rhs.Get();
}

template<typename ObjectT, class DeleterT>
bool operator<=(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs.Get() <= rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator<=(const IntrusivePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() <= rhs;
}
template<typename ObjectT, class DeleterT>
bool operator<=(ObjectT *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs <= rhs.Get();
}

template<typename ObjectT, class DeleterT>
bool operator>=(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs.Get() >= rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator>=(const IntrusivePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() >= rhs;
}
template<typename ObjectT, class DeleterT>
bool operator>=(ObjectT *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs >= rhs.Get();
}

template<typename ObjectT, class DeleterT>
void swap(IntrusivePtr<ObjectT, DeleterT> &lhs, IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
