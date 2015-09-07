// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/Bail.hpp"
#include "../Thread/SpinLock.hpp"
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
		mutable Atomic<std::size_t> x_uRef;

	protected:
		RefCountBase() noexcept
			: x_uRef(1)
		{
		}
		RefCountBase(const RefCountBase &) noexcept
			: RefCountBase() // 默认构造。
		{
		}
		RefCountBase &operator=(const RefCountBase &) noexcept {
			return *this; // 无操作。
		}
		~RefCountBase(){
			if(x_uRef.Load(kAtomicRelaxed) > 1){
				Bail(L"析构正在共享的被引用计数管理的对象。");
			}
		}

	public:
		std::size_t GetRef() const volatile noexcept {
			return x_uRef.Load(kAtomicRelaxed);
		}
		bool TryAddRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)x_uRef.Load(kAtomicRelaxed) >= 0);

			auto uOldRef = x_uRef.Load(kAtomicRelaxed);
			for(;;){
				if(uOldRef == 0){
					return false;
				}
				if(x_uRef.CompareExchange(uOldRef, uOldRef + 1, kAtomicRelaxed)){
					return true;
				}
			}
		}
		void AddRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)x_uRef.Load(kAtomicRelaxed) > 0);

			x_uRef.Increment(kAtomicRelaxed);
		}
		bool DropRef() const volatile noexcept {
			ASSERT((std::ptrdiff_t)x_uRef.Load(kAtomicRelaxed) > 0);

			return x_uRef.Decrement(kAtomicRelaxed) == 0;
		}
	};

	template<typename DstT, typename SrcT, typename = DstT>
	struct StaticCastOrDynamicCastHelper {
		DstT operator()(SrcT &&vSrc) const {
			return dynamic_cast<DstT>(std::forward<SrcT>(vSrc));
		}
	};
	template<typename DstT, typename SrcT>
	struct StaticCastOrDynamicCastHelper<DstT, SrcT,
		decltype(static_cast<DstT>(std::declval<SrcT>()))>
	{
		constexpr DstT operator()(SrcT &&vSrc) const noexcept {
			return static_cast<DstT>(std::forward<SrcT>(vSrc));
		}
	};

	template<typename DstT, typename SrcT>
	DstT StaticCastOrDynamicCast(SrcT &&vSrc){
		return StaticCastOrDynamicCastHelper<DstT, SrcT>()(std::forward<SrcT>(vSrc));
	}

	template<class DeleterT>
	class DeletableBase : public RefCountBase {
		template<typename, class>
		friend class IntrusivePtr;
		template<typename, class>
		friend class IntrusiveWeakPtr;

	private:
		class X_WeakObserver : public RefCountBase {
		private:
			mutable SpinLock x_splOwnerMutex;
			DeletableBase *x_pOwner;

		public:
			explicit X_WeakObserver(DeletableBase *pOwner) noexcept
				: x_pOwner(pOwner)
			{
			}

		public:
			bool HasOwnerExpired() const noexcept {
				const auto uLocked = x_splOwnerMutex.Lock();
				auto pOwner = x_pOwner;
				if(pOwner){
					if(static_cast<const volatile RefCountBase *>(pOwner)->GetRef() == 0){
						pOwner = nullptr;
					}
				}
				x_splOwnerMutex.Unlock(uLocked);
				return !pOwner;
			}
			void ClearOwner() noexcept {
				const auto uLocked = x_splOwnerMutex.Lock();
				x_pOwner = nullptr;
				x_splOwnerMutex.Unlock(uLocked);
			}

			template<typename OtherT>
			IntrusivePtr<OtherT, DeleterT> GetOwner() const noexcept;
		};

	private:
		mutable Atomic<X_WeakObserver *> x_pObserver;

	public:
		DeletableBase() noexcept {
			x_pObserver.Store(nullptr, kAtomicRelease);
		}
		DeletableBase(const DeletableBase &) noexcept
			: DeletableBase()
		{
		}
		DeletableBase &operator=(const DeletableBase &) noexcept {
			return *this;
		}
		~DeletableBase(){
			const auto pObserver = x_pObserver.Load(kAtomicConsume);
			if(pObserver){
				if(static_cast<const volatile RefCountBase *>(pObserver)->DropRef()){
					delete pObserver;
				} else {
					pObserver->ClearOwner();
				}
			}
		}

	private:
		X_WeakObserver *X_CreateObserver() const volatile {
			auto pObserver = x_pObserver.Load(kAtomicConsume);
			if(!pObserver){
				const auto pNewObserver = new X_WeakObserver(const_cast<DeletableBase *>(this));
				if(x_uRef.CompareExchange(pObserver, pNewObserver, kAtomicAcqRel, kAtomicConsume)){
					pObserver = pNewObserver;
				} else {
					delete pNewObserver;
				}
			}
			return pObserver;
		}
	};
}

