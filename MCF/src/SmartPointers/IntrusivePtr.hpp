// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_INTRUSIVE_PTR_HPP_

#include "../Core/Assert.hpp"
#include "../Core/Bail.hpp"
#include "../Core/Atomic.hpp"
#include "../Thread/Mutex.hpp"
#include "DefaultDeleter.hpp"
#include <utility>
#include <type_traits>
#include <typeinfo>
#include <cstddef>
#include <cstdint>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::decay_t<ObjectT>>>
class IntrusiveBase;

template<typename ObjectT>
class IntrusivePtr;
template<typename ObjectT>
class IntrusiveWeakPtr;

template<typename ObjectT, class DeleterT>
class UniquePtr;

namespace Impl_IntrusivePtr {
	class RefCountBase {
	private:
		mutable Atomic<std::size_t> x_uRef;

	protected:
		constexpr RefCountBase() noexcept
			: x_uRef(1)
		{
		}
		constexpr RefCountBase(const RefCountBase &) noexcept
			: RefCountBase() // 默认构造。
		{
		}
		RefCountBase &operator=(const RefCountBase &) noexcept {
			return *this; // 无操作。
		}
		~RefCountBase(){
			if(x_uRef.Load(kAtomicRelaxed) > 1){
				Bail(L"试图析构正在共享的被引用计数管理的对象。");
			}
		}

	public:
		bool IsUnique() const volatile noexcept {
			return x_uRef.Load(kAtomicRelaxed) == 1;
		}
		std::size_t GetRef() const volatile noexcept {
			return x_uRef.Load(kAtomicRelaxed);
		}
		bool TryAddRef() const volatile noexcept {
			MCF_DEBUG_CHECK(static_cast<std::ptrdiff_t>(x_uRef.Load(kAtomicRelaxed)) >= 0);

			auto uOldRef = x_uRef.Load(kAtomicRelaxed);
			do {
				if(uOldRef == 0){
					return false;
				}
			} while(!x_uRef.CompareExchange(uOldRef, uOldRef + 1, kAtomicRelaxed));
			return true;
		}
		void AddRef() const volatile noexcept {
			MCF_DEBUG_CHECK(static_cast<std::ptrdiff_t>(x_uRef.Load(kAtomicRelaxed)) > 0);

			x_uRef.Increment(kAtomicRelaxed);
		}
		bool DropRef() const volatile noexcept {
			MCF_DEBUG_CHECK(static_cast<std::ptrdiff_t>(x_uRef.Load(kAtomicRelaxed)) > 0);

			return x_uRef.Decrement(kAtomicRelaxed) == 0;
		}
	};

	template<typename DstT, typename SrcT, typename = void>
	struct Helper_static_cast_or_dynamic_cast {
		constexpr decltype(auto) operator()(SrcT &vSrc) const {
			return dynamic_cast<DstT>(std::forward<SrcT>(vSrc));
		}
	};
	template<typename DstT, typename SrcT>
	struct Helper_static_cast_or_dynamic_cast<DstT, SrcT, decltype(static_cast<void>(static_cast<DstT>(std::declval<SrcT>())))> {
		constexpr decltype(auto) operator()(SrcT &vSrc) const {
			return static_cast<DstT>(std::forward<SrcT>(vSrc));
		}
	};

	template<typename DstT, typename SrcT>
	constexpr DstT Do_static_cast_or_dynamic_cast(SrcT &&vSrc){
		return Helper_static_cast_or_dynamic_cast<DstT, SrcT>()(vSrc);
	}

	template<typename ObjectT>
	class WeakViewTemplate : public RefCountBase  {
	private:
		mutable Mutex x_mtxGuard;
		ObjectT *x_pParent;

	public:
		explicit constexpr WeakViewTemplate(ObjectT *pParent) noexcept
			: x_mtxGuard(), x_pParent(pParent)
		{
		}

