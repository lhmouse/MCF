// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_SHARED_PTR_HPP_
#define MCF_SMART_POINTERS_SHARED_PTR_HPP_

// 1) 构造函数（包含复制构造函数、转移构造函数以及构造函数模板）、赋值运算符和析构函数应当调用 Reset()；
// 2) Reset() 的形参若具有 SharedPtr 的某模板类类型，则禁止传值，必须传引用。

#include "../Utilities/Assert.hpp"
#include "../Thread/Atomic.hpp"
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

namespace Impl_SharedPtr {
	class SharedControl final {
	public:
		class Sentry {
		private:
			SharedControl *x_pOwner;

		public:
			explicit constexpr Sentry(SharedControl *pOwner) noexcept
				: x_pOwner(pOwner)
			{
			}
			Sentry(Sentry &&rhs) noexcept
				: x_pOwner(std::exchange(rhs.x_pOwner, nullptr))
			{
			}
			~Sentry(){
				if(x_pOwner){
					delete x_pOwner;
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
		void (*const x_pfnDeleter)(void *);

		void *x_pToDelete;
		volatile std::size_t x_uSharedCount;
		volatile std::size_t x_uWeakCount;

	public:
		SharedControl(void (*pfnDeleter)(void *), void *pData) noexcept
			: x_pfnDeleter(pfnDeleter)
			, x_pToDelete(pData), x_uSharedCount(1), x_uWeakCount(1)
		{
			AtomicFence(MemoryModel::kRelaxed);
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
			return AtomicLoad(x_uSharedCount, MemoryModel::kRelaxed);
		}
		void AddShared() noexcept {
			AddWeak();

			ASSERT(AtomicLoad(x_uSharedCount, MemoryModel::kRelaxed) != 0);
			AtomicIncrement(x_uSharedCount, MemoryModel::kRelaxed);
		}
		Sentry TryAddShared(bool &bResult) noexcept {
			AddWeak();

			auto uOldShared = AtomicLoad(x_uSharedCount, MemoryModel::kRelaxed);
			for(;;){
				if(EXPECT_NOT(uOldShared == 0)){
					goto jFailed;
				}
				if(EXPECT_NOT(AtomicCompareExchange(x_uSharedCount, uOldShared, uOldShared + 1, MemoryModel::kRelaxed))){
					goto jDone;
				}
			}

		jFailed:
			bResult = false;
			return DropWeak();

		jDone:
			bResult = true;
			return Sentry(nullptr);
		}
		Sentry DropShared() noexcept {
			ASSERT(AtomicLoad(x_uSharedCount, MemoryModel::kRelaxed) != 0);
			if(AtomicDecrement(x_uSharedCount, MemoryModel::kRelaxed) == 0){
				(*x_pfnDeleter)(x_pToDelete);
#ifndef NDEBUG
				x_pToDelete = (void *)
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
			return AtomicLoad(x_uWeakCount, MemoryModel::kRelaxed);
		}
		void AddWeak() noexcept {
			ASSERT(AtomicLoad(x_uWeakCount, MemoryModel::kRelaxed) != 0);
			AtomicIncrement(x_uWeakCount, MemoryModel::kRelaxed);
		}
		Sentry DropWeak() noexcept {
			ASSERT(AtomicLoad(x_uWeakCount, MemoryModel::kRelaxed) != 0);
			SharedControl *pToDelete = nullptr;
			if(AtomicDecrement(x_uWeakCount, MemoryModel::kRelaxed) == 0){
				pToDelete = this;
			}
			return Sentry(pToDelete);
		}

		void *GetRaw() const noexcept {
			return x_pToDelete;
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
	using Raw = std::remove_pointer_t<std::decay_t<decltype(DeleterT()())>>;
	using ElementType = std::remove_extent_t<ObjectT>;

private:
	Impl_SharedPtr::SharedControl *x_pControl;
	ElementType *x_pElement;

public:
	explicit constexpr SharedPtr(std::nullptr_t = nullptr) noexcept
		: x_pControl(nullptr), x_pElement(nullptr)
	{
	}
	template<typename TestRawT = Raw>
	SharedPtr(std::enable_if_t<std::is_convertible<TestRawT *, ElementType *>::value, TestRawT> *pRaw)
		: SharedPtr()
	{
		Reset(pRaw);
	}
	SharedPtr(Raw *pRaw, ElementType *pElement)
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
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename SharedPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	SharedPtr(const SharedPtr<OtherT, OtherDeleterT> &rhs) noexcept
		: SharedPtr()
	{
		Reset(rhs);
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename SharedPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	SharedPtr(SharedPtr<OtherT, OtherDeleterT> &&rhs) noexcept
		: SharedPtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherT>
	SharedPtr(const SharedPtr<OtherT, DeleterT> &rhs, ElementType *pElement) noexcept
		: SharedPtr()
	{
		Reset(rhs, pElement);
	}
	template<typename OtherT>
	SharedPtr(SharedPtr<OtherT, DeleterT> &&rhs, ElementType *pElement) noexcept
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
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename SharedPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	SharedPtr &operator=(const SharedPtr<OtherT, OtherDeleterT> &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename SharedPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	SharedPtr &operator=(SharedPtr<OtherT, OtherDeleterT> &&rhs) noexcept {
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
		return x_pControl ? static_cast<Raw *>(x_pControl->GetRaw()) : nullptr;
	}
	bool IsNonnull() const noexcept {
		return Get() != nullptr;
	}
	ElementType *Get() const noexcept {
		return x_pElement;
	}

	bool IsUnique() const noexcept {
		return GetSharedCount() == 1;
	}
	std::size_t GetSharedCount() const noexcept {
		return x_pControl ? x_pControl->GetShared() : 0;
	}
	std::size_t GetWeakCount() const noexcept {
		return x_pControl ? x_pControl->GetWeak() : 0;
	}

	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const SharedPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept;
	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const WeakPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept;

	SharedPtr &Reset(std::nullptr_t = nullptr) noexcept {
		const auto pOldControl = std::exchange(x_pControl, nullptr);
		if(pOldControl){
			pOldControl->DropShared(); // noexcept
		}
		x_pElement = nullptr;
		return *this;
	}
	template<typename TestRawT = Raw>
	SharedPtr &Reset(std::enable_if_t<std::is_convertible<TestRawT *, ElementType *>::value, TestRawT> *pRaw){
		return Reset(pRaw, pRaw);
	}
	SharedPtr &Reset(Raw *pRaw, ElementType *pElement){
		ASSERT(!(pRaw && (GetRaw() == pRaw)));

		Impl_SharedPtr::SharedControl *pControl;
		try {
			pControl = new Impl_SharedPtr::SharedControl(
				[](void *pToDelete) noexcept {
					DeleterT()(static_cast<Raw *>(pToDelete));
				},
				const_cast<void *>(static_cast<const volatile void *>(pRaw)));
		} catch(...){
			DeleterT()(pRaw);
			throw;
		}
		const auto pOldControl = std::exchange(x_pControl, pControl);
		if(pOldControl){
			pOldControl->DropShared(); // noexcept
		}
		x_pElement = pElement;
		return *this;
	}
	SharedPtr &Reset(const SharedPtr &rhs) noexcept {
		if(this != &rhs){
			Reset(rhs, rhs.x_pElement);
		} else {
			x_pElement = rhs.x_pElement;
		}
		return *this;
	}
	SharedPtr &Reset(SharedPtr &&rhs) noexcept {
		ASSERT(this != &rhs);

		return Reset(std::move(rhs), rhs.x_pElement);
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename SharedPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	SharedPtr &Reset(const SharedPtr<OtherT, OtherDeleterT> &rhs) noexcept {
		return Reset(rhs, static_cast<ElementType *>(rhs.x_pElement));
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename SharedPtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	SharedPtr &Reset(SharedPtr<OtherT, OtherDeleterT> &&rhs) noexcept {
		return Reset(std::move(rhs), static_cast<ElementType *>(rhs.x_pElement));
	}
	template<typename OtherT>
	SharedPtr &Reset(const SharedPtr<OtherT, DeleterT> &rhs, ElementType *pElement) noexcept {
		const auto pOldControl = std::exchange(x_pControl, rhs.x_pControl);
		if(x_pControl){
			x_pControl->AddShared(); // noexcept
		}
		if(pOldControl){
			pOldControl->DropShared(); // noexcept
		}
		x_pElement = pElement;
		return *this;
	}
	template<typename OtherT>
	SharedPtr &Reset(SharedPtr<OtherT, DeleterT> &&rhs, ElementType *pElement) noexcept {
		const auto pOldControl = std::exchange(x_pControl, rhs.x_pControl);
		if(pOldControl){
			pOldControl->DropShared(); // noexcept
		}
		x_pElement = pElement;

		rhs.x_pControl = nullptr;
		rhs.x_pElement = nullptr;
		return *this;
	}

	void Swap(SharedPtr &rhs) noexcept {
		std::swap(x_pControl, rhs.x_pControl);
		std::swap(x_pElement, rhs.x_pElement);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator ElementType *() const noexcept {
		return Get();
	}

	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value && !std::is_array<TestT>::value, ElementType> &operator*() const noexcept {
		ASSERT(IsNonnull());

		return *Get();
	}
	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value && !std::is_array<TestT>::value, ElementType> *operator->() const noexcept {
		ASSERT(IsNonnull());

		return Get();
	}
	template<typename TestT = ObjectT>
	std::enable_if_t<std::is_array<TestT>::value, ElementType> &operator[](std::size_t uIndex) const noexcept {
		ASSERT(IsNonnull());

		return Get()[uIndex];
	}
};

#define MCF_SMART_POINTERS_DECLARE_TEMPLATE_PARAMETERS_	template<typename ObjectT, class DeleterT>
#define MCF_SMART_POINTERS_INVOKE_TEMPLATE_PARAMETERS_	SharedPtr<ObjectT, DeleterT>
#include "_RationalAndSwap.hpp"

template<typename ObjectT, class DeleterT>
class WeakPtr {
	template<typename, class>
	friend class SharedPtr;
	template<typename, class>
	friend class WeakPtr;

public:
	using ElementType = std::remove_extent_t<ObjectT>;

private:
	Impl_SharedPtr::SharedControl *x_pControl;
	ElementType *x_pElement;

public:
	constexpr WeakPtr() noexcept
		: x_pControl(nullptr), x_pElement(nullptr)
	{
	}
	WeakPtr(const SharedPtr<ObjectT, DeleterT> &rhs) noexcept
		: WeakPtr()
	{
		Reset(rhs);
	}
	WeakPtr(SharedPtr<ObjectT, DeleterT> &&rhs) noexcept
		: WeakPtr()
	{
		Reset(std::move(rhs));
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
	WeakPtr &operator=(const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	WeakPtr &operator=(SharedPtr<ObjectT, DeleterT> &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	WeakPtr &operator=(const WeakPtr &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	WeakPtr &operator=(WeakPtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~WeakPtr(){
		Reset();
	}

public:
	SharedPtr<ObjectT, DeleterT> Lock() const noexcept {
		SharedPtr<ObjectT, DeleterT> pRet;
		if(x_pControl){
			bool bResult;
			x_pControl->TryAddShared(bResult); // noexcept
			if(bResult){
				pRet.x_pControl = x_pControl;
				pRet.x_pElement = x_pElement;
			}
		}
		return pRet;
	}

	std::size_t GetSharedCount() const noexcept {
		return x_pControl ? x_pControl->GetShared() : 0;
	}
	std::size_t GetWeakCount() const noexcept {
		return x_pControl ? x_pControl->GetWeak() : 0;
	}
	bool IsAlive() const noexcept {
		return GetWeakCount() != 0;
	}

	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const SharedPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pControl == rhs.x_pControl;
	}
	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const WeakPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pControl == rhs.x_pControl;
	}

	WeakPtr &Reset() noexcept {
		const auto pOldControl = std::exchange(x_pControl, nullptr);
		if(pOldControl){
			pOldControl->DropWeak(); // noexcept
		}
		x_pElement = nullptr;
		return *this;
	}
	WeakPtr &Reset(const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
		const auto pOldControl = std::exchange(x_pControl, rhs.x_pControl);
		if(x_pControl){
			x_pControl->AddWeak(); // noexcept
		}
		if(pOldControl){
			pOldControl->DropWeak(); // noexcept
		}
		x_pElement = rhs.x_pElement;
		return *this;
	}
	WeakPtr &Reset(SharedPtr<ObjectT, DeleterT> &&rhs) noexcept {
		ASSERT(this != &rhs);

		const auto pOldControl = std::exchange(x_pControl, rhs.x_pControl);
		if(pOldControl){
			pOldControl->DropWeak(); // noexcept
		}
		x_pElement = rhs.x_pElement;

		rhs.x_pControl = nullptr;
		rhs.x_pElement = nullptr;
		return *this;
	}
	WeakPtr &Reset(const WeakPtr &rhs) noexcept {
		if(this != &rhs){
			const auto pOldControl = std::exchange(x_pControl, rhs.x_pControl);
			if(x_pControl){
				x_pControl->AddWeak(); // noexcept
			}
			if(pOldControl){
				pOldControl->DropWeak(); // noexcept
			}
			x_pElement = rhs.x_pElement;
		}
		return *this;
	}
	WeakPtr &Reset(WeakPtr &&rhs) noexcept {
		ASSERT(this != &rhs);

		const auto pOldControl = std::exchange(x_pControl, rhs.x_pControl);
		if(pOldControl){
			pOldControl->DropWeak(); // noexcept
		}
		x_pElement = rhs.x_pElement;

		rhs.x_pControl = nullptr;
		rhs.x_pElement = nullptr;
		return *this;
	}

	void Swap(WeakPtr &rhs) noexcept {
		std::swap(x_pControl, rhs.x_pControl);
		std::swap(x_pElement, rhs.x_pElement);
	}
};

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
	return SharedPtr<DstT, DeleterT>(std::move(rhs), static_cast<DstT *>(rhs.Get()));
}
template<typename DstT, typename SrcT, class DeleterT>
auto DynamicPointerCast(SharedPtr<SrcT, DeleterT> rhs) noexcept {
	const auto pElement = dynamic_cast<DstT *>(rhs.Get());
	if(!pElement){
		return SharedPtr<DstT, DeleterT>();
	}
	return SharedPtr<DstT, DeleterT>(std::move(rhs), pElement);
}
template<typename DstT, typename SrcT, class DeleterT>
auto ConstPointerCast(SharedPtr<SrcT, DeleterT> rhs) noexcept {
	return SharedPtr<DstT, DeleterT>(std::move(rhs), const_cast<DstT *>(rhs.Get()));
}

}

#endif
