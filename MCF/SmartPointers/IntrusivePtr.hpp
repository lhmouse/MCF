// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_

// 1) 构造函数（包含复制构造函数、转移构造函数以及构造函数模板）、赋值运算符和析构函数应当调用 Reset()；
// 2) Reset() 的形参若具有 IntrusivePtr 的某模板类类型，则禁止传值，必须传引用。

#include "../Utilities/Assert.hpp"
#include "DefaultDeleter.hpp"
#include "_Traits.hpp"
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
		using Pointee = std::remove_cv_t<std::remove_reference_t<decltype(*DeleterT()())>>;

	private:
		Pointee *xm_pToDelete;

	public:
		explicit constexpr IntrusiveSentry(Pointee *pToDelete) noexcept
			: xm_pToDelete(pToDelete)
		{
		}
		IntrusiveSentry(IntrusiveSentry &&rhs) noexcept
			: xm_pToDelete(std::exchange(rhs.xm_pToDelete, nullptr))
		{
		}
		~IntrusiveSentry(){
			if(xm_pToDelete){
				DeleterT()(const_cast<Pointee *>(xm_pToDelete));
			}
		}

		IntrusiveSentry(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(const IntrusiveSentry &) = delete;
		IntrusiveSentry &operator=(IntrusiveSentry &&) noexcept = delete;
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
		using Pointee = typename Sentry::Pointee;

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
		std::size_t GetSharedCount() const volatile noexcept {
			return __atomic_load_n(&xm_uRefCount, __ATOMIC_RELAXED);
		}
		void AddRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);

			__atomic_add_fetch(&xm_uRefCount, 1, __ATOMIC_RELEASE);
		}
		auto DropRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);

			Pointee *pToDelete = nullptr;
			if(__atomic_sub_fetch(&xm_uRefCount, 1, __ATOMIC_ACQUIRE) == 0){
				pToDelete = static_cast<Pointee *>(const_cast<IntrusiveBase *>(this));
			}
			return Sentry(pToDelete);
		}

		template<typename OtherT>
		auto Get() const volatile noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);
			return IntrusiveCastHelper<const volatile OtherT, const volatile Pointee>()(static_cast<const volatile Pointee *>(this));
		}
		template<typename OtherT>
		auto Get() const noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);
			return IntrusiveCastHelper<const OtherT, const Pointee>()(static_cast<const Pointee *>(this));
		}
		template<typename OtherT>
		auto Get() volatile noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);
			return IntrusiveCastHelper<volatile OtherT, volatile Pointee>()(static_cast<volatile Pointee *>(this));
		}
		template<typename OtherT>
		auto Get() noexcept {
			ASSERT((std::ptrdiff_t)__atomic_load_n(&xm_uRefCount, __ATOMIC_ACQUIRE) > 0);
			return IntrusiveCastHelper<OtherT, Pointee>()(static_cast<Pointee *>(this));
		}

		template<typename OtherT>
		IntrusivePtr<const volatile OtherT, DeleterT> Share() const volatile noexcept;
		template<typename OtherT>
		IntrusivePtr<const OtherT, DeleterT> Share() const noexcept;
		template<typename OtherT>
		IntrusivePtr<volatile OtherT, DeleterT> Share() volatile noexcept;
		template<typename OtherT>
		IntrusivePtr<OtherT, DeleterT> Share() noexcept;
	};
}

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
using IntrusiveBase = Impl::IntrusiveBase<DeleterT>;

