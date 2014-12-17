// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_UNIQUE_PTR_HPP_
#define MCF_SMART_POINTERS_UNIQUE_PTR_HPP_

#include "../Utilities/Assert.hpp"
#include "DefaultDeleter.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class UniquePtr {
public:
	static_assert(noexcept(DeleterT()(std::declval<std::remove_cv_t<ObjectT> *>())), "Deleter must not throw.");

private:
	ObjectT *xm_pObject;

public:
	constexpr UniquePtr() noexcept
		: UniquePtr(nullptr)
	{
	}
	constexpr explicit UniquePtr(ObjectT *pObject) noexcept
		: xm_pObject(pObject)
	{
	}
	UniquePtr(UniquePtr &&rhs) noexcept
		: UniquePtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherT>
	explicit UniquePtr(UniquePtr<OtherT, DeleterT> &&rhs) noexcept
		: UniquePtr()
	{
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value,
			"Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(std::move(rhs));
	}
	UniquePtr &operator=(UniquePtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	template<typename OtherT>
	UniquePtr &operator=(UniquePtr<OtherT, DeleterT> &&rhs) noexcept {
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value,
			"Unable to convert from `OtherT *` to `ObjectT *`.");

		Reset(std::move(rhs));
		return *this;
	}
	~UniquePtr(){
		Reset();
	}

	UniquePtr(const UniquePtr &) = delete;
	UniquePtr &operator=(const UniquePtr &) = delete;

public:
	bool IsValid() const noexcept {
		return Get() != nullptr;
	}
	ObjectT *Get() const noexcept {
		return xm_pObject;
	}
	ObjectT *Release() noexcept {
		return std::exchange(xm_pObject, nullptr);
	}

	UniquePtr &Reset(ObjectT *pObject = nullptr) noexcept {
		const auto pOld = std::exchange(xm_pObject, pObject);
		if(pOld){
			DeleterT()(const_cast<std::remove_cv_t<ObjectT> *>(pOld));
		}
		return *this;
	}
	UniquePtr &Reset(UniquePtr &&rhs) noexcept {
		return Reset(rhs.Release());
	}
	template<typename OtherT>
	UniquePtr &Reset(UniquePtr<OtherT, DeleterT> &&rhs) noexcept {
		static_assert(std::is_convertible<OtherT *, ObjectT *>::value,
			"Unable to convert from `OtherT *` to `ObjectT *`.");

		return Reset(rhs.Release());
	}

	void Swap(UniquePtr &rhs) noexcept {
		std::swap(xm_pObject, rhs.xm_pObject);
	}

public:
	explicit operator bool() const noexcept {
		return IsValid();
	}
	explicit operator ObjectT *() const noexcept {
		return Get();
	}

	ObjectT &operator*() const noexcept {
		ASSERT(IsValid());

		return *Get();
	}
	ObjectT *operator->() const noexcept {
		ASSERT(IsValid());

		return Get();
	}
};

