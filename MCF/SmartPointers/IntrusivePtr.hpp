// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_

#include "../Utilities/Assert.hpp"
#include "DefaultDeleter.hpp"
#include "Traits.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class IntrusivePtr;

namespace Impl {
	template<class DeleterT>
	class IntrusiveSentry {
	public:
		using Object = std::remove_cv_t<std::remove_reference_t<decltype(*DeleterT()())>>;

	private:
		Object *xm_pToDelete;

	public:
		explicit IntrusiveSentry(Object *pToDelete) noexcept
			: xm_pToDelete(pToDelete)
		{
		}
		IntrusiveSentry(IntrusiveSentry &&rhs) noexcept
			: xm_pToDelete(std::exchange(rhs.xm_pToDelete, nullptr))
		{
		}
		~IntrusiveSentry(){
			if(xm_pToDelete){
				DeleterT()(const_cast<Object *>(xm_pToDelete));
			}
		}

		IntrusiveSentry(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(IntrusiveSentry &&) = delete;
	};

	template<typename DstT, typename SrcT, typename = DstT *>
	struct IntrusiveCastHelper {
		DstT *operator()(SrcT *pSrc) const noexcept {
			return dynamic_cast<DstT *>(pSrc);
		}
	};
	template<typename DstT, typename SrcT>
	struct IntrusiveCastHelper<DstT, SrcT,
		decltype(static_cast<DstT *>(std::declval<SrcT *>()))>
	{
		constexpr DstT *operator()(SrcT *pSrc) const noexcept {
			return static_cast<DstT *>(pSrc);
		}
	};

	template<class DeleterT>
	class IntrusiveBase {
	public:
		using Sentry = IntrusiveSentry<DeleterT>;
		using Object = typename Sentry::Object;

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
		std::size_t GetRef() const volatile noexcept {
			return __atomic_load_n(&xm_uRefCount, __ATOMIC_RELAXED);
		}
		void AddRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);

			__atomic_add_fetch(&xm_uRefCount, 1, __ATOMIC_RELEASE);
		}
		auto DropRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);

			Object *pToDelete = nullptr;
			if(__atomic_sub_fetch(&xm_uRefCount, 1, __ATOMIC_ACQUIRE) == 0){
				pToDelete = static_cast<Object *>(const_cast<IntrusiveBase *>(this));
			}
			return Sentry(pToDelete);
		}

		template<typename OtherT = Object>
		auto Get() const volatile noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);
			return IntrusiveCastHelper<const volatile OtherT, const volatile Object>()(static_cast<const volatile Object *>(this));
		}
		template<typename OtherT = Object>
		auto Get() const noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);
			return IntrusiveCastHelper<const OtherT, const Object>()(static_cast<const Object *>(this));
		}
		template<typename OtherT = Object>
		auto Get() volatile noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);
			return IntrusiveCastHelper<volatile OtherT, volatile Object>()(static_cast<volatile Object *>(this));
		}
		template<typename OtherT = Object>
		auto Get() noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);
			return IntrusiveCastHelper<OtherT, Object>()(static_cast<Object *>(this));
		}

		template<typename OtherT = Object>
		IntrusivePtr<const volatile OtherT, DeleterT> Fork() const volatile noexcept;
		template<typename OtherT = Object>
		IntrusivePtr<const OtherT, DeleterT> Fork() const noexcept;
		template<typename OtherT = Object>
		IntrusivePtr<volatile OtherT, DeleterT> Fork() volatile noexcept;
		template<typename OtherT = Object>
		IntrusivePtr<OtherT, DeleterT> Fork() noexcept;
	};
}

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
using IntrusiveBase = Impl::IntrusiveBase<DeleterT>;

