// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_OPTIONAL_HPP_
#define MCF_CORE_OPTIONAL_HPP_

#include "Variant.hpp"
#include "AddressOf.hpp"
#include <exception>

namespace MCF {

template<typename ElementT>
class Optional {
private:
	Variant<ElementT, std::exception_ptr> x_vData;

public:
	Optional() noexcept { }

	template<typename vOtherT,
		std::enable_if_t<
			!std::is_base_of<Optional, std::decay_t<vOtherT>>::value,
			int> = 0>
	Optional(vOtherT &&vOther){
		Reset(std::move(vOther));
	}
	Optional(std::exception_ptr pException) noexcept
		: x_vData(std::move(pException))
	{ }

public:
	bool IsSet() const noexcept {
		return x_vData.GetActiveIndex() >= 0;
	}
	bool IsElementSet() const noexcept {
		return x_vData.GetActiveIndex() == 0;
	}
	bool IsExceptionSet() const noexcept {
		return x_vData.GetActiveIndex() == 1;
	}

	const ElementT *Get() const {
		const auto ppException = x_vData.template Get<1>();
		if(ppException){
			std::rethrow_exception(*ppException);
		}
		return x_vData.template Get<0>();
	}
	ElementT *Get(){
		const auto ppException = x_vData.template Get<1>();
		if(ppException){
			std::rethrow_exception(*ppException);
		}
		return x_vData.template Get<0>();
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
	std::exception_ptr GetException() const noexcept {
		const auto ppException = x_vData.template Get<1>();
		if(!ppException){
			return { };
		}
		return *ppException;
	}

	Optional &Reset() noexcept {
		x_vData.Reset();
		return *this;
	}
	template<typename vOtherT>
	Optional &Reset(vOtherT &&vOther){
		x_vData.template Reset<0>(std::forward<vOtherT>(vOther));
		return *this;
	}
	Optional &Reset(std::exception_ptr pException) noexcept {
		x_vData.template Reset<1>(std::move(pException));
		return *this;
	}

public:
	explicit operator bool() const noexcept {
		return IsSet();
	}
};

template<typename ElementT>
class Optional<ElementT &> {
private:
	Variant<ElementT *, std::exception_ptr> x_vData;

public:
	Optional() noexcept { }

	Optional(ElementT &vElement) noexcept {
		Reset(vElement);
	}
	Optional(std::exception_ptr pException) noexcept {
		Reset(std::move(pException));
	}

public:
	bool IsSet() const noexcept {
		return x_vData.GetActiveIndex() >= 0;
	}
	bool IsElementSet() const noexcept {
		return x_vData.GetActiveIndex() == 0;
	}
	bool IsExceptionSet() const noexcept {
		return x_vData.GetActiveIndex() == 1;
	}

	ElementT &Get() const {
		const auto ppException = x_vData.template Get<1>();
		if(ppException){
			std::rethrow_exception(*ppException);
		}
		return *x_vData.template Get<0>();
	}
	ElementT &Require() const {
		return Get();
	}
	std::exception_ptr GetException() const noexcept {
		const auto ppException = x_vData.template Get<1>();
		if(!ppException){
			return { };
		}
		return *ppException;
	}

	Optional &Reset() noexcept {
		x_vData.Reset();
		return *this;
	}
	Optional &Reset(ElementT &vElement) noexcept {
		x_vData.template Reset<0>(AddressOf(vElement));
		return *this;
	}
	Optional &Reset(std::exception_ptr pException) noexcept {
		x_vData.template Reset<1>(std::move(pException));
		return *this;
	}

public:
	explicit operator bool() const noexcept {
		return IsSet();
	}
};

template<typename ElementT>
class Optional<ElementT &&> {
	static_assert((sizeof(ElementT *), false), "Class template ElementT instantiated with an rvalue reference type parameter.");
};

}

#endif
