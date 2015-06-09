// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_

// 1) 构造函数（包含复制构造函数、转移构造函数以及构造函数模板）、赋值运算符和析构函数应当调用 Reset()；
// 2) Reset() 的形参若具有 IntrusivePtr 的某模板类类型，则禁止传值，必须传引用。

#include "../Utilities/Assert.hpp"
#include "../Thread/_SpinLock.hpp"
#include "../Thread/Atomic.hpp"
#include "DefaultDeleter.hpp"
#include "UniquePtr.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class IntrusiveBase;

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class IntrusivePtr;

namespace Impl_IntrusivePtr {
	class RefCountBase {
	private:
		mutable volatile std::size_t x_uRef;

	protected:
		RefCountBase() noexcept {
			AtomicStore(x_uRef, 1, MemoryModel::kRelaxed);
		}

	public:
		RefCountBase(const RefCountBase &) noexcept
			: RefCountBase() // 默认构造。
		{
		}
		RefCountBase(RefCountBase &&) noexcept
			: RefCountBase() // 同上。
		{
		}
		RefCountBase &operator=(const RefCountBase &) noexcept {
			return *this; // 无操作。
		}
		RefCountBase &operator=(RefCountBase &&) noexcept {
			return *this; // 同上。
		}
		~RefCountBase(){
			ASSERT(AtomicLoad(x_uRef, MemoryModel::kRelaxed) == 0);
		}

	public:
		std::size_t GetRef() const volatile noexcept {
			return AtomicLoad(x_uRef, MemoryModel::kRelaxed);
		}
		bool TryAddRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)AtomicLoad(x_uRef, MemoryModel::kRelaxed) >= 0);

			auto uOldRef = AtomicLoad(x_uRef, MemoryModel::kRelaxed);
			return AtomicCompareExchange(x_uRef, uOldRef, uOldRef + 1, MemoryModel::kRelaxed);
		}
		void AddRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)AtomicLoad(x_uRef, MemoryModel::kRelaxed) > 0);

			AtomicIncrement(x_uRef, MemoryModel::kRelaxed);
		}
		bool DropRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)AtomicLoad(x_uRef, MemoryModel::kRelaxed) > 0);

			return AtomicDecrement(x_uRef, MemoryModel::kRelaxed) == 0;
		}
	};

	template<typename DstT, typename SrcT>
	struct CvCopier {
		using Type = DstT;
	};
	template<typename DstT, typename SrcT>
	struct CvCopier<DstT, const SrcT> {
		using Type = const DstT;
	};
	template<typename DstT, typename SrcT>
	struct CvCopier<DstT, volatile SrcT> {
		using Type = volatile DstT;
	};
	template<typename DstT, typename SrcT>
	struct CvCopier<DstT, const volatile SrcT> {
		using Type = const volatile DstT;
	};

	template<typename DstT, typename SrcT, typename = DstT>
	struct StaticOrDynamicCastHelper {
		DstT operator()(SrcT &&vSrc) const {
			return dynamic_cast<DstT>(std::forward<SrcT>(vSrc));
		}
	};
	template<typename DstT, typename SrcT>
	struct StaticOrDynamicCastHelper<DstT, SrcT,
		decltype(static_cast<DstT>(std::declval<SrcT>()))>
	{
		constexpr DstT operator()(SrcT &&vSrc) const noexcept {
			return static_cast<DstT>(std::forward<SrcT>(vSrc));
		}
	};

	template<typename DstT, typename SrcT>
	DstT StaticOrDynamicCast(SrcT &&vSrc){
		return StaticOrDynamicCastHelper<DstT, SrcT>()(std::forward<SrcT>(vSrc));
	}
}

template<typename ObjectT, class DeleterT>
class IntrusiveBase : public Impl_IntrusivePtr::RefCountBase {
public:
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
	static_assert(noexcept(DeleterT()(DeleterT()())), "Deleter must not throw.");
	static_assert(!std::is_array<ObjectT>::value, "IntrusivePtr does not support arrays.");

public:
	using ElementType = ObjectT;
	using BuddyType = typename Impl_IntrusivePtr::CvCopier<std::decay_t<decltype(*DeleterT()())>, ElementType>::Type;

