// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_OPTIONAL_HPP_
#define MCF_CORE_OPTIONAL_HPP_

#include "../Utilities/ConstructDestruct.hpp"
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
	Optional(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
		: Optional()
	{
		ResetElement(std::forward<ParamsT>(vParams)...);
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

	const ElementT &Get() const {
		if(x_eState == Impl_Optional::State::kElementSet){
			return *reinterpret_cast<const ElementT *>(&x_vStorage);
		} else if(x_eState == Impl_Optional::State::kExceptionSet){
			std::rethrow_exception(*reinterpret_cast<const std::exception_ptr *>(&x_vStorage));
		} else {
			DEBUG_THROW(Exception, ERROR_NOT_READY, "Optional is not set"_rcs);
		}
	}
	ElementT &Get(){
		if(x_eState == Impl_Optional::State::kElementSet){
			return *reinterpret_cast<ElementT *>(&x_vStorage);
		} else if(x_eState == Impl_Optional::State::kExceptionSet){
			std::rethrow_exception(*reinterpret_cast<const std::exception_ptr *>(&x_vStorage));
		} else {
			DEBUG_THROW(Exception, ERROR_NOT_READY, "Optional is not set"_rcs);
		}
	}

	Optional &Reset() noexcept {
		if(x_eState == Impl_Optional::State::kElementSet){
			Destruct(reinterpret_cast<ElementT *>(&x_vStorage));
			x_eState = Impl_Optional::State::kUnset;
		} else if(x_eState == Impl_Optional::State::kExceptionSet){
			Destruct(reinterpret_cast<std::exception_ptr *>(&x_vStorage));
			x_eState = Impl_Optional::State::kUnset;
		}
		return *this;
	}
	Optional &Reset(const Optional &rhs) noexcept(std::is_nothrow_copy_constructible<ElementT>::value) {
		Reset();

		if(x_eState == Impl_Optional::State::kElementSet){
			ResetElement(*reinterpret_cast<const ElementT *>(&rhs.x_vStorage));
		} else if(x_eState == Impl_Optional::State::kExceptionSet){
			ResetException(*reinterpret_cast<const std::exception_ptr *>(&rhs.x_vStorage));
		}
		return *this;
	}
	Optional &Reset(Optional &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value) {
		Reset();

		if(x_eState == Impl_Optional::State::kElementSet){
			ResetElement(std::move(*reinterpret_cast<ElementT *>(&rhs.x_vStorage)));
		} else if(x_eState == Impl_Optional::State::kExceptionSet){
			ResetException(std::move(*reinterpret_cast<const std::exception_ptr *>(&rhs.x_vStorage)));
		}
		return *this;
	}
	template<typename ...ParamsT>
	Optional &ResetElement(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value) {
		Reset();

		Construct(reinterpret_cast<ElementT *>(&x_vStorage), std::forward<ParamsT>(vParams)...);
		x_eState = Impl_Optional::State::kElementSet;
		return *this;
	}
	Optional &ResetException(std::exception_ptr rhs) noexcept {
		Reset();

		Construct(reinterpret_cast<std::exception_ptr *>(&x_vStorage), std::move(rhs));
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
