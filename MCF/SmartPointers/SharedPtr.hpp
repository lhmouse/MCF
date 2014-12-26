// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_SHARED_PTR_HPP_
#define MCF_SMART_POINTERS_SHARED_PTR_HPP_

// 1) 构造函数（包含复制构造函数、转移构造函数以及构造函数模板）、赋值运算符和析构函数应当调用 Reset()；
// 2) Reset() 的形参若具有 SharedPtr 的某模板类类型，则禁止传值，必须传引用。

#include "../Utilities/Assert.hpp"
#include "../../MCFCRT/ext/expect.h"
#include "DefaultDeleter.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class SharedPtr;

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class WeakPtr;

namespace Impl {
	class SharedControl final {
	public:
		class Sentry {
		private:
			SharedControl *xm_pOwner;

		public:
			explicit constexpr Sentry(SharedControl *pOwner) noexcept
				: xm_pOwner(pOwner)
			{
			}
			Sentry(Sentry &&rhs) noexcept
				: xm_pOwner(std::exchange(rhs.xm_pOwner, nullptr))
			{
			}
			~Sentry(){
				if(xm_pOwner){
					delete xm_pOwner;
				}
			}

			Sentry(const Sentry &) = delete;
			Sentry &operator=(const Sentry &) = delete;
			Sentry &operator=(Sentry &&) noexcept = delete;
		};

	public:
		static void *operator new(std::size_t uSize);
		static void operator delete(void *pData) noexcept;

		static void *operator new[](std::size_t) = delete;
		static void operator delete[](void *) noexcept  = delete;

	private:
		void (*const xm_pfnDeleter)(void *);

		void *xm_pToDelete;
		volatile std::size_t xm_uSharedCount;
		volatile std::size_t xm_uWeakCount;

	public:
		SharedControl(void (*pfnDeleter)(void *), void *pData) noexcept
			: xm_pfnDeleter(pfnDeleter)
			, xm_pToDelete(pData), xm_uSharedCount(1), xm_uWeakCount(1)
		{
			__atomic_thread_fence(__ATOMIC_RELEASE);
		}

	private:
		~SharedControl(){
			ASSERT(GetShared() == 0);
			ASSERT(GetWeak() == 0);
		}

		SharedControl(const SharedControl &) = delete;
		SharedControl(SharedControl &&) noexcept = delete;
		SharedControl &operator=(const SharedControl &) = delete;
		SharedControl &operator=(SharedControl &&) noexcept = delete;

	public:
		std::size_t GetShared() const noexcept {
			return __atomic_load_n(&xm_uSharedCount, __ATOMIC_ACQUIRE);
		}
		void AddShared() noexcept {
			AddWeak();

			ASSERT(__atomic_load_n(&xm_uSharedCount, __ATOMIC_ACQUIRE) != 0);
			__atomic_add_fetch(&xm_uSharedCount, 1, __ATOMIC_RELEASE);
		}
		Sentry TryAddShared(bool &bResult) noexcept {
			AddWeak();

			auto uOldShared = __atomic_load_n(&xm_uSharedCount, __ATOMIC_ACQUIRE);
			for(;;){
				if(EXPECT_NOT(uOldShared == 0)){
					break;
				}
				if(EXPECT_NOT(__atomic_compare_exchange_n(&xm_uSharedCount, &uOldShared, uOldShared + 1,
					false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)))
				{
					goto jDone;
				}
			}

			bResult = false;
			return DropWeak();

		jDone:
			bResult = true;
			return Sentry(nullptr);
		}
		Sentry DropShared() noexcept {
			ASSERT(__atomic_load_n(&xm_uSharedCount, __ATOMIC_ACQUIRE) != 0);
			if(__atomic_sub_fetch(&xm_uSharedCount, 1, __ATOMIC_ACQUIRE) == 0){
				(*xm_pfnDeleter)(xm_pToDelete);
#ifndef NDEBUG
				xm_pToDelete = (void *)
#	ifdef _WIN64
					0xDEADDEADDEADDEAD
#	else
					0xDEADDEAD
#	endif
				;
#endif
			}

			return DropWeak();
		}