template<typename ObjectT, class DeleterT>
class IntrusiveBase : public Impl_IntrusivePtr::DeletableBase<DeleterT> {
	static_assert(!std::is_array<ObjectT>::value, "IntrusiveBase doesn't accept arrays.");

	template<typename, class>
	friend class IntrusivePtr;
	template<typename, class>
	friend class IntrusiveWeakPtr;

private:
	template<typename CvOtherT, typename CvThisT>
	static IntrusivePtr<CvOtherT, DeleterT> X_ForkShared(CvThisT *pThis) noexcept;
	template<typename CvOtherT, typename CvThisT>
	static IntrusiveWeakPtr<CvOtherT, DeleterT> X_ForkWeak(CvThisT *pThis);

public:
	template<typename OtherT = ObjectT>
	IntrusivePtr<const volatile OtherT, DeleterT> Share() const volatile noexcept {
		return X_ForkShared<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<const OtherT, DeleterT> Share() const noexcept {
		return X_ForkShared<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<volatile OtherT, DeleterT> Share() volatile noexcept {
		return X_ForkShared<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<OtherT, DeleterT> Share() noexcept {
		return X_ForkShared<OtherT>(this);
	}

	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const volatile OtherT, DeleterT> Weaken() const volatile {
		return X_ForkWeak<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const OtherT, DeleterT> Weaken() const {
		return X_ForkWeak<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<volatile OtherT, DeleterT> Weaken() volatile {
		return X_ForkWeak<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<OtherT, DeleterT> Weaken(){
		return X_ForkWeak<OtherT>(this);
	}
};

template<typename ObjectT, class DeleterT>
class IntrusivePtr {
	static_assert(sizeof(IntrusiveBase<ObjectT, DeleterT>) > 0, "IntrusiveBase<ObjectT, DeleterT> is not an object type or is an incomplete type.");
	static_assert(sizeof(dynamic_cast<const volatile IntrusiveBase<ObjectT, DeleterT> *>(std::declval<ObjectT *>())), "Unable to locate IntrusiveBase for the managed object type.");

public:
	using Element = ObjectT;
	using Deleter = DeleterT;

	static_assert(noexcept(Deleter()(std::declval<std::remove_cv_t<Element> *>())), "Deleter must not throw.");

public:
	static const IntrusivePtr kNull;

private:
	Element *x_pElement;

public:
	constexpr IntrusivePtr(std::nullptr_t = nullptr) noexcept
		: x_pElement(nullptr)
	{
	}
	explicit IntrusivePtr(Element *rhs) noexcept
		: x_pElement(rhs)
	{
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusivePtr(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
		: x_pElement(rhs.Release())
	{
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusivePtr(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs) noexcept
		: x_pElement(rhs.Get())
	{
		if(x_pElement){
			static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(x_pElement)->AddRef();
		}
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusivePtr(IntrusivePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
		: x_pElement(rhs.x_pElement)
	{
		rhs.x_pElement = nullptr;
	}
	IntrusivePtr(const IntrusivePtr &rhs) noexcept
		: x_pElement(rhs.Get())
	{
		if(x_pElement){
			static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(x_pElement)->AddRef();
		}
	}
	IntrusivePtr(IntrusivePtr &&rhs) noexcept
		: x_pElement(rhs.x_pElement)
	{
		rhs.x_pElement = nullptr;
	}
	IntrusivePtr &operator=(const IntrusivePtr &rhs) noexcept {
		return Reset(rhs);
	}
	IntrusivePtr &operator=(IntrusivePtr &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~IntrusivePtr(){
		if(x_pElement){
			if(static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(x_pElement)->DropRef()){
				Deleter()(const_cast<std::remove_cv_t<Element> *>(x_pElement));
			}
		}
	}

public:
	bool IsNonnull() const noexcept {
		return !!x_pElement;
	}
	std::size_t GetRefCount() const noexcept {
		return static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(x_pElement)->GetRef();
	}
	Element *Get() const noexcept {
		return x_pElement;
	}
	Element *Release() noexcept {
		return std::exchange(x_pElement, nullptr);
	}

	IntrusivePtr &Reset(std::nullptr_t = nullptr) noexcept {
		IntrusivePtr().Swap(*this);
		return *this;
	}
	IntrusivePtr &Reset(Element *rhs) noexcept {
		IntrusivePtr(rhs).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusivePtr &Reset(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		IntrusivePtr(std::move(rhs)).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusivePtr &Reset(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs) noexcept {
		IntrusivePtr(rhs).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusivePtr &Reset(IntrusivePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		IntrusivePtr(std::move(rhs)).Swap(*this);
		return *this;
	}
	IntrusivePtr &Reset(const IntrusivePtr &rhs) noexcept {
		IntrusivePtr(rhs).Swap(*this);
		return *this;
	}
	IntrusivePtr &Reset(IntrusivePtr &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
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
const IntrusivePtr<ObjectT, DeleterT> IntrusivePtr<ObjectT, DeleterT>::kNull;

namespace Impl_IntrusivePtr {
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<OtherT, DeleterT> DeletableBase<DeleterT>::X_WeakObserver::GetOwner() const noexcept {
		const auto uLocked = x_splOwnerMutex.Lock();
		auto pOther = StaticCastOrDynamicCast<OtherT *>(x_pOwner);
		if(pOther){
			if(!static_cast<const volatile RefCountBase *>(pOther)->TryAddRef()){
				pOther = nullptr;
			}
		}
		x_splOwnerMutex.Unlock(uLocked);
		return IntrusivePtr<OtherT, DeleterT>(pOther);
	}
}

template<typename ObjectT, class DeleterT>
	template<typename CvOtherT, typename CvThisT>
IntrusivePtr<CvOtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::X_ForkShared(CvThisT *pThis) noexcept {
	const auto pOther = Impl_IntrusivePtr::StaticCastOrDynamicCast<CvOtherT *>(pThis);
	if(!pOther){
		return nullptr;
	}
	static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pThis)->AddRef();
	return IntrusivePtr<CvOtherT, DeleterT>(pOther);
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator==(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::equal_to<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator==(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, typename IntrusivePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return std::equal_to<void>()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator==(typename IntrusivePtr<ObjectLhsT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::equal_to<void>()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator!=(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::not_equal_to<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator!=(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, typename IntrusivePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return std::not_equal_to<void>()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator!=(typename IntrusivePtr<ObjectLhsT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::not_equal_to<void>()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::less<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, typename IntrusivePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return std::less<void>()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<(typename IntrusivePtr<ObjectLhsT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::less<void>()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::greater<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, typename IntrusivePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return std::greater<void>()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>(typename IntrusivePtr<ObjectLhsT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::greater<void>()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<=(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::less_equal<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<=(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, typename IntrusivePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return std::less_equal<void>()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<=(typename IntrusivePtr<ObjectLhsT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::less_equal<void>()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>=(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return std::greater_equal<void>()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>=(const IntrusivePtr<ObjectLhsT, DeleterT> &lhs, typename IntrusivePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return std::greater_equal<void>()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>=(typename IntrusivePtr<ObjectLhsT, DeleterT>::Element *lhs, const IntrusivePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
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

template<typename ObjectT, class DeleterT>
class IntrusiveWeakPtr {
	static_assert(sizeof(IntrusiveBase<ObjectT, DeleterT>) > 0, "IntrusiveBase<ObjectT, DeleterT> is not an object type or is an incomplete type.");
	static_assert(sizeof(dynamic_cast<const volatile IntrusiveBase<ObjectT, DeleterT> *>(std::declval<ObjectT *>())), "Unable to locate IntrusiveBase for the managed object type.");

public:
	using Element = ObjectT;
	using Deleter = DeleterT;

	static_assert(noexcept(Deleter()(std::declval<std::remove_cv_t<Element> *>())), "Deleter must not throw.");

public:
	static const IntrusiveWeakPtr kNull;

private:
	typename Impl_IntrusivePtr::DeletableBase<DeleterT>::X_WeakObserver *x_pObserver;

public:
	constexpr IntrusiveWeakPtr(std::nullptr_t = nullptr) noexcept
		: x_pObserver(nullptr)
	{
	}
	explicit IntrusiveWeakPtr(Element *rhs)
		: x_pObserver(nullptr)
	{
		const auto pBase = static_cast<const volatile Impl_IntrusivePtr::DeletableBase<DeleterT> *>(rhs);
		if(pBase){
			x_pObserver = pBase->X_CreateObserver();
			static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(x_pObserver)->AddRef();
		}
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusiveWeakPtr(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs)
		: IntrusiveWeakPtr(rhs.Get())
	{
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusiveWeakPtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusiveWeakPtr(const IntrusiveWeakPtr<OtherObjectT, OtherDeleterT> &rhs) noexcept
		: x_pObserver(rhs.x_pObserver)
	{
		if(x_pObserver){
			static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(x_pObserver)->AddRef();
		}
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusiveWeakPtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusiveWeakPtr(IntrusiveWeakPtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
		: x_pObserver(rhs.x_pObserver)
	{
		rhs.x_pObserver = nullptr;
	}
	IntrusiveWeakPtr(const IntrusiveWeakPtr &rhs) noexcept
		: x_pObserver(rhs.x_pObserver)
	{
		if(x_pObserver){
			static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(x_pObserver)->AddRef();
		}
	}
	IntrusiveWeakPtr(IntrusiveWeakPtr &&rhs) noexcept
		: x_pObserver(rhs.x_pObserver)
	{
		rhs.x_pObserver = nullptr;
	}
	IntrusiveWeakPtr &operator=(const IntrusiveWeakPtr &rhs) noexcept {
		return Reset(rhs);
	}
	IntrusiveWeakPtr &operator=(IntrusiveWeakPtr &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~IntrusiveWeakPtr(){
		const auto pOldObserver = x_pObserver;
		if(pOldObserver){
			if(static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pOldObserver)->DropRef()){
				delete pOldObserver;
			}
		}
	}

public:
	bool HasExpired() const noexcept {
		if(!x_pObserver){
			return true;
		}
		return x_pObserver->HasOwnerExpired();
	}
	IntrusivePtr<ObjectT, DeleterT> Lock() const noexcept {
		if(!x_pObserver){
			return nullptr;
		}
		return x_pObserver->template GetOwner<ObjectT>();
	}

	IntrusiveWeakPtr &Reset(std::nullptr_t = nullptr) noexcept {
		IntrusiveWeakPtr().Swap(*this);
		return *this;
	}
	IntrusiveWeakPtr &Reset(Element *rhs){
		IntrusiveWeakPtr(rhs).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusiveWeakPtr &Reset(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs){
		IntrusiveWeakPtr(rhs).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusiveWeakPtr &Reset(const IntrusiveWeakPtr<OtherObjectT, OtherDeleterT> &rhs) noexcept {
		IntrusiveWeakPtr(rhs).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusiveWeakPtr &Reset(IntrusiveWeakPtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		IntrusiveWeakPtr(std::move(rhs)).Swap(*this);
		return *this;
	}
	IntrusiveWeakPtr &Reset(const IntrusiveWeakPtr &rhs) noexcept {
		IntrusiveWeakPtr(rhs).Swap(*this);
		return *this;
	}
	IntrusiveWeakPtr &Reset(IntrusiveWeakPtr &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}

	void Swap(IntrusiveWeakPtr &rhs) noexcept {
		std::swap(x_pObserver, rhs.x_pObserver);
	}

public:
	template<typename ObjectRhsT>
	bool operator==(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::equal_to<void>()(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator!=(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::not_equal_to<void>()(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator<(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::less<void>()(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator>(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::greater<void>()(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator<=(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::less_equal<void>()(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator>=(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::greater_equal<void>()(x_pObserver, rhs.x_pObserver);
	}
};

template<typename ObjectT, class DeleterT>
const IntrusiveWeakPtr<ObjectT, DeleterT> IntrusiveWeakPtr<ObjectT, DeleterT>::kNull;

template<typename ObjectT, class DeleterT>
	template<typename CvOtherT, typename CvThisT>
IntrusiveWeakPtr<CvOtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::X_ForkWeak(CvThisT *pThis){
	const auto pOther = Impl_IntrusivePtr::StaticCastOrDynamicCast<CvOtherT *>(pThis);
	if(!pOther){
		return nullptr;
	}
	return IntrusiveWeakPtr<CvOtherT, DeleterT>(pOther);
}

template<typename ObjectT, class DeleterT>
void swap(IntrusiveWeakPtr<ObjectT, DeleterT> &lhs, IntrusiveWeakPtr<ObjectT, DeleterT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ObjectT, class DeleterT>
IntrusiveWeakPtr<ObjectT, DeleterT> Weaken(const IntrusivePtr<ObjectT, DeleterT> &rhs) noexcept {
	return IntrusiveWeakPtr<ObjectT, DeleterT>(rhs);
}

}

#endif
