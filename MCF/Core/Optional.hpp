// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_OPTIONAL_HPP_
#define MCF_CORE_OPTIONAL_HPP_

#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/TupleManipulators.hpp"
#include "../Utilities/AddressOf.hpp"
#include "Exception.hpp"
#include <exception>
#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl_Optional {
	enum class State : unsigned char {
		kUnset        = 0,
		kElementSet   = 1,
		kExceptionSet = 2,
	};
}

template<typename ElementT>
class Optional {
private:
	union X_Storage {
		ElementT v;
		std::exception_ptr ep;

		X_Storage(){
		}
		~X_Storage(){
		}
	};

private:
	X_Storage x_vStorage;
	Impl_Optional::State x_eState;

public:
	Optional() noexcept
		: x_eState(Impl_Optional::State::kUnset)
	{
	}
	template<typename ...ParamsT>
	Optional(std::tuple<ParamsT...> vParamTuple) noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
		: Optional()
	{
		SqueezeTuple([this](auto &&...vParams){ this->ResetElement(static_cast<decltype(vParams) &&>(vParams)...); }, vParamTuple);
	}
	Optional(std::exception_ptr rhs) noexcept
		: Optional()
	{
		ResetException(std::move(rhs));
	}
	Optional(const Optional &rhs) noexcept(std::is_nothrow_copy_constructible<ElementT>::value)
		: Optional()
	{
		Reset(rhs);
	}
	Optional(Optional &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value)
		: Optional()
	{
		Reset(std::move(rhs));
	}
	Optional &operator=(const Optional &rhs) noexcept(std::is_nothrow_copy_constructible<ElementT>::value) {
		Reset(rhs);
		return *this;
	}
	Optional &operator=(Optional &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value) {
		Reset(std::move(rhs));
		return *this;
	}
	~Optional(){
		Reset();
	}

public:
	bool IsSet() const noexcept {
		return x_eState != Impl_Optional::State::kUnset;
	}
	bool IsElementSet() const noexcept {
		return x_eState == Impl_Optional::State::kElementSet;
	}
	bool IsExceptionSet() const noexcept {
		return x_eState == Impl_Optional::State::kExceptionSet;
	}

	std::exception_ptr GetException() const noexcept {
		if(x_eState == Impl_Optional::State::kElementSet){
			return std::exception_ptr();
		} else if(x_eState == Impl_Optional::State::kExceptionSet){
			return x_vStorage.ep;
		} else {
			return DEBUG_MAKE_EXCEPTION_PTR(Exception, ERROR_NOT_READY, "Optional is not set"_rcs);
		}
	}

	const ElementT &Get() const {
		const auto pException = GetException();
		if(pException){
			std::rethrow_exception(pException);
		}
		return x_vStorage.v;
	}
	ElementT &Get(){
		const auto pException = GetException();
		if(pException){
			std::rethrow_exception(pException);
		}
		return x_vStorage.v;
	}

	Optional &Reset() noexcept {
		if(x_eState == Impl_Optional::State::kElementSet){
			Destruct(AddressOf(x_vStorage.v));
			x_eState = Impl_Optional::State::kUnset;
		} else if(x_eState == Impl_Optional::State::kExceptionSet){
			Destruct(AddressOf(x_vStorage.ep));
			x_eState = Impl_Optional::State::kUnset;
		}
		return *this;
	}
	Optional &Reset(const Optional &rhs) noexcept(std::is_nothrow_copy_constructible<ElementT>::value) {
		Reset();

		if(rhs.x_eState == Impl_Optional::State::kElementSet){
			ResetElement(rhs.x_vStorage.v);
		} else if(rhs.x_eState == Impl_Optional::State::kExceptionSet){
			ResetException(rhs.x_vStorage.ep);
		}
		return *this;
	}
	Optional &Reset(Optional &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value) {
		Reset();

		if(rhs.x_eState == Impl_Optional::State::kElementSet){
			ResetElement(std::move(rhs.x_vStorage.v));
			rhs.Reset();
		} else if(rhs.x_eState == Impl_Optional::State::kExceptionSet){
			ResetException(std::move(rhs.x_vStorage.ep));
			rhs.Reset();
		}
		return *this;
	}
	template<typename ...ParamsT>
	Optional &ResetElement(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value) {
		Reset();

		Construct(AddressOf(x_vStorage.v), std::forward<ParamsT>(vParams)...);
		x_eState = Impl_Optional::State::kElementSet;
		return *this;
	}
	Optional &ResetException(std::exception_ptr rhs) noexcept {
		Reset();

		Construct(AddressOf(x_vStorage.ep), std::move(rhs));
		x_eState = Impl_Optional::State::kExceptionSet;
		return *this;
	}

public:
	explicit operator bool() const noexcept {
		return IsElementSet();
	}
};

}

#endif
