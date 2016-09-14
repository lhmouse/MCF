// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VARIANT_HPP_
#define MCF_CORE_VARIANT_HPP_

#include "AlignedStorage.hpp"
#include "Exception.hpp"
#include "ConstructDestruct.hpp"
#include <type_traits>
#include <tuple>
#include <cstddef>

namespace MCF {

template<typename ...ElementsT>
class Variant {
private:
	using X_TypeTuple = std::tuple<ElementsT...>;

private:
	AlignedStorage<ElementsT...> x_vStorage;
	int x_nActiveIndex;

public:
	Variant() noexcept
		: x_nActiveIndex(-1)
	{
	}
	Variant(const Variant &rhs) noexcept((std::is_nothrow_copy_constructible<ElementsT>::value && ...))
		: Variant()
	{
		//
	}
	Variant(Variant &&rhs) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...))
		: Variant()
	{
		//
	}
	Variant(const Variant &rhs) noexcept((std::is_nothrow_copy_constructible<ElementsT>::value && ...)) {
		//
		return *this;
	}
	Variant(Variant &&rhs) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...)) {
		//
		return *this;
	}
	~Variant(){
		Clear();
	}

public:
	int GetActiveIndex() const noexcept {
		return x_nActiveIndex;
	}
	void Clear() noexcept {
		int i = 0;
		(... || ((x_nActiveIndex == i++) && (static_cast<void>(reinterpret_cast<ElementsT *>(&x_vStorage)->~ElementsT()), true)));
		x_nActiveIndex = -1;
	}
	template<unsigned kIndexT>
	const std::tuple_element_t<kIndexT, X_TypeTuple> *Get() const noexcept {
		if(static_cast<unsigned>(x_nActiveIndex) != kIndexT){
			return nullptr;
		}
		return reinterpret_cast<const std::tuple_element_t<kIndexT, X_TypeTuple> *>(&x_vStorage);
	}
	template<unsigned kIndexT>
	std::tuple_element_t<kIndexT, X_TypeTuple> *Get() noexcept {
		if(static_cast<unsigned>(x_nActiveIndex) != kIndexT){
			return nullptr;
		}
		return reinterpret_cast<std::tuple_element_t<kIndexT, X_TypeTuple> *>(&x_vStorage);
	}
//	template<typename ElementT>
//	const ElementT *Get() const noexcept {
//		if(static_cast<unsigned>(x_nActiveIndex) != kIndexT){
//			return nullptr;
//		}
//		return static_cast<const std::tuple_element_t<kIndexT, X_TypeTuple> *>(static_cast<const void *>(&x_vStorage));
//	}
//	template<typename ElementT>
//	ElementT *Get() noexcept {
//		if(static_cast<unsigned>(x_nActiveIndex) != kIndexT){
//			return nullptr;
//		}
//		return static_cast<std::tuple_element_t<kIndexT, X_TypeTuple> *>(static_cast<void *>(&x_vStorage));
//	}
	template<unsigned kIndexT>
	const std::tuple_element_t<kIndexT, X_TypeTuple> &Require() const noexcept {
		const auto pElement = Get<kIndexT>();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Variant: 未设定活动元素或活动元素类型不匹配。"));
		}
		return *pElement;
	}
	template<unsigned kIndexT>
	std::tuple_element_t<kIndexT, X_TypeTuple> *Get() noexcept {
		const auto pElement = Get<kIndexT>();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Variant: 未设定活动元素或活动元素类型不匹配。"));
		}
		return *pElement;
	}
	template<typename ElementT>
	const ElementT &Require() const noexcept {
		const auto pElement = Get<ElementT>();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Variant: 未设定活动元素或活动元素类型不匹配。"));
		}
		return *pElement;
	}
	template<typename ElementT>
	ElementT &Require() noexcept {
		const auto pElement = Get<ElementT>();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Variant: 未设定活动元素或活动元素类型不匹配。"));
		}
		return *pElement;
	}
	template<typename ElementT>
	void Set(ElementT &&vElement){
		Clear();

		//
	}

	void Swap(Function &rhs) noexcept {
		//
	}

public:
	friend void swap(Function &lhs, Function &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
