// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_REF_WRAPPER_HPP_
#define MCF_CORE_REF_WRAPPER_HPP_

#include "AddressOf.hpp"
#include <type_traits>
#include <utility>

namespace MCF {

template<typename ObjectT>
class RefWrapper {
private:
	std::remove_reference_t<ObjectT> *x_pObject;

public:
	constexpr RefWrapper(ObjectT &vObject) noexcept
		: x_pObject(AddressOf(vObject))
	{
	}

public:
	constexpr ObjectT &Get() const noexcept {
		return *x_pObject;
	}

	void Swap(RefWrapper &rhs) noexcept {
		using std::swap;
		swap(x_pObject, rhs.x_pObject);
	}

public:
	constexpr operator ObjectT &() const noexcept {
		return Get();
	}

	template<typename ...ParamsT>
	decltype(auto) operator()(ParamsT &&...vParams) const {
		return Get()(std::forward<ParamsT>(vParams)...);
	}

	constexpr bool operator==(const RefWrapper &rhs) const {
		return Get() == rhs.Get();
	}
	constexpr bool operator==(const ObjectT &rhs) const {
		return Get() == rhs;
	}
	friend constexpr bool operator==(const ObjectT &lhs, const RefWrapper &rhs){
		return lhs == rhs.Get();
	}

	constexpr bool operator!=(const RefWrapper &rhs) const {
		return Get() != rhs.Get();
	}
	constexpr bool operator!=(const ObjectT &rhs) const {
		return Get() != rhs;
	}
	friend constexpr bool operator!=(const ObjectT &lhs, const RefWrapper &rhs){
		return lhs != rhs.Get();
	}

	constexpr bool operator<(const RefWrapper &rhs) const {
		return Get() < rhs.Get();
	}
	constexpr bool operator<(const ObjectT &rhs) const {
		return Get() < rhs;
	}
	friend constexpr bool operator<(const ObjectT &lhs, const RefWrapper &rhs){
		return lhs < rhs.Get();
	}

	constexpr bool operator>(const RefWrapper &rhs) const {
		return Get() > rhs.Get();
	}
	constexpr bool operator>(const ObjectT &rhs) const {
		return Get() > rhs;
	}
	friend constexpr bool operator>(const ObjectT &lhs, const RefWrapper &rhs){
		return lhs > rhs.Get();
	}

	constexpr bool operator<=(const RefWrapper &rhs) const {
		return Get() <= rhs.Get();
	}
	constexpr bool operator<=(const ObjectT &rhs) const {
		return Get() <= rhs;
	}
	friend constexpr bool operator<=(const ObjectT &lhs, const RefWrapper &rhs){
		return lhs <= rhs.Get();
	}

	constexpr bool operator>=(const RefWrapper &rhs) const {
		return Get() >= rhs.Get();
	}
	constexpr bool operator>=(const ObjectT &rhs) const {
		return Get() >= rhs;
	}
	friend constexpr bool operator>=(const ObjectT &lhs, const RefWrapper &rhs){
		return lhs >= rhs.Get();
	}

	friend void swap(RefWrapper &lhs, RefWrapper &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

template<typename ObjectT>
constexpr RefWrapper<const volatile ObjectT> Ref(const volatile ObjectT &vObject) noexcept {
	return RefWrapper<const volatile ObjectT>(vObject);
}
template<typename ObjectT>
void Ref(const volatile ObjectT &&vObject) noexcept = delete;

template<typename ObjectT>
constexpr RefWrapper<const ObjectT> Ref(const ObjectT &vObject) noexcept {
	return RefWrapper<const ObjectT>(vObject);
}
template<typename ObjectT>
void Ref(const ObjectT &&vObject) noexcept = delete;

template<typename ObjectT>
constexpr RefWrapper<volatile ObjectT> Ref(volatile ObjectT &vObject) noexcept {
	return RefWrapper<volatile ObjectT>(vObject);
}
template<typename ObjectT>
void Ref(volatile ObjectT &&vObject) noexcept = delete;

template<typename ObjectT>
constexpr RefWrapper<ObjectT> Ref(ObjectT &vObject) noexcept {
	return RefWrapper<ObjectT>(vObject);
}
template<typename ObjectT>
void Ref(ObjectT &&vObject) noexcept = delete;

}

#endif