template<typename ObjectT, class DeleterT>
class UniquePtr<ObjectT [], DeleterT> {
public:
	static_assert(noexcept(DeleterT()(std::declval<std::remove_cv_t<ObjectT> *>())), "Deleter must not throw.");

private:
	ObjectT *xm_pObject;

public:
	constexpr UniquePtr() noexcept
		: UniquePtr(nullptr)
	{
	}
	constexpr explicit UniquePtr(ObjectT *pObject) noexcept
		: xm_pObject(pObject)
	{
	}
	UniquePtr(UniquePtr &&rhs) noexcept
		: UniquePtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherT>
	explicit UniquePtr(UniquePtr<OtherT [], DeleterT> &&rhs) noexcept
		: UniquePtr()
	{
		static_assert(std::is_same<std::remove_cv_t<OtherT>, std::remove_cv_t<ObjectT>>::value,
			"Unable to convert from `OtherT *` to `ObjectT *`. Note: arrays do not support covariation.");

		Reset(std::move(rhs));
	}
	UniquePtr &operator=(UniquePtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	template<typename OtherT>
	UniquePtr &operator=(UniquePtr<OtherT [], DeleterT> &&rhs) noexcept {
		static_assert(std::is_same<std::remove_cv_t<OtherT>, std::remove_cv_t<ObjectT>>::value,
			"Unable to convert from `OtherT *` to `ObjectT *`. Note: arrays do not support covariation.");

		Reset(std::move(rhs));
		return *this;
	}
	~UniquePtr(){
		Reset();
	}

	UniquePtr(const UniquePtr &) = delete;
	UniquePtr &operator=(const UniquePtr &) = delete;

public:
	bool IsValid() const noexcept {
		return Get() != nullptr;
	}
	ObjectT *Get() const noexcept {
		return xm_pObject;
	}
	ObjectT *Release() noexcept {
		return std::exchange(xm_pObject, nullptr);
	}

	UniquePtr &Reset(ObjectT *pObject = nullptr) noexcept {
		const auto pOld = std::exchange(xm_pObject, pObject);
		if(pOld){
			ASSERT(pOld != pObject);
			DeleterT()(const_cast<std::remove_cv_t<ObjectT> *>(pOld));
		}
		return *this;
	}
	UniquePtr &Reset(UniquePtr &&rhs) noexcept {
		return Reset(rhs.Release());
	}
	template<typename OtherT>
	UniquePtr &Reset(UniquePtr<OtherT [], DeleterT> &&rhs) noexcept {
		static_assert(std::is_same<std::remove_cv_t<OtherT>, std::remove_cv_t<ObjectT>>::value,
			"Unable to convert from `OtherT *` to `ObjectT *`. Note: arrays do not support covariation.");

		return Reset(rhs.Release());
	}

	void Swap(UniquePtr &rhs) noexcept {
		std::swap(xm_pObject, rhs.xm_pObject);
	}

public:
	explicit operator bool() const noexcept {
		return IsValid();
	}
	explicit operator ObjectT *() const noexcept {
		return Get();
	}

	ObjectT &operator[](std::size_t uIndex) const noexcept {
		ASSERT(IsValid());

		return Get()[uIndex];
	}
};

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator==(const UniquePtr<Object1T, Deleter1T> &lhs, const UniquePtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() == rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator==(const UniquePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() == rhs;
}
template<typename ObjectT, class DeleterT>
bool operator==(ObjectT *lhs, const UniquePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs == rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator!=(const UniquePtr<Object1T, Deleter1T> &lhs, const UniquePtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() != rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator!=(const UniquePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() != rhs;
}
template<typename ObjectT, class DeleterT>
bool operator!=(ObjectT *lhs, const UniquePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs != rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator<(const UniquePtr<Object1T, Deleter1T> &lhs, const UniquePtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() < rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator<(const UniquePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() < rhs;
}
template<typename ObjectT, class DeleterT>
bool operator<(ObjectT *lhs, const UniquePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs < rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator>(const UniquePtr<Object1T, Deleter1T> &lhs, const UniquePtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() > rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator>(const UniquePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() > rhs;
}
template<typename ObjectT, class DeleterT>
bool operator>(ObjectT *lhs, const UniquePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs > rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator<=(const UniquePtr<Object1T, Deleter1T> &lhs, const UniquePtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() <= rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator<=(const UniquePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() <= rhs;
}
template<typename ObjectT, class DeleterT>
bool operator<=(ObjectT *lhs, const UniquePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs <= rhs.Get();
}

template<typename Object1T, class Deleter1T, typename Object2T, class Deleter2T>
bool operator>=(const UniquePtr<Object1T, Deleter1T> &lhs, const UniquePtr<Object2T, Deleter2T> &rhs) noexcept {
	return lhs.Get() >= rhs.Get();
}
template<typename ObjectT, class DeleterT>
bool operator>=(const UniquePtr<ObjectT, DeleterT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() >= rhs;
}
template<typename ObjectT, class DeleterT>
bool operator>=(ObjectT *lhs, const UniquePtr<ObjectT, DeleterT> &rhs) noexcept {
	return lhs >= rhs.Get();
}

template<typename ObjectT, class DeleterT>
void swap(UniquePtr<ObjectT, DeleterT> &lhs, UniquePtr<ObjectT, DeleterT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ObjectT, typename ...ParamsT>
auto MakeUnique(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	return UniquePtr<ObjectT, DefaultDeleter<ObjectT>>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

}

#endif
