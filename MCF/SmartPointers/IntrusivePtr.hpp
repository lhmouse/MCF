// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_

// 1) 构造函数（包含复制构造函数、转移构造函数以及构造函数模板）、赋值运算符和析构函数应当调用 Reset()；
// 2) Reset() 的形参若具有 IntrusivePtr 的某模板类类型，则禁止传值，必须传引用。

#include "../Utilities/Assert.hpp"
#include "../Thread/Atomic.hpp"
#include "DefaultDeleter.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
	class IntrusiveBase;
template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
	class IntrusivePtr;

namespace Impl {
	template<class DeleterT>
	class IntrusiveSentry {
	public:
		using Pointee = std::decay_t<decltype(*DeleterT()())>;

	private:
		Pointee *x_pToDelete;

	public:
		explicit constexpr IntrusiveSentry(Pointee *pToDelete) noexcept
			: x_pToDelete(pToDelete)
		{
		}
		IntrusiveSentry(IntrusiveSentry &&rhs) noexcept
			: x_pToDelete(std::exchange(rhs.x_pToDelete, nullptr))
		{
		}
		~IntrusiveSentry(){
			if(x_pToDelete){
				DeleterT()(const_cast<Pointee *>(x_pToDelete));
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
}

template<typename ObjectT, class DeleterT>
class IntrusiveBase {
public:
	using Sentry = Impl::IntrusiveSentry<DeleterT>;
	using Pointee = typename Sentry::Pointee;

private:
	mutable volatile std::size_t x_uRefCount;

protected:
	IntrusiveBase() noexcept {
		AtomicStore(x_uRefCount, 1, MemoryModel::RELAXED);
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
		return AtomicLoad(x_uRefCount, MemoryModel::RELAXED);
	}
	void AddRef() const volatile noexcept {
		ASSERT((std::ptrdiff_t)AtomicLoad(x_uRefCount, MemoryModel::RELAXED) > 0);

		AtomicIncrement(x_uRefCount, MemoryModel::RELAXED);
	}
	Sentry DropRef() const volatile noexcept {
		ASSERT((std::ptrdiff_t)AtomicLoad(x_uRefCount, MemoryModel::RELAXED) > 0);

		Pointee *pToDelete = nullptr;
		if(AtomicDecrement(x_uRefCount, MemoryModel::RELAXED) == 0){
			pToDelete = static_cast<Pointee *>(const_cast<IntrusiveBase *>(this));
		}
		return Sentry(pToDelete);
	}

	template<typename OtherT = ObjectT>
	const volatile OtherT *Get() const volatile noexcept {
		ASSERT((std::ptrdiff_t)AtomicLoad(x_uRefCount, MemoryModel::RELAXED) > 0);

		return Impl::IntrusiveCastHelper<const volatile OtherT, const volatile Pointee>()(
			static_cast<const volatile Pointee *>(this));
	}
	template<typename OtherT = ObjectT>
	const OtherT *Get() const noexcept {
		ASSERT((std::ptrdiff_t)AtomicLoad(x_uRefCount, MemoryModel::RELAXED) > 0);

		return Impl::IntrusiveCastHelper<const OtherT, const Pointee>()(
			static_cast<const Pointee *>(this));
	}
	template<typename OtherT = ObjectT>
	volatile OtherT *Get() volatile noexcept {
		ASSERT((std::ptrdiff_t)AtomicLoad(x_uRefCount, MemoryModel::RELAXED) > 0);

		return Impl::IntrusiveCastHelper<volatile OtherT, volatile Pointee>()(
			static_cast<volatile Pointee *>(this));
	}
	template<typename OtherT = ObjectT>
	OtherT *Get() noexcept {
		ASSERT((std::ptrdiff_t)AtomicLoad(x_uRefCount, MemoryModel::RELAXED) > 0);

		return Impl::IntrusiveCastHelper<OtherT, Pointee>()(
			static_cast<Pointee *>(this));
	}

	template<typename OtherT = ObjectT>
		IntrusivePtr<const volatile OtherT, DeleterT> Share() const volatile noexcept;
	template<typename OtherT = ObjectT>
		IntrusivePtr<const OtherT, DeleterT> Share() const noexcept;
	template<typename OtherT = ObjectT>
		IntrusivePtr<volatile OtherT, DeleterT> Share() volatile noexcept;
	template<typename OtherT = ObjectT>
		IntrusivePtr<OtherT, DeleterT> Share() noexcept;
};

template<typename ObjectT, class DeleterT>
class IntrusivePtr {
	template<typename, class>
		friend class IntrusivePtr;

	static_assert(noexcept(DeleterT()(DeleterT()())), "Deleter must not throw.");
	static_assert(!std::is_array<ObjectT>::value, "IntrusivePtr does not support arrays.");

public:
	using ElementType = ObjectT;
	using BuddyType = IntrusiveBase<std::remove_cv_t<ObjectT>, DeleterT>;

private:
	const volatile BuddyType *x_pBuddy;

public:
	explicit constexpr IntrusivePtr(ElementType *pElement = nullptr) noexcept
		: x_pBuddy(pElement)
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
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusivePtr(const IntrusivePtr<OtherT, OtherDeleterT> &rhs) noexcept
		: IntrusivePtr()
	{
		Reset(rhs);
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusivePtr(IntrusivePtr<OtherT, OtherDeleterT> &&rhs) noexcept
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
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusivePtr &operator=(const IntrusivePtr<OtherT, OtherDeleterT> &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusivePtr &operator=(IntrusivePtr<OtherT, OtherDeleterT> &&rhs) noexcept {
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
	ElementType *Get() const noexcept {
		if(!x_pBuddy){
			return nullptr;
		}
		return const_cast<ElementType *>(x_pBuddy->template Get<const volatile ElementType>());
	}
	auto ReleaseBuddy() noexcept {
		return const_cast<
			std::conditional_t<std::is_same<const volatile ElementType, ElementType>::value, const volatile BuddyType *,
				std::conditional_t<std::is_same<const ElementType, ElementType>::value, const BuddyType *,
					std::conditional_t<std::is_same<volatile ElementType, ElementType>::value, volatile BuddyType *,
						BuddyType *>>>
			>(std::exchange(x_pBuddy, nullptr));
	}
	ElementType *Release() noexcept {
		const auto pOldBuddy = ReleaseBuddy();
		if(!pOldBuddy){
			return nullptr;
		}
		const auto pRet = pOldBuddy->template Get<ElementType>();
		if(!pRet){
			pOldBuddy->DropRef();
			return nullptr;
		}
		return pRet;
	}

	bool IsUnique() const noexcept {
		return GetSharedCount() == 1;
	}
	std::size_t GetSharedCount() const noexcept {
		if(!x_pBuddy){
			return 0;
		}
		return x_pBuddy->GetSharedCount();
	}

	IntrusivePtr &Reset(ElementType *pElement = nullptr) noexcept {
		ASSERT(!(pElement && (Get() == pElement)));
		const auto pOldBuddy = std::exchange(x_pBuddy, pElement);
		if(pOldBuddy){
			pOldBuddy->DropRef();
		}
		return *this;
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusivePtr &Reset(const IntrusivePtr<OtherT, OtherDeleterT> &rhs) noexcept {
		const auto pObject = static_cast<ElementType *>(rhs.Get());
		if(pObject){
			pObject->AddRef();
		}
		Reset(pObject);
		return *this;
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusivePtr &Reset(IntrusivePtr<OtherT, OtherDeleterT> &&rhs) noexcept {
		Reset(rhs.Release());
		return *this;
	}

	void Swap(IntrusivePtr &rhs) noexcept {
		std::swap(x_pBuddy, rhs.x_pBuddy);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator ElementType *() const noexcept {
		return Get();
	}

	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value, ElementType> &operator*() const noexcept {
		ASSERT(IsNonnull());

		return *Get();
	}
	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value, ElementType> *operator->() const noexcept {
		ASSERT(IsNonnull());

		return Get();
	}
};

template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<const volatile OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() const volatile noexcept {
	const auto pShared = Get<const volatile OtherT>();
	if(!pShared){
		return nullptr;
	}
	pShared->AddRef();
	return IntrusivePtr<const volatile OtherT, DeleterT>(pShared);
}
template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<const OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() const noexcept {
	const auto pShared = Get<const OtherT>();
	if(!pShared){
		return nullptr;
	}
	pShared->AddRef();
	return IntrusivePtr<const OtherT, DeleterT>(pShared);
}
template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<volatile OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() volatile noexcept {
	const auto pShared = Get<volatile OtherT>();
	if(!pShared){
		return nullptr;
	}
	pShared->AddRef();
	return IntrusivePtr<volatile OtherT, DeleterT>(pShared);
}
template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() noexcept {
	const auto pShared = Get<OtherT>();
	if(!pShared){
		return nullptr;
	}
	pShared->AddRef();
	return IntrusivePtr<OtherT, DeleterT>(pShared);
}

#define MCF_SMART_POINTERS_DECLARE_TEMPLATE_PARAMETERS_	template<typename ObjectT, class DeleterT>
#define MCF_SMART_POINTERS_INVOKE_TEMPLATE_PARAMETERS_	IntrusivePtr<ObjectT, DeleterT>
#include "_RationalAndSwap.hpp"

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
	return pRet;
}
template<typename DstT, typename SrcT, class DeleterT>
auto ConstPointerCast(IntrusivePtr<SrcT, DeleterT> rhs) noexcept {
	return IntrusivePtr<DstT, DeleterT>(const_cast<DstT *>(rhs.Release()));
}

}

#endif