	public:
		bool IsParentExpired() const noexcept {
			const auto vLock = x_mtxGuard.GetLock();
			const auto pParent = x_pParent;
			if(!pParent){
				return true;
			}
			return pParent->RefCountBase::GetRef() == 0;
		}
		template<typename OtherT>
		IntrusivePtr<OtherT> LockParent() const noexcept {
			const auto vLock = x_mtxGuard.GetLock();
			const auto pOther = Do_static_cast_or_dynamic_cast<OtherT *>(x_pParent);
			if(!pOther){
				return nullptr;
			}
			if(!pOther->RefCountBase::TryAddRef()){
				return nullptr;
			}
			return IntrusivePtr<OtherT>(pOther);
		}
		void ClearParent() noexcept {
			const auto vLock = x_mtxGuard.GetLock();
			x_pParent = nullptr;
		}
	};
}

template<typename ObjectT, class DeleterT>
class IntrusiveBase : public Impl_IntrusivePtr::RefCountBase {
	template<typename>
	friend class IntrusivePtr;
	template<typename>
	friend class IntrusiveWeakPtr;

public:
	using Element = std::remove_extent_t<ObjectT>;
	using Deleter = DeleterT;

protected:
	template<typename CvOtherT, typename CvThisT>
	static IntrusivePtr<CvOtherT> Y_ForkStrong(CvThisT *pThis) noexcept {
		const auto pOther = Impl_IntrusivePtr::Do_static_cast_or_dynamic_cast<CvOtherT *>(pThis);
		if(!pOther){
			return nullptr;
		}
		pOther->Impl_IntrusivePtr::RefCountBase::AddRef();
		return IntrusivePtr<CvOtherT>(pOther);
	}
	template<typename CvOtherT, typename CvThisT>
	static IntrusiveWeakPtr<CvOtherT> Y_ForkWeak(CvThisT *pThis){
		const auto pOther = Impl_IntrusivePtr::Do_static_cast_or_dynamic_cast<CvOtherT *>(pThis);
		if(!pOther){
			return nullptr;
		}
		return IntrusiveWeakPtr<CvOtherT>(pOther);
	}

private:
	mutable Atomic<Impl_IntrusivePtr::WeakViewTemplate<ObjectT> *> x_pView;

public:
	constexpr IntrusiveBase() noexcept
		: Impl_IntrusivePtr::RefCountBase()
		, x_pView(nullptr)
	{
	}
	IntrusiveBase(const IntrusiveBase & /* pOther */) noexcept
		: IntrusiveBase()
	{
	}
	IntrusiveBase &operator=(const IntrusiveBase & /* pOther */) noexcept {
		return *this;
	}
	~IntrusiveBase(){
		const auto pView = x_pView.Load(kAtomicConsume);
		if(pView){
			if(pView->RefCountBase::DropRef()){
				delete pView;
			} else {
				pView->ClearParent();
			}
		}
	}

private:
	Impl_IntrusivePtr::WeakViewTemplate<ObjectT> *X_GetView() const volatile {
		auto pView = x_pView.Load(kAtomicConsume);
		return pView;
	}
	Impl_IntrusivePtr::WeakViewTemplate<ObjectT> *X_RequireView() const volatile {
		auto pView = x_pView.Load(kAtomicConsume);
		if(!pView){
			const auto pObject = Impl_IntrusivePtr::Do_static_cast_or_dynamic_cast<const volatile ObjectT *>(this);
			if(!pObject){
				throw std::bad_cast();
			}
			const auto pNewView = new Impl_IntrusivePtr::WeakViewTemplate<ObjectT>(const_cast<ObjectT *>(pObject));
			if(x_pView.CompareExchange(pView, pNewView, kAtomicRelease, kAtomicConsume)){
				pView = pNewView;
			} else {
				delete pNewView;
			}
		}
		return pView;
	}

public:
	void ReserveWeak() const volatile {
		X_RequireView();
	}

