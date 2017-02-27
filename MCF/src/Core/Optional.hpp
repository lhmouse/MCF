// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_OPTIONAL_HPP_
#define MCF_CORE_OPTIONAL_HPP_

#include "Variant.hpp"
#include <exception>

namespace MCF {

template<typename ElementT>
class Optional {
private:
	Variant<ElementT, std::exception_ptr> x_vData;

public:
	Optional() noexcept
		: x_vData()
	{
	}
	template<typename ParamT,
		std::enable_if_t<
			!std::is_base_of<Optional, std::decay_t<ParamT>>::value,
			int> = 0>
	Optional(ParamT &&vOther)
		: x_vData(std::move(vOther))
	{
	}
	Optional(std::exception_ptr pException) noexcept
		: x_vData(std::move(pException))
	{
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

	const ElementT *Get() const {
		const auto pException = x_vData.template Get<1>();
		if(pException){
			std::rethrow_exception(*pException);
		}
		return x_vData.template Get<0>();
	}
	ElementT *Get(){
		const auto pException = x_vData.template Get<1>();
		if(pException){
			std::rethrow_exception(*pException);
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
		const auto pException = x_vData.template Get<1>();
		if(!pException){
			return { };
		}
		return *pException;
	}

	Optional &Reset() noexcept {
		x_vData.Clear();
		return *this;
	}
	Optional &Reset(ElementT vElement){
		x_vData.template Set<0>(std::move(vElement));
		return *this;
	}
	Optional &Reset(std::exception_ptr pException) noexcept {
		x_vData.template Set<1>(std::move(pException));
		return *this;
	}

public:
	explicit operator bool() const noexcept {
		return IsSet();
	}
};

}

#endif