		std::size_t GetWeak() const noexcept {
			return __atomic_load_n(&xm_uWeakCount, __ATOMIC_ACQUIRE);
		}
		void AddWeak() noexcept {
			ASSERT(__atomic_load_n(&xm_uWeakCount, __ATOMIC_ACQUIRE) != 0);
			__atomic_add_fetch(&xm_uWeakCount, 1, __ATOMIC_RELEASE);
		}
		Sentry DropWeak() noexcept {
			ASSERT(__atomic_load_n(&xm_uWeakCount, __ATOMIC_ACQUIRE) != 0);
			SharedControl *pToDelete = nullptr;
			if(__atomic_sub_fetch(&xm_uWeakCount, 1, __ATOMIC_ACQUIRE) == 0){
				pToDelete = this;
			}
			return Sentry(pToDelete);
		}

		void *GetRaw() const noexcept {
			return xm_pToDelete;
		}
	};
}

template<typename ObjectT, class DeleterT>
class SharedPtr {
	template<typename, class>
	friend class SharedPtr;

	template<typename, class>
	friend class WeakPtr;

	static_assert(noexcept(DeleterT()(DeleterT()())), "Deleter must not throw.");

public:
	using Raw = std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<decltype(DeleterT()())>>>;
	using Element = std::remove_extent_t<ObjectT>;

private:
	Impl::SharedControl *xm_pControl;
	Element *xm_pElement;

public:
	constexpr SharedPtr() noexcept
		: xm_pControl(nullptr), xm_pElement(nullptr)
	{
	}
	template<typename TestRawT = Raw>
	SharedPtr(std::enable_if_t<std::is_convertible<TestRawT *, Element *>::value, TestRawT> *pRaw)
		: SharedPtr()
	{
		Reset(pRaw);
	}
	SharedPtr(Raw *pRaw, Element *pElement)
		: SharedPtr()
	{
		Reset(pRaw, pElement);
	}
	SharedPtr(const SharedPtr &rhs) noexcept
		: SharedPtr()
	{
		Reset(rhs);
	}
	SharedPtr(SharedPtr &&rhs) noexcept
		: SharedPtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherT,
		std::enable_if_t<std::is_array<OtherT>::value
			? std::is_same<std::remove_cv_t<std::remove_extent_t<OtherT>>, std::remove_cv_t<Element>>::value
			: std::is_convertible<OtherT *, Element *>::value,
		int> = 0>
	SharedPtr(SharedPtr<OtherT, DeleterT> rhs) noexcept
		: SharedPtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherT>
	SharedPtr(SharedPtr<OtherT, DeleterT> rhs, Element *pElement) noexcept
		: SharedPtr()
	{
		Reset(std::move(rhs), pElement);
	}
	SharedPtr &operator=(const SharedPtr &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	SharedPtr &operator=(SharedPtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	template<typename OtherT,
		std::enable_if_t<std::is_array<OtherT>::value
			? std::is_same<std::remove_cv_t<std::remove_extent_t<OtherT>>, std::remove_cv_t<Element>>::value
			: std::is_convertible<OtherT *, Element *>::value,
		int> = 0>
	SharedPtr &operator=(SharedPtr<OtherT, DeleterT> rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~SharedPtr(){
		Reset();
	}

public:
	bool IsOwning() const noexcept {
		return GetRaw() != nullptr;
	}
	Raw *GetRaw() const noexcept {
		return xm_pControl ? static_cast<Raw *>(xm_pControl->GetRaw()) : nullptr;
	}
	bool IsNonnull() const noexcept {
		return Get() != nullptr;
	}
	Element *Get() const noexcept {
		return xm_pElement;
	}

	bool IsUnique() const noexcept {
		return GetSharedCount() == 1;
	}
	std::size_t GetSharedCount() const noexcept {
		return xm_pControl ? xm_pControl->GetShared() : 0;
	}
	std::size_t GetWeakCount() const noexcept {
		return xm_pControl ? xm_pControl->GetWeak() : 0;
	}

	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const SharedPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return xm_pControl == rhs.xm_pControl;
	}
	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const WeakPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept;

	SharedPtr &Reset() noexcept {
		const auto pOldControl = std::exchange(xm_pControl, nullptr);
		if(pOldControl){
			pOldControl->DropShared(); // noexcept
		}
		xm_pElement = nullptr;
		return *this;
	}
	template<typename TestRawT = Raw>
	SharedPtr &Reset(std::enable_if_t<std::is_convertible<TestRawT *, Element *>::value, TestRawT> *pRaw){
		return Reset(pRaw, pRaw);
	}
	SharedPtr &Reset(Raw *pRaw, Element *pElement){
		ASSERT(GetRaw() != pRaw);

		Impl::SharedControl *pControl;
		try {
			pControl = new Impl::SharedControl(
				[](void *pToDelete) noexcept {
					DeleterT()(static_cast<Raw *>(pToDelete));
				},
				const_cast<void *>(static_cast<const volatile void *>(pRaw)));
		} catch(...){
			DeleterT()(pRaw);
			throw;
		}
		const auto pOldControl = std::exchange(xm_pControl, pControl);
		if(pOldControl){
			pOldControl->DropShared(); // noexcept
		}
		xm_pElement = pElement;
		return *this;
	}
	SharedPtr &Reset(const SharedPtr &rhs) noexcept {
		if(this != &rhs){
			Reset(rhs, rhs.xm_pElement);
		} else {
			xm_pElement = rhs.xm_pElement;
		}
		return *this;
	}
	SharedPtr &Reset(SharedPtr &&rhs) noexcept {
		ASSERT(this != &rhs);

		return Reset(std::move(rhs), rhs.xm_pElement);
	}
	template<typename OtherT,
		std::enable_if_t<std::is_array<OtherT>::value
			? std::is_same<std::remove_cv_t<std::remove_extent_t<OtherT>>, std::remove_cv_t<Element>>::value
			: std::is_convertible<OtherT *, Element *>::value,
		int> = 0>
	SharedPtr &Reset(const SharedPtr<OtherT, DeleterT> &rhs) noexcept {
		return Reset(rhs, static_cast<Element *>(rhs.xm_pElement));
	}
	template<typename OtherT,
		std::enable_if_t<std::is_array<OtherT>::value
			? std::is_same<std::remove_cv_t<std::remove_extent_t<OtherT>>, std::remove_cv_t<Element>>::value
			: std::is_convertible<OtherT *, Element *>::value,
		int> = 0>
	SharedPtr &Reset(SharedPtr<OtherT, DeleterT> &&rhs) noexcept {
		return Reset(std::move(rhs), static_cast<Element *>(rhs.xm_pElement));
	}
	template<typename OtherT>
	SharedPtr &Reset(const SharedPtr<OtherT, DeleterT> &rhs, Element *pElement) noexcept {
		const auto pOldControl = std::exchange(xm_pControl, rhs.xm_pControl);
		if(xm_pControl){
			xm_pControl->AddShared(); // noexcept
		}
		if(pOldControl){
			pOldControl->DropShared(); // noexcept
		}
		xm_pElement = pElement;
		return *this;
	}
	template<typename OtherT>
	SharedPtr &Reset(SharedPtr<OtherT, DeleterT> &&rhs, Element *pElement) noexcept {
		const auto pOldControl = std::exchange(xm_pControl, rhs.xm_pControl);
		if(pOldControl){
			pOldControl->DropShared(); // noexcept
		}
		xm_pElement = pElement;

		rhs.xm_pControl = nullptr;
		rhs.xm_pElement = nullptr;
		return *this;
	}

	void Swap(SharedPtr &rhs) noexcept {
		std::swap(xm_pControl, rhs.xm_pControl);
		std::swap(xm_pElement, rhs.xm_pElement);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator Element *() const noexcept {
		return Get();
	}

	template<typename RetT = Element>
	std::enable_if_t<!std::is_void<RetT>::value && !std::is_array<RetT>::value, Element> &operator*() const noexcept {
		ASSERT(IsNonnull());

		return *Get();
	}
	template<typename RetT = Element>
	std::enable_if_t<!std::is_void<RetT>::value && !std::is_array<RetT>::value, Element> *operator->() const noexcept {
		ASSERT(IsNonnull());

		return Get();
	}
	template<typename RetT = Element>
	std::enable_if_t<std::is_array<RetT>::value, Element> &operator[](std::size_t uIndex) const noexcept {
		ASSERT(IsNonnull());

		return Get()[uIndex];
	}
};

template<typename ObjectT, class DeleterT>
class WeakPtr {
	template<typename, class>
	friend class SharedPtr;

	template<typename, class>
	friend class WeakPtr;

public:
	using Element = std::remove_extent_t<ObjectT>;

private:
	Impl::SharedControl *xm_pControl;
	Element *xm_pElement;

public:
	constexpr WeakPtr() noexcept
		: xm_pControl(nullptr), xm_pElement(nullptr)
	{
	}
	WeakPtr(const WeakPtr &rhs) noexcept
		: WeakPtr()
	{
		Reset(rhs);
	}
	WeakPtr(WeakPtr &&rhs) noexcept
		: WeakPtr()
	{
		Reset(std::move(rhs));
	}
	WeakPtr(const SharedPtr<ObjectT, DeleterT> &rhs) noexcept
		: WeakPtr()
	{
		Reset(rhs);
	}
	WeakPtr &operator=(const WeakPtr &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	WeakPtr &operator=(WeakPtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	WeakPtr &operator=(const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	~WeakPtr(){
		Reset();
	}

public:
	SharedPtr<ObjectT, DeleterT> Lock() const noexcept {
		SharedPtr<ObjectT, DeleterT> pRet;
		if(xm_pControl){
			bool bResult;
			xm_pControl->TryAddShared(bResult); // noexcept
			if(bResult){
				pRet.xm_pControl = xm_pControl;
				pRet.xm_pElement = xm_pElement;
			}
		}
		return std::move(pRet);
	}

	std::size_t GetSharedCount() const noexcept {
		return xm_pControl ? xm_pControl->GetShared() : 0;
	}
	std::size_t GetWeakCount() const noexcept {
		return xm_pControl ? xm_pControl->GetWeak() : 0;
	}
	bool IsAlive() const noexcept {
		return GetWeakCount() != 0;
	}

	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const SharedPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return xm_pControl == rhs.xm_pControl;
	}
	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const WeakPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return xm_pControl == rhs.xm_pControl;
	}

	WeakPtr &Reset() noexcept {
		const auto pOldControl = std::exchange(xm_pControl, nullptr);
		if(pOldControl){
			pOldControl->DropWeak(); // noexcept
		}
		xm_pElement = nullptr;
		return *this;
	}
	WeakPtr &Reset(const WeakPtr &rhs) noexcept {
		if(this != &rhs){
			const auto pOldControl = std::exchange(xm_pControl, rhs.xm_pControl);
			if(xm_pControl){
				xm_pControl->AddWeak(); // noexcept
			}
			if(pOldControl){
				pOldControl->DropWeak(); // noexcept
			}
			xm_pElement = rhs.xm_pElement;
		}
		return *this;
	}
	WeakPtr &Reset(WeakPtr &&rhs) noexcept {
		ASSERT(this != &rhs);

		const auto pOldControl = std::exchange(xm_pControl, rhs.xm_pControl);
		if(pOldControl){
			pOldControl->DropWeak(); // noexcept
		}
		xm_pElement = rhs.xm_pElement;

		rhs.xm_pControl = nullptr;
		rhs.xm_pElement = nullptr;
		return *this;
	}
	WeakPtr &Reset(const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
		const auto pOldControl = std::exchange(xm_pControl, rhs.xm_pControl);
		if(xm_pControl){
			xm_pControl->AddWeak(); // noexcept
		}
		if(pOldControl){
			pOldControl->DropWeak(); // noexcept
		}
		xm_pElement = rhs.xm_pElement;
		return *this;
	}

	void Swap(WeakPtr &rhs) noexcept {
		std::swap(xm_pControl, rhs.xm_pControl);
		std::swap(xm_pElement, rhs.xm_pElement);
	}
};

template<typename ObjectT, class DeleterT>
	template<typename OtherObjectT, class OtherDeleterT>
bool SharedPtr<ObjectT, DeleterT>::IsSharedWith(const WeakPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
	return xm_pControl == rhs.xm_pControl;
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator==(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() == rhs.xm_pElement;
}
template<typename ObjectT, class DeleterT>
bool operator==(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() == rhs;
}
template<typename ObjectT, class DeleterT>
bool operator==(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs == rhs.xm_pElement;
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator!=(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() != rhs.xm_pElement;
}
template<typename ObjectT, class DeleterT>
bool operator!=(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() != rhs;
}
template<typename ObjectT, class DeleterT>
bool operator!=(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs != rhs.xm_pElement;
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator<(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() < rhs.xm_pElement;
}
template<typename ObjectT, class DeleterT>
bool operator<(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() < rhs;
}
template<typename ObjectT, class DeleterT>
bool operator<(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs < rhs.xm_pElement;
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator>(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() > rhs.xm_pElement;
}
template<typename ObjectT, class DeleterT>
bool operator>(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() > rhs;
}
template<typename ObjectT, class DeleterT>
bool operator>(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs > rhs.xm_pElement;
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator<=(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() <= rhs.xm_pElement;
}
template<typename ObjectT, class DeleterT>
bool operator<=(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() <= rhs;
}
template<typename ObjectT, class DeleterT>
bool operator<=(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs <= rhs.xm_pElement;
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator>=(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() >= rhs.xm_pElement;
}
template<typename ObjectT, class DeleterT>
bool operator>=(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() >= rhs;
}
template<typename ObjectT, class DeleterT>
bool operator>=(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs >= rhs.xm_pElement;
}

template<typename ObjectT, class DeleterT>
void swap(SharedPtr<ObjectT, DeleterT> &lhs, SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	lhs.Swap(rhs);
}
template<typename ObjectT, class DeleterT>
void swap(WeakPtr<ObjectT, DeleterT> &lhs, WeakPtr<ObjectT, DeleterT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ObjectT, typename ...ParamsT>
auto MakeShared(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	const auto pObject = new ObjectT(std::forward<ParamsT>(vParams)...);
	return SharedPtr<ObjectT, DefaultDeleter<ObjectT>>(pObject);
}

template<typename DstT, typename SrcT, class DeleterT>
auto StaticPointerCast(SharedPtr<SrcT, DeleterT> rhs) noexcept {
	return SharedPtr<DstT, DeleterT>(std::move(rhs), static_cast<DstT *>(rhs.xm_pElement));
}
template<typename DstT, typename SrcT, class DeleterT>
auto DynamicPointerCast(SharedPtr<SrcT, DeleterT> rhs) noexcept {
	return SharedPtr<DstT, DeleterT>(std::move(rhs), dynamic_cast<DstT *>(rhs.xm_pElement));
}
template<typename DstT, typename SrcT, class DeleterT>
auto ConstPointerCast(SharedPtr<SrcT, DeleterT> rhs) noexcept {
	return SharedPtr<DstT, DeleterT>(std::move(rhs), const_cast<DstT *>(rhs.xm_pElement));
}

}

#endif
