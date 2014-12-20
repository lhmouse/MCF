// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_UNIQUE_PTR_HPP_
#define MCF_SMART_POINTERS_UNIQUE_PTR_HPP_

// 1) 构造函数（包含复制构造函数、转移构造函数以及构造函数模板）、赋值运算符和析构函数应当调用 Reset()；
// 2) Reset() 的形参若具有 UniquePtr 的某模板类类型，则禁止传值，必须传引用。

#include "../Utilities/Assert.hpp"
#include "DefaultDeleter.hpp"
#include "_Traits.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class UniquePtr
	: public Impl::SmartPointerCheckDereferencable<UniquePtr<ObjectT, DeleterT>, ObjectT>
	, public Impl::SmartPointerCheckArray<UniquePtr<ObjectT, DeleterT>, ObjectT>
{
	static_assert(noexcept(DeleterT()(DeleterT()())), "Deleter must not throw.");

public:
	using Element = std::remove_extent_t<ObjectT>;

private:
	Element *xm_pElement;

public:
	constexpr explicit UniquePtr(Element *pElement = nullptr) noexcept
		: xm_pElement(pElement)
	{
	}
	UniquePtr(UniquePtr &&rhs) noexcept
		: UniquePtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherT,
		std::enable_if_t<std::is_array<OtherT>::value
			? std::is_same<std::remove_cv_t<std::remove_extent_t<OtherT>>, std::remove_cv_t<Element>>::value
			: std::is_convertible<OtherT *, Element *>::value,
		int> = 0>
	explicit UniquePtr(UniquePtr<OtherT, DeleterT> rhs) noexcept
		: UniquePtr()
	{
		Reset(std::move(rhs));
	}
	UniquePtr &operator=(UniquePtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	template<typename OtherT,
		std::enable_if_t<std::is_array<OtherT>::value
			? std::is_same<std::remove_cv_t<std::remove_extent_t<OtherT>>, std::remove_cv_t<Element>>::value
			: std::is_convertible<OtherT *, Element *>::value,
		int> = 0>
	UniquePtr &operator=(UniquePtr<OtherT, DeleterT> rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~UniquePtr(){
		Reset();
	}

	UniquePtr(const UniquePtr &) = delete;
	UniquePtr &operator=(const UniquePtr &) = delete;

public:
	bool IsNonnull() const noexcept {
		return Get() != nullptr;
	}
	Element *Get() const noexcept {
		return xm_pElement;
	}
	Element *Release() noexcept {
		return std::exchange(xm_pElement, nullptr);
	}

	UniquePtr &Reset(Element *pElement = nullptr) noexcept {
		ASSERT(Get() != pElement);
		const auto pOld = std::exchange(xm_pElement, pElement);
		if(pOld){
			DeleterT()(const_cast<std::remove_cv_t<Element> *>(pOld));
		}
		return *this;
	}
	UniquePtr &Reset(UniquePtr &&rhs) noexcept {
		return Reset(rhs.Release());
	}
	template<typename OtherT,
		std::enable_if_t<std::is_array<OtherT>::value
			? std::is_same<std::remove_cv_t<std::remove_extent_t<OtherT>>, std::remove_cv_t<Element>>::value
			: std::is_convertible<OtherT *, Element *>::value,
		int> = 0>
	UniquePtr &Reset(UniquePtr<OtherT, DeleterT> rhs) noexcept {
		return Reset(rhs.Release());
	}

	void Swap(UniquePtr &rhs) noexcept {
		std::swap(xm_pElement, rhs.xm_pElement);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator Element *() const noexcept {
		return Get();
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
