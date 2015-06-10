// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_

#include "../Utilities/Assert.hpp"
// #include "../Thread/_SpinLock.hpp"
#include "../Thread/Atomic.hpp"
#include "DefaultDeleter.hpp"
#include "UniquePtr.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<ObjectT>>
class IntrusiveBase;

template<typename ObjectT, class DeleterT = DefaultDeleter<ObjectT>>
class IntrusivePtr;
template<typename ObjectT, class DeleterT = DefaultDeleter<ObjectT>>
class IntrusiveWeakPtr;

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
	friend IntrusivePtr<ObjectT, DeleterT>;
	friend IntrusiveWeakPtr<ObjectT, DeleterT>;

private:
	template<typename CvOtherT, typename CvThisT>
	static IntrusivePtr<CvOtherT, DeleterT> xForkShared(CvThisT *pThis) noexcept;

public:
	virtual ~IntrusiveBase() = default;

public:
	template<typename OtherT = ObjectT>
	IntrusivePtr<const volatile OtherT, DeleterT> Share() const volatile noexcept {
		return xForkShared<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<const OtherT, DeleterT> Share() const noexcept {
		return xForkShared<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<volatile OtherT, DeleterT> Share() volatile noexcept {
		return xForkShared<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<OtherT, DeleterT> Share() noexcept {
		return xForkShared<OtherT>(this);
	}
};

template<typename ObjectT, class DeleterT>
class IntrusivePtr {
public:
	static_assert(!std::is_array<ObjectT>::value, "Intrusive pointers don't accept arrays.");

	using Element = ObjectT;
	using Deleter = DeleterT;

	static_assert(noexcept(Deleter()(std::declval<std::remove_cv_t<Element> *>())), "Deleter must not throw.");

private:
	Element *x_pElement;

public:
	constexpr IntrusivePtr(std::nullptr_t = nullptr) noexcept
		: x_pElement(nullptr)
	{
	}
	explicit IntrusivePtr(Element *rhs) noexcept
		: IntrusivePtr()
	{
		Reset(rhs);
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<OtherObjectT *, Element *>::value,
			int> = 0>
	IntrusivePtr(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs) noexcept
		: IntrusivePtr()
	{
		Reset(rhs);
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<OtherObjectT *, Element *>::value,
			int> = 0>
	IntrusivePtr(IntrusivePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
		: IntrusivePtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<OtherObjectT *, Element *>::value,
			int> = 0>
	IntrusivePtr(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
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
	IntrusivePtr &operator=(std::nullptr_t) noexcept {
		return Reset();
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<OtherObjectT *, Element *>::value,
			int> = 0>
	IntrusivePtr &operator=(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs) noexcept {
		return Reset(rhs);
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<OtherObjectT *, Element *>::value,
			int> = 0>
	IntrusivePtr &operator=(IntrusivePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<OtherObjectT *, Element *>::value,
			int> = 0>
	IntrusivePtr &operator=(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	IntrusivePtr &operator=(const IntrusivePtr &rhs) noexcept {
		return Reset(rhs);
	}
	IntrusivePtr &operator=(IntrusivePtr &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~IntrusivePtr(){
		Reset();
	}

public:
	bool IsNonnull() const noexcept {
		return !!x_pElement;
	}
	Element *Get() const noexcept {
		return x_pElement;
	}
	Element *Release() noexcept {
		return std::exchange(x_pElement, nullptr);
	}

	IntrusivePtr &Reset(Element *pElement = nullptr) noexcept {
		const auto pOldElement = std::exchange(x_pElement, pElement);
		if(pOldElement){
			ASSERT(pOldElement != pElement);

			if(static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pOldElement)->DropRef()){
				Deleter()(const_cast<std::remove_cv_t<Element> *>(pOldElement));
			}
		}
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<OtherObjectT *, Element *>::value,
			int> = 0>
	IntrusivePtr &Reset(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs) noexcept {
		const auto pElement = rhs.Get();
		if(pElement){
			static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pElement)->AddRef();
		}
		return Reset(pElement);
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<OtherObjectT *, Element *>::value,
			int> = 0>
	IntrusivePtr &Reset(IntrusivePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		return Reset(rhs.Release());
	}

	void Swap(IntrusivePtr &rhs) noexcept {
		std::swap(x_pElement, rhs.x_pElement);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator Element *() const noexcept {
		return Get();
	}

	Element &operator*() const noexcept {
		ASSERT(IsNonnull());

		return *Get();
	}
	Element *operator->() const noexcept {
		ASSERT(IsNonnull());

		return Get();
	}
};

template<typename ObjectT, class DeleterT>
	template<typename CvOtherT, typename CvThisT>
IntrusivePtr<CvOtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::xForkShared(CvThisT *pThis) noexcept {
	const auto pOther = Impl_IntrusivePtr::StaticOrDynamicCast<CvOtherT *>(pThis);
	if(!pOther){
		return nullptr;
	}
	static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pOther)->AddRef();
	return IntrusivePtr<CvOtherT, DeleterT>(pOther);
}

template<typename ObjectT, class DeleterT>
bool operator==(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::equal_to<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectT, class DeleterT>
bool operator==(const IntrusivePtr<ObjectT, DeleterT> &lhs, typename IntrusivePtr<ObjectT, DeleterT>::Element *rhs) noexcept {
	return std::equal_to<void>()(lhs.Get(), rhs);
}
template<typename ObjectT, class DeleterT>
bool operator==(typename IntrusivePtr<ObjectT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::equal_to<void>()(lhs, rhs.Get());
}

template<typename ObjectT, class DeleterT>
bool operator!=(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::not_equal_to<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectT, class DeleterT>
bool operator!=(const IntrusivePtr<ObjectT, DeleterT> &lhs, typename IntrusivePtr<ObjectT, DeleterT>::Element *rhs) noexcept {
	return std::not_equal_to<void>()(lhs.Get(), rhs);
}
template<typename ObjectT, class DeleterT>
bool operator!=(typename IntrusivePtr<ObjectT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::not_equal_to<void>()(lhs, rhs.Get());
}

template<typename ObjectT, class DeleterT>
bool operator<(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::less<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectT, class DeleterT>
bool operator<(const IntrusivePtr<ObjectT, DeleterT> &lhs, typename IntrusivePtr<ObjectT, DeleterT>::Element *rhs) noexcept {
	return std::less<void>()(lhs.Get(), rhs);
}
template<typename ObjectT, class DeleterT>
bool operator<(typename IntrusivePtr<ObjectT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::less<void>()(lhs, rhs.Get());
}

template<typename ObjectT, class DeleterT>
bool operator>(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::greater<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectT, class DeleterT>
bool operator>(const IntrusivePtr<ObjectT, DeleterT> &lhs, typename IntrusivePtr<ObjectT, DeleterT>::Element *rhs) noexcept {
	return std::greater<void>()(lhs.Get(), rhs);
}
template<typename ObjectT, class DeleterT>
bool operator>(typename IntrusivePtr<ObjectT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::greater<void>()(lhs, rhs.Get());
}

template<typename ObjectT, class DeleterT>
bool operator<=(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::less_equal<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectT, class DeleterT>
bool operator<=(const IntrusivePtr<ObjectT, DeleterT> &lhs, typename IntrusivePtr<ObjectT, DeleterT>::Element *rhs) noexcept {
	return std::less_equal<void>()(lhs.Get(), rhs);
}
template<typename ObjectT, class DeleterT>
bool operator<=(typename IntrusivePtr<ObjectT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::less_equal<void>()(lhs, rhs.Get());
}

template<typename ObjectT, class DeleterT>
bool operator>=(const IntrusivePtr<ObjectT, DeleterT> &lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::greater_equal<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectT, class DeleterT>
bool operator>=(const IntrusivePtr<ObjectT, DeleterT> &lhs, typename IntrusivePtr<ObjectT, DeleterT>::Element *rhs) noexcept {
	return std::greater_equal<void>()(lhs.Get(), rhs);
}
template<typename ObjectT, class DeleterT>
bool operator>=(typename IntrusivePtr<ObjectT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return std::greater_equal<void>()(lhs, rhs.Get());
}

template<typename ObjectT, class DeleterT>
void swap(IntrusivePtr<ObjectT, DeleterT> &lhs, IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ObjectT, typename DeleterT = DefaultDeleter<ObjectT>, typename ...ParamsT>
IntrusivePtr<ObjectT, DeleterT> MakeIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return IntrusivePtr<ObjectT, DeleterT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT, class DeleterT>
IntrusivePtr<DstT, DeleterT> StaticPointerCast(IntrusivePtr<SrcT, DeleterT> pSrc) noexcept {
	return IntrusivePtr<DstT, DeleterT>(static_cast<DstT *>(pSrc.Release()));
}
template<typename DstT, typename SrcT, class DeleterT>
IntrusivePtr<DstT, DeleterT> ConstPointerCast(IntrusivePtr<SrcT, DeleterT> pSrc) noexcept {
	return IntrusivePtr<DstT, DeleterT>(const_cast<DstT *>(pSrc.Release()));
}

template<typename DstT, typename SrcT, class DeleterT>
IntrusivePtr<DstT, DeleterT> DynamicPointerCast(IntrusivePtr<SrcT, DeleterT> pSrc) noexcept {
	const auto pTest = dynamic_cast<DstT *>(pSrc.Get());
	if(!pTest){
		return nullptr;
	}
	pSrc.Release();
	return IntrusivePtr<DstT, DeleterT>(pTest);
}




/*namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class IntrusiveBase;

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class IntrusivePtr;
template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class IntrusiveWeakPtr;

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
	friend IntrusivePtr<ObjectT, DeleterT>;
	friend IntrusiveWeakPtr<ObjectT, DeleterT>;

private:
	class xWeakObserver : public Impl_IntrusivePtr::RefCountBase {
	private:
		mutable SpinLock x_splOwnerLock;
		IntrusiveBase *x_pOwner;

	public:
		explicit xWeakObserver(IntrusiveBase *pOwner) noexcept
			: x_pOwner(pOwner)
		{
		}

	public:
		IntrusiveBase *GetOwner() const noexcept {
			IntrusiveBase *pRet = nullptr;
			const auto uLocked = x_splOwnerLock.Lock();
			{
				if(x_pOwner && x_pOwner->Impl_IntrusivePtr::RefCountBase::TryAddRef()){
					pRet = x_pOwner;
				}
			}
			x_splOwnerLock.Unlock(uLocked);
			return pRet;
		}
		void ClearOwner() noexcept {
			const auto uLocked = x_splOwnerLock.Lock();
			{
				x_pOwner = nullptr;
			}
			x_splOwnerLock.Unlock(uLocked);
		}
	};

private:
	mutable xWeakObserver *volatile x_pObserver;

protected:
	IntrusiveBase() noexcept {
		AtomicStore(x_pObserver, nullptr, MemoryModel::kRelease);
	}
	IntrusiveBase(const IntrusiveBase &) noexcept {
		AtomicStore(x_pObserver, nullptr, MemoryModel::kRelease);
	}
	IntrusiveBase &operator=(const IntrusiveBase &) noexcept {
		return *this;
	}
	~IntrusiveBase(){
		const auto pObserver = AtomicLoad(x_pObserver, MemoryModel::kAcquire);
		if(pObserver){
			if(pObserver->Impl_IntrusivePtr::RefCountBase::DropRef()){
				delete pObserver;
			} else {
				pObserver->ClearOwner();
			}
		}
	}

private:
	xWeakObserver *xCreateWeakObserver() const volatile {
		auto pRet = AtomicLoad(x_pObserver, MemoryModel::kAcquire);
		if(!pRet){
			auto pObserver = new xWeakObserver(const_cast<IntrusiveBase *>(this));
			if(AtomicCompareExchange(x_pObserver, pRet, pObserver, MemoryModel::kAcqRel)){
				pRet = pObserver;
			} else {
				delete pObserver;
			}
		}
		return pRet;
	}

public:
	template<typename OtherT = ObjectT>
	IntrusivePtr<const volatile OtherT, DeleterT> Share() const volatile noexcept;
	template<typename OtherT = ObjectT>
	IntrusivePtr<const OtherT, DeleterT> Share() const noexcept;
	template<typename OtherT = ObjectT>
	IntrusivePtr<volatile OtherT, DeleterT> Share() volatile noexcept;
	template<typename OtherT = ObjectT>
	IntrusivePtr<OtherT, DeleterT> Share() noexcept;

	template<typename OtherT = ObjectT>
	IntrusivePtr<const volatile OtherT, DeleterT> Weaken() const volatile noexcept;
	template<typename OtherT = ObjectT>
	IntrusivePtr<const OtherT, DeleterT> Weaken() const noexcept;
	template<typename OtherT = ObjectT>
	IntrusivePtr<volatile OtherT, DeleterT> Weaken() volatile noexcept;
	template<typename OtherT = ObjectT>
	IntrusivePtr<OtherT, DeleterT> Weaken() noexcept;
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
	constexpr IntrusivePtr(std::nullptr_t = nullptr) noexcept
		: x_pBuddy(nullptr)
	{
	}
	explicit IntrusivePtr(ElementType *rhs) noexcept
		: IntrusivePtr()
	{
		Reset(rhs);
	}
	explicit IntrusivePtr(IntrusivePtr<ObjectT, DeleterT> &&rhs) noexcept
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
	IntrusivePtr &operator=(std::nullptr_t) noexcept {
		Reset();
		return *this;
	}
	IntrusivePtr &operator=(IntrusivePtr<ObjectT, DeleterT> &&rhs) noexcept {
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
	IntrusivePtr &Reset(IntrusivePtr<ObjectT, DeleterT> &&rhs) noexcept {
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
class IntrusiveWeakPtr {
	static_assert(noexcept(DeleterT()(DeleterT()())), "Deleter must not throw.");
	static_assert(!std::is_array<ObjectT>::value, "IntrusiveWeakPtr does not support arrays.");

private:
	using xBase = IntrusiveBase<ObjectT, DeleterT>;
	using xWeakObserver = typename xBase::xWeakObserver;

public:
	using ElementType = ObjectT;

private:
	xWeakObserver *x_pObserver;

public:
	constexpr IntrusiveWeakPtr(std::nullptr_t = nullptr) noexcept
		: x_pObserver(nullptr)
	{
	}
	explicit IntrusiveWeakPtr(ElementType *rhs) noexcept
		: IntrusiveWeakPtr()
	{
		Reset(rhs);
	}
	IntrusiveWeakPtr(const IntrusiveWeakPtr &rhs) noexcept
		: IntrusiveWeakPtr()
	{
		Reset(rhs);
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusiveWeakPtr(const IntrusivePtr<OtherT, OtherDeleterT> &rhs) noexcept
		: IntrusiveWeakPtr()
	{
		Reset(rhs);
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusiveWeakPtr(const IntrusiveWeakPtr<OtherT, OtherDeleterT> &rhs) noexcept
		: IntrusiveWeakPtr()
	{
		Reset(rhs);
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusiveWeakPtr(IntrusiveWeakPtr<OtherT, OtherDeleterT> &&rhs) noexcept
		: IntrusiveWeakPtr()
	{
		Reset(std::move(rhs));
	}
	IntrusiveWeakPtr &operator=(std::nullptr_t) noexcept {
		Reset();
		return *this;
	}
	IntrusiveWeakPtr &operator=(const IntrusiveWeakPtr &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusiveWeakPtr &operator=(const IntrusivePtr<OtherT, OtherDeleterT> &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusiveWeakPtr &operator=(const IntrusiveWeakPtr<OtherT, OtherDeleterT> &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusiveWeakPtr &operator=(IntrusiveWeakPtr<OtherT, OtherDeleterT> &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~IntrusiveWeakPtr(){
		Reset();
	}

private:
	IntrusiveWeakPtr &xInternalReset(xWeakObserver *pNewObserver) noexcept {
		const auto pOldObserver = std::exchange(x_pObserver, pNewObserver);
		if(pOldObserver){
			if(pOldObserver->Impl_IntrusivePtr::RefCountBase::DropRef()){
				delete pOldObserver;
			}
		}
		return *this;
	}

public:
	bool IsNonempty() const noexcept {
		return !!x_pObserver;
	}

	IntrusivePtr<ObjectT, DeleterT> Lock() const noexcept {
		const auto pObserver = x_pObserver;
		if(!pObserver){
			return nullptr;
		}
		const auto pOwner = pObserver->GetOwner();
		if(!pOwner){
			return nullptr;
		}
		return pOwner->template Share<ObjectT>();
	}

	IntrusiveWeakPtr &Reset(std::nullptr_t = nullptr) noexcept {
		return xInternalReset(nullptr);
	}
	IntrusiveWeakPtr &Reset(ElementType *pElement){
		if(!pElement){
			return Reset();
		}
		const auto pNewObserver = pElement->xCreateWeakObserver();
		pNewObserver->Impl_IntrusivePtr::RefCountBase::AddRef();
		return xInternalReset(pNewObserver);
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusiveWeakPtr &Reset(const IntrusivePtr<OtherT, OtherDeleterT> &rhs) noexcept {
		return Reset(rhs.Get());
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusiveWeakPtr &Reset(const IntrusiveWeakPtr<OtherT, OtherDeleterT> &rhs) noexcept {
		if(rhs.x_pObserver){
			rhs.x_pObserver->Impl_IntrusivePtr::RefCountBase::AddRef();
		}
		return xInternalReset(rhs.x_pObserver);
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	IntrusiveWeakPtr &Reset(IntrusiveWeakPtr<OtherT, OtherDeleterT> &&rhs) noexcept {
		return xInternalReset(std::exchange(rhs.x_pObserver, nullptr));
	}

	void Swap(IntrusiveWeakPtr &rhs) noexcept {
		std::swap(x_pObserver, rhs.x_pObserver);
	}
};

template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<const volatile OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() const volatile noexcept {
	const auto pObj = Impl_IntrusivePtr::StaticOrDynamicCast<const volatile OtherT *>(this);
	if(!pObj){
		return nullptr;
	}
	pObj->Impl_IntrusivePtr::RefCountBase::AddRef();
	return IntrusivePtr<const volatile OtherT, DeleterT>(pObj);
}
template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<const OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() const noexcept {
	const auto pObj = Impl_IntrusivePtr::StaticOrDynamicCast<const OtherT *>(this);
	if(!pObj){
		return nullptr;
	}
	pObj->Impl_IntrusivePtr::RefCountBase::AddRef();
	return IntrusivePtr<const OtherT, DeleterT>(pObj);
}
template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<volatile OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() volatile noexcept {
	const auto pObj = Impl_IntrusivePtr::StaticOrDynamicCast<volatile OtherT *>(this);
	if(!pObj){
		return nullptr;
	}
	pObj->Impl_IntrusivePtr::RefCountBase::AddRef();
	return IntrusivePtr<volatile OtherT, DeleterT>(pObj);
}
template<typename ObjectT, class DeleterT>
	template<typename OtherT>
IntrusivePtr<OtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::Share() noexcept {
	const auto pObj = Impl_IntrusivePtr::StaticOrDynamicCast<OtherT *>(this);
	if(!pObj){
		return nullptr;
	}
	pObj->Impl_IntrusivePtr::RefCountBase::AddRef();
	return IntrusivePtr<OtherT, DeleterT>(pObj);
}

template<typename ObjectT, typename ...ParamsT>
IntrusivePtr<ObjectT, DefaultDeleter<ObjectT>> MakeIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

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
*/
}

#endif
