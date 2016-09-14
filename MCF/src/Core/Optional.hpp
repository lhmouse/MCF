// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_OPTIONAL_HPP_
#define MCF_CORE_OPTIONAL_HPP_

#include "ConstructDestruct.hpp"
#include "AlignedStorage.hpp"
#include "Exception.hpp"
#include "TupleManipulators.hpp"
#include <exception>
#include <type_traits>
#include <utility>

namespace MCF {

template<typename ElementT>
class Optional {
private:
	enum X_State : unsigned char {
		xkUnset        = 0,
		xkElementSet   = 1,
		xkExceptionSet = 2,
	};

private:
	AlignedStorage<ElementT, std::exception_ptr> x_vStorage;
	X_State x_eState;

public:
	Optional() noexcept
		: x_eState(xkUnset)
	{
	}
	template<typename ...ParamsT>
	Optional(std::tuple<ParamsT...> tupParams) noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
		: Optional()
	{
		Reset(std::move(tupParams));
	}
	Optional(std::exception_ptr rhs) noexcept
		: Optional()
	{
		Reset(std::move(rhs));
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
		return Reset(rhs);
	}
	Optional &operator=(Optional &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value) {
		return Reset(std::move(rhs));
	}
	~Optional(){
		Reset();
	}

public:
	bool IsSet() const noexcept {
		return x_eState != xkUnset;
	}
	bool IsElementSet() const noexcept {
		return x_eState == xkElementSet;
	}
	bool IsExceptionSet() const noexcept {
		return x_eState == xkExceptionSet;
	}

	std::exception_ptr GetException() const noexcept {
		if(x_eState == xkElementSet){
			return std::exception_ptr();
		} else if(x_eState == xkExceptionSet){
			return *reinterpret_cast<const std::exception_ptr *>(x_vStorage);
		} else {
			return MCF_MAKE_EXCEPTION_PTR(Exception, ERROR_NOT_READY, Rcntws::View(L"Optional: 尚未设定元素或异常对象。"));
		}
	}

	const ElementT *Get() const {
		if(x_eState == xkElementSet){
			return reinterpret_cast<const ElementT *>(x_vStorage);
		} else if(x_eState == xkExceptionSet){
			std::rethrow_exception(*reinterpret_cast<const std::exception_ptr *>(x_vStorage));
		} else {
			return nullptr;
		}
	}
	ElementT *Get(){
		if(x_eState == xkElementSet){
			return reinterpret_cast<ElementT *>(x_vStorage);
		} else if(x_eState == xkExceptionSet){
			std::rethrow_exception(*reinterpret_cast<const std::exception_ptr *>(x_vStorage));
		} else {
			return nullptr;
		}
	}
	const ElementT *Require() const {
		const auto pElement = Get();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Optional: 尚未设定元素。"));
		}
		return pElement;
	}
	ElementT *Require(){
		const auto pElement = Get();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Optional: 尚未设定元素。"));
		}
		return pElement;
	}

	Optional &Reset() noexcept {
		if(x_eState == xkElementSet){
			Destruct(reinterpret_cast<ElementT *>(x_vStorage));
			x_eState = xkUnset;
		} else if(x_eState == xkExceptionSet){
			Destruct(reinterpret_cast<std::exception_ptr *>(x_vStorage));
			x_eState = xkUnset;
		}
		return *this;
	}
	Optional &Reset(const Optional &rhs) noexcept(std::is_nothrow_copy_constructible<ElementT>::value) {
		Reset();

		if(rhs.x_eState == xkElementSet){
			Reset(std::forward_as_tuple(*reinterpret_cast<const ElementT *>(rhs.x_vStorage)));
			x_eState = xkUnset;
		} else if(rhs.x_eState == xkExceptionSet){
			Reset(*reinterpret_cast<const std::exception_ptr *>(rhs.x_vStorage));
			x_eState = xkUnset;
		}
		return *this;
	}
	Optional &Reset(Optional &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value) {
		Reset();

		if(rhs.x_eState == xkElementSet){
			Reset(std::forward_as_tuple(std::move(*reinterpret_cast<ElementT *>(rhs.x_vStorage))));
			x_eState = xkUnset;
		} else if(rhs.x_eState == xkExceptionSet){
			Reset(std::move(*reinterpret_cast<std::exception_ptr *>(rhs.x_vStorage)));
			x_eState = xkUnset;
		}
		return *this;
	}
	template<typename ...ParamsT>
	Optional &Reset(std::tuple<ParamsT...> tupParams) noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value) {
		Reset();

		Squeeze([this](auto &&...vParams){ Construct(reinterpret_cast<ElementT *>(x_vStorage), std::forward<ParamsT>(vParams)...); }, std::move(tupParams));
		x_eState = xkElementSet;
		return *this;
	}
	Optional &Reset(std::exception_ptr rhs) noexcept {
		Reset();

		Construct(reinterpret_cast<std::exception_ptr *>(x_vStorage), std::move(rhs));
		x_eState = xkExceptionSet;
		return *this;
	}

public:
	explicit operator bool() const noexcept {
		return IsSet();
	}
};

}

#endif