	static_assert(std::is_base_of<BuddyType, ElementType>::value, "ElementType is not derived from BuddyType ??");
	static_assert(std::is_base_of<Impl_IntrusivePtr::RefCountBase, BuddyType>::value, "BuddyType is not derived from Impl_IntrusivePtr::RefCountBase ??");

private:
	BuddyType *x_pBuddy;

public:
	explicit constexpr IntrusivePtr(ElementType *pElement = nullptr) noexcept
		: x_pBuddy(pElement)
	{
	}
	explicit IntrusivePtr(UniquePtr<ObjectT, DeleterT> &&rhs) noexcept
		: IntrusivePtr()
	{
		Reset(std::move(rhs));
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
	IntrusivePtr &operator=(UniquePtr<ObjectT, DeleterT> &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
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
		return !!x_pBuddy;
	}
	BuddyType *GetBuddy() const noexcept {
		return x_pBuddy;
	}
	BuddyType *ReleaseBuddy() noexcept {
		return std::exchange(x_pBuddy, nullptr);
	}
	ElementType *Get() const noexcept {
		return Impl_IntrusivePtr::StaticOrDynamicCast<ElementType *>(x_pBuddy);
	}
	ElementType *Release() noexcept {
		ElementType *pRet = nullptr;
		const auto pOldBuddy = std::exchange(x_pBuddy, nullptr);
		if(pOldBuddy){
			pRet = Impl_IntrusivePtr::StaticOrDynamicCast<ElementType *>(pOldBuddy);
			ASSERT(pRet);
		}
		return pRet;
	}

	bool IsUnique() const noexcept {
		return GetSharedCount() == 1;
	}
	std::size_t GetSharedCount() const noexcept {
		return x_pBuddy ? x_pBuddy->Impl_IntrusivePtr::RefCountBase::GetRef() : 0;
	}

	IntrusivePtr &Reset(ElementType *pElement = nullptr) noexcept {
		const auto pOldBuddy = std::exchange(x_pBuddy, pElement);
		if(pOldBuddy){
			if(pOldBuddy->Impl_IntrusivePtr::RefCountBase::DropRef()){
				DeleterT()(const_cast<std::remove_cv_t<BuddyType> *>(pOldBuddy));
			}
		}
		return *this;
	}
	IntrusivePtr &Reset(UniquePtr<ObjectT, DeleterT> &&rhs) noexcept {
		return Reset(rhs.Release());
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusivePtr &Reset(const IntrusivePtr<OtherT, OtherDeleterT> &rhs) noexcept {
		const auto pElement = rhs.Get();
		if(pElement){
			pElement->Impl_IntrusivePtr::RefCountBase::AddRef();
		}
		return Reset(pElement);
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusivePtr &Reset(IntrusivePtr<OtherT, OtherDeleterT> &&rhs) noexcept {
		return Reset(static_cast<ElementType *>(rhs.Release()));
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
	IntrusivePtr<const volatile OtherT, DeleterT> pRet;
	const auto pObj = Impl_IntrusivePtr::StaticOrDynamicCast<const volatile OtherT *>(this);
	if(pObj){
		pObj->Impl_IntrusivePtr::RefCountBase::AddRef();
		pRet.Reset(pObj);
	}
	return pRet;
}
template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<const OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() const noexcept {
	IntrusivePtr<const OtherT, DeleterT> pRet;
	const auto pObj = Impl_IntrusivePtr::StaticOrDynamicCast<const OtherT *>(this);
	if(pObj){
		pObj->Impl_IntrusivePtr::RefCountBase::AddRef();
		pRet.Reset(pObj);
	}
	return pRet;
}
template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<volatile OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() volatile noexcept {
	IntrusivePtr<volatile OtherT, DeleterT> pRet;
	const auto pObj = Impl_IntrusivePtr::StaticOrDynamicCast<volatile OtherT *>(this);
	if(pObj){
		pObj->Impl_IntrusivePtr::RefCountBase::AddRef();
		pRet.Reset(pObj);
	}
	return pRet;
}
template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() noexcept {
	IntrusivePtr<OtherT, DeleterT> pRet;
	const auto pObj = Impl_IntrusivePtr::StaticOrDynamicCast<OtherT *>(this);
	if(pObj){
		pObj->Impl_IntrusivePtr::RefCountBase::AddRef();
		pRet.Reset(pObj);
	}
	return pRet;
}

#define MCF_SMART_POINTERS_DECLARE_TEMPLATE_PARAMETERS_	template<typename ObjectT, class DeleterT>
#define MCF_SMART_POINTERS_INVOKE_TEMPLATE_PARAMETERS_	IntrusivePtr<ObjectT, DeleterT>
#include "_RationalAndSwap.hpp"

template<typename ObjectT, typename ...ParamsT>
IntrusivePtr<ObjectT, DefaultDeleter<ObjectT>> MakeIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	return IntrusivePtr<ObjectT, DefaultDeleter<ObjectT>>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT, class DeleterT>
IntrusivePtr<DstT, DeleterT> StaticPointerCast(IntrusivePtr<SrcT, DeleterT> rhs) noexcept {
	return IntrusivePtr<DstT, DeleterT>(static_cast<DstT *>(rhs.Release()));
}
template<typename DstT, typename SrcT, class DeleterT>
IntrusivePtr<DstT, DeleterT> DynamicPointerCast(IntrusivePtr<SrcT, DeleterT> rhs) noexcept {
	const auto pObj = dynamic_cast<DstT *>(rhs.Get());
	if(!pObj){
		return nullptr;
	}
	rhs.Release();
	return IntrusivePtr<DstT, DeleterT>(pObj);
}
template<typename DstT, typename SrcT, class DeleterT>
IntrusivePtr<DstT, DeleterT> ConstPointerCast(IntrusivePtr<SrcT, DeleterT> rhs) noexcept {
	return IntrusivePtr<DstT, DeleterT>(const_cast<DstT *>(rhs.Release()));
}

}

#endif