template<typename ObjectT, class DeleterT>
class IntrusivePtr
	: public Impl::SmartPointerCheckDereferencable<IntrusivePtr<ObjectT, DeleterT>, ObjectT>
	, public Impl::SmartPointerCheckArray<IntrusivePtr<ObjectT, DeleterT>, ObjectT>
{
	template<typename, class>
	friend class IntrusivePtr;

	static_assert(noexcept(DeleterT()(DeleterT()())), "Deleter must not throw.");
	static_assert(!std::is_array<ObjectT>::value, "IntrusivePtr does not support arrays.");

public:
	using Element = ObjectT;
	using Buddy = Impl::IntrusiveBase<DeleterT>;

private:
	const volatile Buddy *xm_pBuddy;

public:
	constexpr explicit IntrusivePtr(Element *pElement = nullptr) noexcept
		: xm_pBuddy(pElement)
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
		std::enable_if_t<std::is_convertible<OtherT *, Element *>::value, int> = 0>
	IntrusivePtr(IntrusivePtr<OtherT, DeleterT> rhs) noexcept
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
		std::enable_if_t<std::is_convertible<OtherT *, Element *>::value, int> = 0>
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
	Element *Get() const noexcept {
		if(!xm_pBuddy){
			return nullptr;
		}
		return const_cast<Element *>(xm_pBuddy->template Get<const volatile Element>());
	}
	auto ReleaseBuddy() noexcept {
		return const_cast<
			std::conditional_t<std::is_same<const volatile Element, Element>::value, const volatile Buddy *,
				std::conditional_t<std::is_same<const Element, Element>::value, const Buddy *,
					std::conditional_t<std::is_same<volatile Element, Element>::value, volatile Buddy *,
						Buddy *>
					>
				>
			>(std::exchange(xm_pBuddy, nullptr));
	}
	Element *Release() noexcept {
		const auto pOldBuddy = ReleaseBuddy();
		if(!pOldBuddy){
			return nullptr;
		}
		const auto pRet = pOldBuddy->template Get<Element>();
		if(!pRet){
			pOldBuddy->DropRef();
			return nullptr;
		}
		return pRet;
	}

	std::size_t GetSharedCount() const noexcept {
		if(!xm_pBuddy){
			return 0;
		}
		return xm_pBuddy->GetSharedCount();
	}
	IntrusivePtr Share() const noexcept {
		return IntrusivePtr(*this);
	}

	IntrusivePtr &Reset(Element *pElement = nullptr) noexcept {
		ASSERT(Get() != pElement);
		const auto pOldBuddy = std::exchange(xm_pBuddy, pElement);
		if(pOldBuddy){
			pOldBuddy->DropRef();
		}
		return *this;
	}
	template<typename OtherT,
		std::enable_if_t<std::is_convertible<OtherT *, Element *>::value, int> = 0>
	IntrusivePtr &Reset(const IntrusivePtr<OtherT, DeleterT> &rhs) noexcept {
		const auto pObject = static_cast<Element *>(rhs.Get());
		if(pObject){
			pObject->AddRef();
		}
		Reset(pObject);
		return *this;
	}
	template<typename OtherT,
		std::enable_if_t<std::is_convertible<OtherT *, Element *>::value, int> = 0>
	IntrusivePtr &Reset(IntrusivePtr<OtherT, DeleterT> &&rhs) noexcept {
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
	explicit operator Element *() const noexcept {
		return Get();
	}
};

namespace Impl {
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<const volatile OtherT, DeleterT> IntrusiveBase<DeleterT>::Share() const volatile noexcept {
		const auto pShared = Get<const volatile OtherT>();
		if(!pShared){
			return nullptr;
		}
		pShared->AddRef();
		return IntrusivePtr<const volatile OtherT, DeleterT>(pShared);
	}
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<const OtherT, DeleterT> IntrusiveBase<DeleterT>::Share() const noexcept {
		const auto pShared = Get<const OtherT>();
		if(!pShared){
			return nullptr;
		}
		pShared->AddRef();
		return IntrusivePtr<const OtherT, DeleterT>(pShared);
	}
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<volatile OtherT, DeleterT> IntrusiveBase<DeleterT>::Share() volatile noexcept {
		const auto pShared = Get<volatile OtherT>();
		if(!pShared){
			return nullptr;
		}
		pShared->AddRef();
		return IntrusivePtr<volatile OtherT, DeleterT>(pShared);
	}
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<OtherT, DeleterT> IntrusiveBase<DeleterT>::Share() noexcept {
		const auto pShared = Get<OtherT>();
		if(!pShared){
			return nullptr;
		}
		pShared->AddRef();
		return IntrusivePtr<OtherT, DeleterT>(pShared);
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
	return IntrusivePtr<DstT, DeleterT>(static_cast<DstT *>(rhs.Release()));
}
template<typename DstT, typename SrcT, class DeleterT>
auto DynamicPointerCast(IntrusivePtr<SrcT, DeleterT> rhs) noexcept {
	IntrusivePtr<DstT, DeleterT> pRet(dynamic_cast<DstT *>(rhs.Get()));
	if(pRet){
		rhs.ReleaseBuddy();
	}
	return std::move(pRet);
}
template<typename DstT, typename SrcT, class DeleterT>
auto ConstPointerCast(IntrusivePtr<SrcT, DeleterT> rhs) noexcept {
	return IntrusivePtr<DstT, DeleterT>(const_cast<DstT *>(rhs.Release()));
}

}

#endif