	template<typename OtherT = ObjectT>
	IntrusivePtr<const volatile OtherT> Share() const volatile noexcept {
		return Y_ForkStrong<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<const OtherT> Share() const noexcept {
		return Y_ForkStrong<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<volatile OtherT> Share() volatile noexcept {
		return Y_ForkStrong<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<OtherT> Share() noexcept {
		return Y_ForkStrong<OtherT>(this);
	}

	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const volatile OtherT> Weaken() const volatile {
		return Y_ForkWeak<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const OtherT> Weaken() const {
		return Y_ForkWeak<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<volatile OtherT> Weaken() volatile {
		return Y_ForkWeak<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<OtherT> Weaken(){
		return Y_ForkWeak<OtherT>(this);
	}
};

namespace Impl_IntrusivePtr {
	template<typename ObjectT, class DeleterT>
	const volatile IntrusiveBase<ObjectT, DeleterT> *LocateIntrusiveBase(const volatile IntrusiveBase<ObjectT, DeleterT> *pObject) noexcept {
		return pObject;
	}
}

template<typename ObjectT>
class IntrusivePtr {
	template<typename>
	friend class IntrusivePtr;
	template<typename>
	friend class IntrusiveWeakPtr;

public:
	using Element = std::remove_extent_t<ObjectT>;
	using Deleter = typename std::decay_t<decltype(*(Impl_IntrusivePtr::LocateIntrusiveBase(std::declval<ObjectT *>())))>::Deleter;

	static_assert(noexcept(Deleter()(std::declval<std::remove_cv_t<Element> *>())), "Deleter must not throw.");

private:
	Element *x_pElement;

private:
	Element *X_Fork() const noexcept {
		const auto pElement = x_pElement;
		if(pElement){
			pElement->Impl_IntrusivePtr::RefCountBase::AddRef();
		}
		return pElement;
	}

public:
	constexpr IntrusivePtr(std::nullptr_t = nullptr) noexcept
		: x_pElement(nullptr)
	{
	}
	explicit IntrusivePtr(Element *pElement) noexcept
		: x_pElement(pElement)
	{
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value && std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusivePtr(UniquePtr<OtherObjectT, OtherDeleterT> &&pOther) noexcept
		: x_pElement(pOther.Release())
	{
	}
	template<typename OtherObjectT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT>::Element *, Element *>::value && std::is_convertible<typename IntrusivePtr<OtherObjectT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusivePtr(const IntrusivePtr<OtherObjectT> &pOther) noexcept
		: x_pElement(pOther.X_Fork())
	{
	}
	template<typename OtherObjectT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT>::Element *, Element *>::value && std::is_convertible<typename IntrusivePtr<OtherObjectT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusivePtr(IntrusivePtr<OtherObjectT> &&pOther) noexcept
		: x_pElement(pOther.Release())
	{
	}
	IntrusivePtr(const IntrusivePtr &pOther) noexcept
		: x_pElement(pOther.X_Fork())
	{
	}
	IntrusivePtr(IntrusivePtr &&pOther) noexcept
		: x_pElement(pOther.Release())
	{
	}
	IntrusivePtr &operator=(const IntrusivePtr &pOther) noexcept {
		return Reset(pOther);
	}
	IntrusivePtr &operator=(IntrusivePtr &&pOther) noexcept {
		return Reset(std::move(pOther));
	}
	~IntrusivePtr(){
		const auto pElement = x_pElement;
#ifndef NDEBUG
		__builtin_memset(&x_pElement, 0xEF, sizeof(x_pElement));
#endif
		if(pElement){
			if(pElement->Impl_IntrusivePtr::RefCountBase::DropRef()){
				Deleter()(const_cast<std::remove_cv_t<Element> *>(pElement));
			}
		}
	}

public:
	constexpr bool IsNull() const noexcept {
		return !x_pElement;
	}
	constexpr Element *Get() const noexcept {
		return x_pElement;
	}
	Element *Release() noexcept {
		return std::exchange(x_pElement, nullptr);
	}
	bool IsUnique() const noexcept {
		const auto pElement = x_pElement;
		if(!pElement){
			return false;
		}
		return pElement->Impl_IntrusivePtr::RefCountBase::IsUnique();
	}
	std::size_t GetRef() const noexcept {
		const auto pElement = x_pElement;
		if(!pElement){
			return 0;
		}
		return pElement->Impl_IntrusivePtr::RefCountBase::GetRef();
	}
	std::size_t GetWeakRef() const noexcept {
		const auto pElement = x_pElement;
		if(!pElement){
			return 0;
		}
		const auto pView = Impl_IntrusivePtr::LocateIntrusiveBase(pElement)->X_GetView();
		if(!pView){
			return 0;
		}
		return pView->Impl_IntrusivePtr::RefCountBase::GetRef() - 1;
	}

	IntrusivePtr &Reset(std::nullptr_t = nullptr) noexcept {
		IntrusivePtr().Swap(*this);
		return *this;
	}
	IntrusivePtr &Reset(Element *pElement) noexcept {
		IntrusivePtr(pElement).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	IntrusivePtr &Reset(UniquePtr<OtherObjectT, OtherDeleterT> &&pOther) noexcept {
		IntrusivePtr(std::move(pOther)).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT>
	IntrusivePtr &Reset(const IntrusivePtr<OtherObjectT> &pOther) noexcept {
		IntrusivePtr(pOther).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT>
	IntrusivePtr &Reset(IntrusivePtr<OtherObjectT> &&pOther) noexcept {
		IntrusivePtr(std::move(pOther)).Swap(*this);
		return *this;
	}
	IntrusivePtr &Reset(const IntrusivePtr &pOther) noexcept {
		IntrusivePtr(pOther).Swap(*this);
		return *this;
	}
	IntrusivePtr &Reset(IntrusivePtr &&pOther) noexcept {
		IntrusivePtr(std::move(pOther)).Swap(*this);
		return *this;
	}

	void Swap(IntrusivePtr &pOther) noexcept {
		using std::swap;
		swap(x_pElement, pOther.x_pElement);
	}

public:
	explicit constexpr operator bool() const noexcept {
		return !IsNull();
	}
	explicit constexpr operator Element *() const noexcept {
		return Get();
	}

	constexpr Element &operator*() const noexcept {
		MCF_DEBUG_CHECK(!IsNull());

		return *Get();
	}
	constexpr Element *operator->() const noexcept {
		MCF_DEBUG_CHECK(!IsNull());

		return Get();
	}

	template<typename OtherObjectT>
	constexpr bool operator==(const IntrusivePtr<OtherObjectT> &pOther) const noexcept {
		return x_pElement == pOther.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator==(OtherObjectT *pOther) const noexcept {
		return x_pElement == pOther;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator==(OtherObjectT *pSelf, const IntrusivePtr &pOther) noexcept {
		return pSelf == pOther.x_pElement;
	}

	template<typename OtherObjectT>
	constexpr bool operator!=(const IntrusivePtr<OtherObjectT> &pOther) const noexcept {
		return x_pElement != pOther.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator!=(OtherObjectT *pOther) const noexcept {
		return x_pElement != pOther;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator!=(OtherObjectT *pSelf, const IntrusivePtr &pOther) noexcept {
		return pSelf != pOther.x_pElement;
	}

	template<typename OtherObjectT>
	constexpr bool operator<(const IntrusivePtr<OtherObjectT> &pOther) const noexcept {
		return x_pElement < pOther.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator<(OtherObjectT *pOther) const noexcept {
		return x_pElement < pOther;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator<(OtherObjectT *pSelf, const IntrusivePtr &pOther) noexcept {
		return pSelf < pOther.x_pElement;
	}

	template<typename OtherObjectT>
	constexpr bool operator>(const IntrusivePtr<OtherObjectT> &pOther) const noexcept {
		return x_pElement > pOther.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator>(OtherObjectT *pOther) const noexcept {
		return x_pElement > pOther;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator>(OtherObjectT *pSelf, const IntrusivePtr &pOther) noexcept {
		return pSelf > pOther.x_pElement;
	}

	template<typename OtherObjectT>
	constexpr bool operator<=(const IntrusivePtr<OtherObjectT> &pOther) const noexcept {
		return x_pElement <= pOther.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator<=(OtherObjectT *pOther) const noexcept {
		return x_pElement <= pOther;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator<=(OtherObjectT *pSelf, const IntrusivePtr &pOther) noexcept {
		return pSelf <= pOther.x_pElement;
	}

	template<typename OtherObjectT>
	constexpr bool operator>=(const IntrusivePtr<OtherObjectT> &pOther) const noexcept {
		return x_pElement >= pOther.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator>=(OtherObjectT *pOther) const noexcept {
		return x_pElement >= pOther;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator>=(OtherObjectT *pSelf, const IntrusivePtr &pOther) noexcept {
		return pSelf >= pOther.x_pElement;
	}

	friend void swap(IntrusivePtr &pSelf, IntrusivePtr &pOther) noexcept {
		pSelf.Swap(pOther);
	}
};

template<typename ObjectT, typename ...ParamsT>
IntrusivePtr<ObjectT> MakeIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return IntrusivePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT>
IntrusivePtr<DstT> StaticPointerCast(IntrusivePtr<SrcT> pSrc) noexcept {
	const auto pTest = static_cast<DstT *>(pSrc.Get());
	pSrc.Release();
	return IntrusivePtr<DstT>(pTest);
}
template<typename DstT, typename SrcT>
IntrusivePtr<DstT> DynamicPointerCast(IntrusivePtr<SrcT> pSrc) noexcept {
	const auto pTest = dynamic_cast<DstT *>(pSrc.Get());
	if(pTest){
		pSrc.Release();
	}
	return IntrusivePtr<DstT>(pTest);
}
template<typename DstT, typename SrcT>
IntrusivePtr<DstT> ConstPointerCast(IntrusivePtr<SrcT> pSrc) noexcept {
	const auto pTest = const_cast<DstT *>(pSrc.Get());
	pSrc.Release();
	return IntrusivePtr<DstT>(pTest);
}

template<typename ObjectT>
class IntrusiveWeakPtr {
	template<typename>
	friend class IntrusivePtr;
	template<typename>
	friend class IntrusiveWeakPtr;

public:
	using Element = typename IntrusivePtr<ObjectT>::Element;
	using Deleter = typename IntrusivePtr<ObjectT>::Deleter;

	static_assert(noexcept(Deleter()(std::declval<std::remove_cv_t<Element> *>())), "Deleter must not throw.");

private:
	Impl_IntrusivePtr::WeakViewTemplate<std::remove_cv_t<Element>> *x_pView;

private:
	static Impl_IntrusivePtr::WeakViewTemplate<std::remove_cv_t<Element>> *X_CreateViewFromElement(const volatile Element *pElement){
		if(!pElement){
			return nullptr;
		}
		const auto pView = Impl_IntrusivePtr::LocateIntrusiveBase(pElement)->X_RequireView();
		pView->Impl_IntrusivePtr::RefCountBase::AddRef();
		return pView;
	}

public:
	constexpr IntrusiveWeakPtr(std::nullptr_t = nullptr) noexcept
		: x_pView(nullptr)
	{
	}
	explicit IntrusiveWeakPtr(Element *pElement)
		: x_pView(X_CreateViewFromElement(pElement))
	{
	}
	template<typename OtherObjectT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT>::Element *, Element *>::value && std::is_convertible<typename IntrusivePtr<OtherObjectT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusiveWeakPtr(const IntrusivePtr<OtherObjectT> &pOther)
		: x_pView(X_CreateViewFromElement(pOther.Get()))
	{
	}
	template<typename OtherObjectT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT>::Element *, Element *>::value && std::is_convertible<typename IntrusivePtr<OtherObjectT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusiveWeakPtr(const IntrusiveWeakPtr<OtherObjectT> &pOther) noexcept
		: x_pView(pOther.X_Fork())
	{
	}
	template<typename OtherObjectT,
		std::enable_if_t<
			std::is_convertible<typename IntrusivePtr<OtherObjectT>::Element *, Element *>::value && std::is_convertible<typename IntrusivePtr<OtherObjectT>::Deleter, Deleter>::value,
			int> = 0>
	IntrusiveWeakPtr(IntrusiveWeakPtr<OtherObjectT> &&pOther) noexcept
		: x_pView(pOther.X_Release())
	{
	}
	IntrusiveWeakPtr(const IntrusiveWeakPtr &pOther) noexcept
		: x_pView(pOther.X_Fork())
	{
	}
	IntrusiveWeakPtr(IntrusiveWeakPtr &&pOther) noexcept
		: x_pView(pOther.X_Release())
	{
	}
	IntrusiveWeakPtr &operator=(const IntrusiveWeakPtr &pOther) noexcept {
		return Reset(pOther);
	}
	IntrusiveWeakPtr &operator=(IntrusiveWeakPtr &&pOther) noexcept {
		return Reset(std::move(pOther));
	}
	~IntrusiveWeakPtr(){
		const auto pView = x_pView;
#ifndef NDEBUG
		__builtin_memset(&x_pView, 0xDC, sizeof(x_pView));
#endif
		if(pView){
			if(pView->Impl_IntrusivePtr::RefCountBase::DropRef()){
				delete pView;
			}
		}
	}

private:
	Impl_IntrusivePtr::WeakViewTemplate<std::remove_cv_t<Element>> *X_Fork() const noexcept {
		const auto pView = x_pView;
		if(!pView){
			return nullptr;
		}
		pView->Impl_IntrusivePtr::RefCountBase::AddRef();
		return pView;
	}
	Impl_IntrusivePtr::WeakViewTemplate<std::remove_cv_t<Element>> *X_Release() noexcept {
		return std::exchange(x_pView, nullptr);
	}

public:
	constexpr bool IsEmpty() const noexcept {
		return !x_pView;
	}
	constexpr bool IsExpired() const noexcept {
		const auto pView = x_pView;
		if(!pView){
			return true;
		}
		return pView->IsParentExpired();
	}
	std::size_t GetWeakRef() const noexcept {
		const auto pView = x_pView;
		if(!pView){
			return 0;
		}
		return pView->Impl_IntrusivePtr::RefCountBase::GetRef() - 1;
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<OtherT> Lock() const noexcept {
		const auto pView = x_pView;
		if(!pView){
			return nullptr;
		}
		return pView->template LockParent<OtherT>();
	}

	IntrusiveWeakPtr &Reset(std::nullptr_t = nullptr) noexcept {
		IntrusiveWeakPtr().Swap(*this);
		return *this;
	}
	IntrusiveWeakPtr &Reset(Element *pElement){
		IntrusiveWeakPtr(pElement).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT>
	IntrusiveWeakPtr &Reset(const IntrusivePtr<OtherObjectT> &pOther){
		IntrusiveWeakPtr(pOther).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT>
	IntrusiveWeakPtr &Reset(const IntrusiveWeakPtr<OtherObjectT> &pOther) noexcept {
		IntrusiveWeakPtr(pOther).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT>
	IntrusiveWeakPtr &Reset(IntrusiveWeakPtr<OtherObjectT> &&pOther) noexcept {
		IntrusiveWeakPtr(std::move(pOther)).Swap(*this);
		return *this;
	}
	IntrusiveWeakPtr &Reset(const IntrusiveWeakPtr &pOther) noexcept {
		IntrusiveWeakPtr(pOther).Swap(*this);
		return *this;
	}
	IntrusiveWeakPtr &Reset(IntrusiveWeakPtr &&pOther) noexcept {
		IntrusiveWeakPtr(std::move(pOther)).Swap(*this);
		return *this;
	}

	void Swap(IntrusiveWeakPtr &pOther) noexcept {
		using std::swap;
		swap(x_pView, pOther.x_pView);
	}

public:
	template<typename OtherObjectT>
	bool operator==(const IntrusiveWeakPtr<OtherObjectT> &pOther) const noexcept {
		return x_pView == pOther.x_pView;
	}
	template<typename OtherObjectT>
	bool operator!=(const IntrusiveWeakPtr<OtherObjectT> &pOther) const noexcept {
		return x_pView != pOther.x_pView;
	}
	template<typename OtherObjectT>
	bool operator<(const IntrusiveWeakPtr<OtherObjectT> &pOther) const noexcept {
		return x_pView < pOther.x_pView;
	}
	template<typename OtherObjectT>
	bool operator>(const IntrusiveWeakPtr<OtherObjectT> &pOther) const noexcept {
		return x_pView > pOther.x_pView;
	}
	template<typename OtherObjectT>
	bool operator<=(const IntrusiveWeakPtr<OtherObjectT> &pOther) const noexcept {
		return x_pView <= pOther.x_pView;
	}
	template<typename OtherObjectT>
	bool operator>=(const IntrusiveWeakPtr<OtherObjectT> &pOther) const noexcept {
		return x_pView >= pOther.x_pView;
	}

	friend void swap(IntrusiveWeakPtr &pSelf, IntrusiveWeakPtr &pOther) noexcept {
		pSelf.Swap(pOther);
	}
};

}

#endif