template<typename ObjectT, class DeleterT>
class IntrusivePtr
	: public Impl::SmartPointerCheckDereferencable<IntrusivePtr<ObjectT, DeleterT>, ObjectT>
	, public Impl::SmartPointerCheckArray<IntrusivePtr<ObjectT, DeleterT>, ObjectT>
{
	static_assert(noexcept(DeleterT()(DeleterT()())), "Deleter must not throw.");
	static_assert(!std::is_array<ObjectT>::value, "IntrusivePtr does not support arrays.");

public:
	using Buddy = Impl::IntrusiveBase<DeleterT>;

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
	template<typename OtherT,
		std::enable_if_t<std::is_convertible<OtherT *, ObjectT *>::value, int> = 0>
	explicit IntrusivePtr(IntrusivePtr<OtherT, DeleterT> rhs) noexcept
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
	template<typename OtherT,
		std::enable_if_t<std::is_convertible<OtherT *, ObjectT *>::value, int> = 0>
	IntrusivePtr &operator=(IntrusivePtr<OtherT, DeleterT> rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~IntrusivePtr(){
		Reset();
	}

public:
	bool IsNonnull() const noexcept {
		return Get() != nullptr;
	}
	ObjectT *Get() const noexcept {
		if(!xm_pBuddy){
			return nullptr;
		}
		return const_cast<ObjectT *>(xm_pBuddy->template Get<const volatile ObjectT>());
	}
	ObjectT *Release() noexcept {
		const auto pRet = Get();
		if(pRet){
			// pRet 现在持有所有权，释放原本的所有权。
			xm_pBuddy = nullptr;
		}
		return pRet;
	}

	std::size_t GetRef() const noexcept {
		if(!xm_pBuddy){
			return 0;
		}
		return xm_pBuddy->GetRef();
	}
	IntrusivePtr Share() const noexcept {
		return IntrusivePtr(*this);
	}

	IntrusivePtr &Reset(ObjectT *pObject = nullptr) noexcept {
		const auto pOld = std::exchange(xm_pBuddy, pObject);
		if(pOld){
			ASSERT(pOld != pObject);
			pOld->DropRef();
		}
		return *this;
	}
	template<typename OtherT,
		std::enable_if_t<std::is_convertible<OtherT *, ObjectT *>::value, int> = 0>
	IntrusivePtr &Reset(IntrusivePtr<OtherT, DeleterT> rhs) noexcept {
		Reset(rhs.Release());
		return *this;
	}

	void Swap(IntrusivePtr &rhs) noexcept {
		std::swap(xm_pBuddy, rhs.xm_pBuddy);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator ObjectT *() const noexcept {
		return Get();
	}
};

namespace Impl {
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<const volatile OtherT, DeleterT> IntrusiveBase<DeleterT>::Fork() const volatile noexcept {
		const auto pForked = Get<const volatile OtherT>();
		if(!pForked){
			return nullptr;
		}
		pForked->AddRef();
		return IntrusivePtr<const volatile OtherT, DeleterT>(pForked);
	}
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<const OtherT, DeleterT> IntrusiveBase<DeleterT>::Fork() const noexcept {
		const auto pForked = Get<const OtherT>();
		if(!pForked){
			return nullptr;
		}
		pForked->AddRef();
		return IntrusivePtr<const OtherT, DeleterT>(pForked);
	}
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<volatile OtherT, DeleterT> IntrusiveBase<DeleterT>::Fork() volatile noexcept {
		const auto pForked = Get<volatile OtherT>();
		if(!pForked){
			return nullptr;
		}
		pForked->AddRef();
		return IntrusivePtr<volatile OtherT, DeleterT>(pForked);
	}
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<OtherT, DeleterT> IntrusiveBase<DeleterT>::Fork() noexcept {
		const auto pForked = Get<OtherT>();
		if(!pForked){
			return nullptr;
		}
		pForked->AddRef();
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

template<typename ObjectT, typename ...ParamsT>
auto MakeIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	return IntrusivePtr<ObjectT, DefaultDeleter<ObjectT>>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT, class DeleterT>
auto StaticPointerCast(IntrusivePtr<SrcT, DeleterT> rhs) noexcept {
	SrcT *const pSrc = rhs.Release();
	DstT *const pDst = static_cast<DstT *>(pSrc);
	return IntrusivePtr<DstT, DeleterT>(pDst);
}
template<typename DstT, typename SrcT, class DeleterT>
auto DynamicPointerCast(IntrusivePtr<SrcT, DeleterT> rhs) noexcept {
	SrcT *const pSrc = rhs.Release();
	DstT *const pDst = dynamic_cast<DstT *>(pSrc);
	if(!pDst && pSrc){
		pSrc->DropRef();
	}
	return IntrusivePtr<DstT, DeleterT>(pDst);
}

}

#endif
