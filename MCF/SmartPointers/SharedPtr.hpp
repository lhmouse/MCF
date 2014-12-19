// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_SHARED_PTR_HPP_
#define MCF_SMART_POINTERS_SHARED_PTR_HPP_

#include "../Utilities/Assert.hpp"
#include "DefaultDeleter.hpp"
#include "Traits.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {
/*
namespace Impl {
	class SharedPtrControlBlock {

		void *pData;
		std::size_t uSize;
		void (*pfnDestructor)(void *);

		volatile std::size_t uRefCount;
		volatile std::size_t uWeakCount;
	};
}

template<typename ObjectT>
class SharedPtr {
private:
	Impl::SharedPtrControlBlock *xm_pControlBlock;
	ObjectT *xm_pObject;

public:
	constexpr SharedPtr() noexcept
		: xm_pControlBlock(nullptr), xm_pObject(nullptr)
	{
	}
	explicit SharedPtr(ObjectT *pObject)
		: SharedPtr()
	{
		Reset(pObject);
	}
	SharedPtr(const SharedPtr &rhs) noexcept
		: SharedPtr()
	{
		Reset(rhs);
	}bbb
	SharedPtr(SharedPtr &&rhs) noexcept
		: SharedPtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherT>
	explicit SharedPtr(SharedPtr<OtherT> rhs) noexcept
		: SharedPtr()
	{
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(std::move(rhs));
	}
	template<typename OtherT>
	explicit SharedPtr(SharedPtr<OtherT> rhs, ObjectT *pObject) noexcept
		: SharedPtr()
	{
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(std::move(rhs), pObject);
	}
	SharedPtr &operator=(const SharedPtr &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	SharedPtr &operator=(SharedPtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	template<typename OtherT>
	SharedPtr &operator=(SharedPtr<OtherT> rhs) noexcept {
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(std::move(rhs));
	}
	template<typename OtherT>
	SharedPtr &operator=(SharedPtr<OtherT> rhs, ObjectT *pObject) noexcept
		: SharedPtr()
	{
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(std::move(rhs), pObject);
	}
	~SharedPtr(){
		Reset();
	}

	SharedPtr(const SharedPtr &) = delete;
	SharedPtr &operator=(const SharedPtr &) = delete;

public:
	bool IsNonnull() const noexcept {
		return Get() != nullptr;
	}
	ObjectT *Get() const noexcept {
		return xm_pObject;
	}
	ObjectT *Release() noexcept {
		return std::exchange(xm_pObject, nullptr);
	}

	SharedPtr &Reset(ObjectT *pObject = nullptr) noexcept {
		const auto pOld = std::exchange(xm_pObject, pObject);
		if(pOld){
			DeleterT()(const_cast<std::remove_cv_t<ObjectT> *>(pOld));
		}
		return *this;
	}
	SharedPtr &Reset(SharedPtr &&rhs) noexcept {
		return Reset(rhs.Release());
	}
	template<typename OtherT>
	SharedPtr &Reset(SharedPtr<OtherT, DeleterT> &&rhs) noexcept {
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value, "Unable to convert from `OtherT *` to `ObjectT *`.");

		return Reset(rhs.Release());
	}

	void Swap(SharedPtr &rhs) noexcept {
		std::swap(xm_pObject, rhs.xm_pObject);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator ObjectT *() const noexcept {
		return Get();
	}

	ObjectT &operator*() const noexcept {
		ASSERT(IsNonnull());

		return *Get();
	}
	ObjectT *operator->() const noexcept {
		ASSERT(IsNonnull());

		return Get();
	}
};

template<typename ObjectT, class DeleterT>
class SharedPtr<ObjectT [], DeleterT> {
public:
	static_assert(noexcept(DeleterT()(std::declval<std::remove_cv_t<ObjectT> *>())), "Deleter must not throw.");

private:
	ObjectT *xm_pObject;

public:
	constexpr SharedPtr() noexcept
		: SharedPtr(nullptr)
	{
	}
	constexpr explicit SharedPtr(ObjectT *pObject) noexcept
		: xm_pObject(pObject)
	{
	}
	SharedPtr(SharedPtr &&rhs) noexcept
		: SharedPtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherT>
	explicit SharedPtr(SharedPtr<OtherT [], DeleterT> &&rhs) noexcept
		: SharedPtr()
	{
		static_assert(std::is_same<std::remove_cv_t<OtherT>, std::remove_cv_t<ObjectT>>::value,
			"Unable to convert from `OtherT *` to `ObjectT *`. Note: arrays do not support covariation.");

		Reset(std::move(rhs));
	}
	SharedPtr &operator=(SharedPtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	template<typename OtherT>
	SharedPtr &operator=(SharedPtr<OtherT [], DeleterT> &&rhs) noexcept {
		static_assert(std::is_same<std::remove_cv_t<OtherT>, std::remove_cv_t<ObjectT>>::value,
			"Unable to convert from `OtherT *` to `ObjectT *`. Note: arrays do not support covariation.");

		Reset(std::move(rhs));
		return *this;
	}
	~SharedPtr(){
		Reset();
	}

	SharedPtr(const SharedPtr &) = delete;
	SharedPtr &operator=(const SharedPtr &) = delete;

public:
	bool IsNonnull() const noexcept {
		return Get() != nullptr;
	}
	ObjectT *Get() const noexcept {
		return xm_pObject;
	}
	ObjectT *Release() noexcept {
		return std::exchange(xm_pObject, nullptr);
	}

	SharedPtr &Reset(ObjectT *pObject = nullptr) noexcept {
		const auto pOld = std::exchange(xm_pObject, pObject);
		if(pOld){
			ASSERT(pOld != pObject);
			DeleterT()(const_cast<std::remove_cv_t<ObjectT> *>(pOld));
		}
		return *this;
	}
	SharedPtr &Reset(SharedPtr &&rhs) noexcept {
		return Reset(rhs.Release());
	}
	template<typename OtherT>
	SharedPtr &Reset(SharedPtr<OtherT [], DeleterT> &&rhs) noexcept {
		static_assert(std::is_same<std::remove_cv_t<OtherT>, std::remove_cv_t<ObjectT>>::value,
			"Unable to convert from `OtherT *` to `ObjectT *`. Note: arrays do not support covariation.");

		return Reset(rhs.Release());
	}

	void Swap(SharedPtr &rhs) noexcept {
		std::swap(xm_pObject, rhs.xm_pObject);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator ObjectT *() const noexcept {
		return Get();
	}

	ObjectT &operator[](std::size_t uIndex) const noexcept {
		ASSERT(IsNonnull());

		return Get()[uIndex];
	}
};

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator==(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() == rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator==(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() == rhs;
}
template<typename ObjectT, class DeleterT>
bool operator==(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs == rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator!=(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() != rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator!=(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() != rhs;
}
template<typename ObjectT, class DeleterT>
bool operator!=(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs != rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator<(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() < rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator<(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() < rhs;
}
template<typename ObjectT, class DeleterT>
bool operator<(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs < rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator>(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() > rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator>(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() > rhs;
}
template<typename ObjectT, class DeleterT>
bool operator>(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs > rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator<=(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() <= rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator<=(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() <= rhs;
}
template<typename ObjectT, class DeleterT>
bool operator<=(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs <= rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator>=(const SharedPtr<Object1T, Deleter1T> &lhs, const SharedPtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() >= rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator>=(const SharedPtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() >= rhs;
}
template<typename ObjectT, class DeleterT>
bool operator>=(ObjectT *lhs, const SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs >= rhs.Get();
}

template<typename ObjectT, class DeleterT>
void swap(SharedPtr<ObjectT, DeleterT> &lhs, SharedPtr<ObjectT, DeleterT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ObjectT, typename ...ParamsT>
auto MakeShared(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	return SharedPtr<ObjectT, DefaultDeleter<ObjectT>>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT, class DeleterT>
auto StaticPointerCast(SharedPtr<SrcT, DeleterT> rhs) noexcept {
	SrcT *const pSrc = rhs.Release();
	DstT *const pDst = static_cast<DstT *>(pSrc);
	return SharedPtr<DstT, DeleterT>(pDst);
}
template<typename DstT, typename SrcT, class DeleterT>
auto DynamicPointerCast(SharedPtr<SrcT, DeleterT> rhs) noexcept {
	SrcT *const pSrc = rhs.Release();
	DstT *const pDst = dynamic_cast<DstT *>(pSrc);
	if(!pDst && pSrc){
		pSrc->DropRef();
	}
	return SharedPtr<DstT, DeleterT>(pDst);
}
*/
}

#endif
