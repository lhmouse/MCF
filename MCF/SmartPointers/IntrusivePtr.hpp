// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_

#include "../Utilities/Assert.hpp"
#include "../Thread/_SpinLock.hpp"
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
			for(;;){
				if(uOldRef == 0){
					return false;
				}
				if(AtomicCompareExchange(x_uRef, uOldRef, uOldRef + 1, MemoryModel::kRelaxed)){
					return true;
				}
			}
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

	template<typename DstT, typename SrcT, typename = void>
	struct StaticCastOrDynamicCastHelper {
		DstT operator()(SrcT &&vSrc) const {
			return dynamic_cast<DstT>(std::forward<SrcT>(vSrc));
		}
	};
	template<typename DstT, typename SrcT>
	struct StaticCastOrDynamicCastHelper<DstT, SrcT,
		decltype(static_cast<void>(static_cast<DstT>(std::declval<SrcT>())))>
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
		class xWeakObserver : public RefCountBase {
		private:
			mutable SpinLock x_splOwnerMutex;
			DeletableBase *x_pOwner;

		public:
			explicit xWeakObserver(DeletableBase *pOwner) noexcept
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
		mutable xWeakObserver *volatile x_pObserver;

	public:
		DeletableBase() noexcept {
			AtomicStore(x_pObserver, nullptr, MemoryModel::kRelease);
		}
		DeletableBase(const DeletableBase &) noexcept
			: DeletableBase()
		{
		}
		DeletableBase &operator=(const DeletableBase &) noexcept {
			return *this;
		}
		virtual ~DeletableBase(){
			const auto pObserver = AtomicLoad(x_pObserver, MemoryModel::kConsume);
			if(pObserver){
				if(static_cast<const volatile RefCountBase *>(pObserver)->DropRef()){
					delete pObserver;
				} else {
					pObserver->ClearOwner();
				}
			}
		}

	private:
		xWeakObserver *xCreateObserver() const volatile {
			auto pObserver = AtomicLoad(x_pObserver, MemoryModel::kConsume);
			if(!pObserver){
				const auto pNewObserver = new xWeakObserver(const_cast<DeletableBase *>(this));
				if(AtomicCompareExchange(x_pObserver, pObserver, pNewObserver, MemoryModel::kAcqRel, MemoryModel::kConsume)){
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
	template<typename, class>
	friend class IntrusivePtr;
	template<typename, class>
	friend class IntrusiveWeakPtr;

private:
	template<typename CvOtherT, typename CvThisT>
	static IntrusivePtr<CvOtherT, DeleterT> xForkShared(CvThisT *pThis) noexcept;
	template<typename CvOtherT, typename CvThisT>
	static IntrusiveWeakPtr<CvOtherT, DeleterT> xForkWeak(CvThisT *pThis);

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

	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const volatile OtherT, DeleterT> Weaken() const volatile {
		return xForkWeak<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const OtherT, DeleterT> Weaken() const {
		return xForkWeak<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<volatile OtherT, DeleterT> Weaken() volatile {
		return xForkWeak<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<OtherT, DeleterT> Weaken(){
		return xForkWeak<OtherT>(this);
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
			std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusivePtr(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
		: IntrusivePtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusivePtr(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs) noexcept
		: IntrusivePtr()
	{
		Reset(rhs);
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusivePtr(IntrusivePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
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
	std::size_t GetRefCount() const noexcept {
		return static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(x_pElement)->GetRef();
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
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusivePtr &Reset(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		return Reset(static_cast<Element *>(rhs.Release()));
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusivePtr &Reset(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs) noexcept {
		const auto pElement = rhs.Get();
		if(pElement == x_pElement){
			return *this;
		}
		if(pElement){
			static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pElement)->AddRef();
		}
		return Reset(static_cast<Element *>(pElement));
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusivePtr &Reset(IntrusivePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		return Reset(static_cast<Element *>(rhs.Release()));
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

namespace Impl_IntrusivePtr {
	template<class DeleterT>
		template<typename OtherT>
	IntrusivePtr<OtherT, DeleterT> DeletableBase<DeleterT>::xWeakObserver::GetOwner() const noexcept {
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
IntrusivePtr<CvOtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::xForkShared(CvThisT *pThis) noexcept {
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
public:
	static_assert(!std::is_array<ObjectT>::value, "Intrusive pointers don't accept arrays.");

	using Element = ObjectT;
	using Deleter = DeleterT;

	static_assert(noexcept(Deleter()(std::declval<std::remove_cv_t<Element> *>())), "Deleter must not throw.");

private:
	typename Impl_IntrusivePtr::DeletableBase<DeleterT>::xWeakObserver *x_pObserver;

public:
	constexpr IntrusiveWeakPtr(std::nullptr_t = nullptr) noexcept
		: x_pObserver(nullptr)
	{
	}
	explicit IntrusiveWeakPtr(Element *rhs)
		: IntrusiveWeakPtr()
	{
		Reset(rhs);
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusivePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusiveWeakPtr(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs)
		: IntrusiveWeakPtr()
	{
		Reset(rhs);
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusiveWeakPtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusiveWeakPtr(const IntrusiveWeakPtr<OtherObjectT, OtherDeleterT> &rhs) noexcept
		: IntrusiveWeakPtr()
	{
		Reset(rhs);
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename IntrusiveWeakPtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename IntrusiveWeakPtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusiveWeakPtr(IntrusiveWeakPtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
		: IntrusiveWeakPtr()
	{
		Reset(std::move(rhs));
	}
	IntrusiveWeakPtr(const IntrusiveWeakPtr &rhs) noexcept
		: IntrusiveWeakPtr()
	{
		Reset(rhs);
	}
	IntrusiveWeakPtr(IntrusiveWeakPtr &&rhs) noexcept
		: IntrusiveWeakPtr()
	{
		Reset(std::move(rhs));
	}
	IntrusiveWeakPtr &operator=(const IntrusiveWeakPtr &rhs) noexcept {
		return Reset(rhs);
	}
	IntrusiveWeakPtr &operator=(IntrusiveWeakPtr &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~IntrusiveWeakPtr(){
		Reset();
	}

private:
	IntrusiveWeakPtr &xResetObserver(typename IntrusiveBase<ObjectT, DeleterT>::xWeakObserver *pObserver) noexcept {
		const auto pOldObserver = std::exchange(x_pObserver, pObserver);
		if(pOldObserver){
			if(static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pOldObserver)->DropRef()){
				delete pOldObserver;
			}
		}
		return *this;
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

	IntrusiveWeakPtr &Reset(Element *pElement = nullptr){
		auto pObserver = static_cast<decltype(x_pObserver)>(nullptr);
		const auto pBase = static_cast<const volatile Impl_IntrusivePtr::DeletableBase<DeleterT> *>(pElement);
		if(pBase){
			pObserver = pBase->xCreateObserver();
			static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pObserver)->AddRef();
		}
		return xResetObserver(pObserver);
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusiveWeakPtr &Reset(const IntrusivePtr<OtherObjectT, OtherDeleterT> &rhs){
		return Reset(rhs.Get());
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusiveWeakPtr &Reset(const IntrusiveWeakPtr<OtherObjectT, OtherDeleterT> &rhs) noexcept {
		const auto pObserver = rhs.x_pObserver;
		if(pObserver == x_pObserver){
			return *this;
		}
		if(pObserver){
			static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pObserver)->AddRef();
		}
		return xResetObserver(pObserver);
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusiveWeakPtr &Reset(IntrusiveWeakPtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		return xResetObserver(std::exchange(rhs.x_pObserver, nullptr));
	}

	void Swap(IntrusiveWeakPtr &rhs) noexcept {
		std::swap(x_pObserver, rhs.x_pObserver);
	}

public:
	template<typename ObjectRhsT>
	bool operator==(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::equal_to<void>(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator!=(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::not_equal_to<void>(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator<(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::less<void>(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator>(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::greater<void>(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator<=(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::less_equal<void>(x_pObserver, rhs.x_pObserver);
	}
	template<typename ObjectRhsT>
	bool operator>=(const IntrusiveWeakPtr<ObjectRhsT, DeleterT> &rhs) const noexcept {
		return std::greater_equal<void>(x_pObserver, rhs.x_pObserver);
	}
};

template<typename ObjectT, class DeleterT>
	template<typename CvOtherT, typename CvThisT>
IntrusiveWeakPtr<CvOtherT, DeleterT> IntrusiveBase<ObjectT, DeleterT>::xForkWeak(CvThisT *pThis){
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

}

#endif
