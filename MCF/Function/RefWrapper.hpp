// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_REF_WRAPPER_HPP_
#define MCF_FUNCTION_REF_WRAPPER_HPP_

#include <type_traits>
#include <utility>
#include <memory>

namespace MCF {

template<typename ObjectT>
class RefWrapper {
public:
	using Reference = ObjectT &;

private:
	std::remove_reference_t<ObjectT> *x_pObject;

public:
	constexpr RefWrapper(ObjectT &vObject) noexcept
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
		return Get()(std::forward<ParamsT>(vParams)...);
	}
};

template<typename ObjectT>
class RefWrapper<ObjectT &&> {
public:
	using Reference = ObjectT &&;

private:
	std::remove_reference_t<ObjectT> *x_pObject;

public:
	constexpr RefWrapper(ObjectT &vObject) noexcept
		: x_pObject(std::addressof(vObject))
	{
	}
	constexpr RefWrapper(ObjectT &&vObject) noexcept
		: RefWrapper(vObject)
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
	operator ObjectT &() const & noexcept {
		return Get();
	}
	operator ObjectT &&() const && noexcept {
		return Forward();
	}

	template<typename ...ParamsT>
	decltype(auto) operator()(ParamsT &&...vParams) const & {
		return Get()(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	decltype(auto) operator()(ParamsT &&...vParams) const && {
		return Forward()(std::forward<ParamsT>(vParams)...);
	}
};

template<typename ObjectT>
RefWrapper<const volatile ObjectT> Ref(const volatile ObjectT &vObject) noexcept {
	return RefWrapper<const volatile ObjectT>(vObject);
}
template<typename ObjectT>
RefWrapper<const ObjectT> Ref(const ObjectT &vObject) noexcept {
	return RefWrapper<const ObjectT>(vObject);
}
template<typename ObjectT>
RefWrapper<volatile ObjectT> Ref(volatile ObjectT &vObject) noexcept {
	return RefWrapper<volatile ObjectT>(vObject);
}
template<typename ObjectT>
RefWrapper<ObjectT> Ref(ObjectT &vObject) noexcept {
	return RefWrapper<ObjectT>(vObject);
}

template<typename ObjectT>
RefWrapper<const volatile ObjectT &&> Ref(const volatile ObjectT &&vObject) noexcept = delete;
template<typename ObjectT>
RefWrapper<const ObjectT &&> Ref(const ObjectT &&vObject) noexcept = delete;
template<typename ObjectT>
RefWrapper<volatile ObjectT &&> Ref(volatile ObjectT &&vObject) noexcept = delete;
template<typename ObjectT>
RefWrapper<ObjectT &&> Ref(ObjectT &&vObject) noexcept = delete;

}

#endif
