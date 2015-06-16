// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_REFERENCE_WRAPPER_HPP_
#define MCF_FUNCTION_REFERENCE_WRAPPER_HPP_

#include <type_traits>
#include <utility>
#include <memory>

namespace MCF {

template<typename ObjectT>
class ReferenceWrapper {
private:
	std::remove_reference_t<ObjectT> *x_pObject;

public:
	constexpr ReferenceWrapper(ObjectT &vObject) noexcept
		: x_pObject(std::addressof(vObject))
	{
	}

public:
	ObjectT &Get() const noexcept {
		return *x_pObject;
	}
	ObjectT &Forward() const noexcept {
		return *x_pObject;
	}

public:
	operator ObjectT &() const noexcept {
		return Get();
	}

	template<typename ...ParamsT>
	decltype(auto) operator()(ParamsT &&...vParams) const {
		return Forward()(std::forward<ParamsT>(vParams)...);
	}
};

template<typename ObjectT>
class ReferenceWrapper<ObjectT &&> {
private:
	std::remove_reference_t<ObjectT> *x_pObject;

public:
	constexpr ReferenceWrapper(ObjectT &&vObject) noexcept
		: x_pObject(std::addressof(vObject))
	{
	}

public:
	ObjectT &Get() const noexcept {
		return *x_pObject;
	}
	ObjectT &&Forward() const noexcept {
		return std::move(*x_pObject);
	}

public:
	operator ObjectT &() const noexcept {
		return Get();
	}

	template<typename ...ParamsT>
	decltype(auto) operator()(ParamsT &&...vParams) const {
		return Forward()(std::forward<ParamsT>(vParams)...);
	}
};

template<typename ObjectT>
ReferenceWrapper<const volatile ObjectT> Ref(const volatile ObjectT &vObject) noexcept {
	return ReferenceWrapper<const volatile ObjectT>(vObject);
}
template<typename ObjectT>
ReferenceWrapper<const ObjectT> Ref(const ObjectT &vObject) noexcept {
	return ReferenceWrapper<const ObjectT>(vObject);
}
template<typename ObjectT>
ReferenceWrapper<volatile ObjectT> Ref(volatile ObjectT &vObject) noexcept {
	return ReferenceWrapper<volatile ObjectT>(vObject);
}
template<typename ObjectT>
ReferenceWrapper<ObjectT> Ref(ObjectT &vObject) noexcept {
	return ReferenceWrapper<ObjectT>(vObject);
}

template<typename ObjectT>
ReferenceWrapper<const volatile ObjectT &&> Ref(const volatile ObjectT &&vObject) noexcept = delete;
template<typename ObjectT>
ReferenceWrapper<const ObjectT &&> Ref(const ObjectT &&vObject) noexcept = delete;
template<typename ObjectT>
ReferenceWrapper<volatile ObjectT &&> Ref(volatile ObjectT &&vObject) noexcept = delete;
template<typename ObjectT>
ReferenceWrapper<ObjectT &&> Ref(ObjectT &&vObject) noexcept = delete;

}

#endif
