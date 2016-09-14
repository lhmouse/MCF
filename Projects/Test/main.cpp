#include <MCF/StdMCF.hpp>
#include <MCF/Core/AlignedStorage.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/ConstructDestruct.hpp>
#include <cstdio>

// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

//#ifndef MCF_CORE_VARIANT_HPP_
#if 0
#define MCF_CORE_VARIANT_HPP_

//#include "AlignedStorage.hpp"
//#include "Exception.hpp"
//#include "ConstructDestruct.hpp"
#include <type_traits>
#include <tuple>
#include <cstddef>

namespace MCF {

template<typename ...ElementsT>
class Variant {
private:
	using X_TypeTuple = std::tuple<ElementsT...>;

private:
	AlignedStorage<1, ElementsT...> x_vStorage;
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
	Variant &operator=(const Variant &rhs) noexcept((std::is_nothrow_copy_constructible<ElementsT>::value && ...)) {
		//
		return *this;
	}
	Variant &operator=(Variant &&rhs) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...)) {
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
		const auto nActiveIndex = x_nActiveIndex;
		x_nActiveIndex = -1;
		//
	}
	template<unsigned kIndexT>
	const std::tuple_element_t<kIndexT, X_TypeTuple> *Get() const noexcept {
		if(static_cast<unsigned>(x_nActiveIndex) != kIndexT){
			return nullptr;
		}
		return static_cast<const std::tuple_element_t<kIndexT, X_TypeTuple> *>(static_cast<const void *>(&x_vStorage));
	}
	template<unsigned kIndexT>
	std::tuple_element_t<kIndexT, X_TypeTuple> *Get() noexcept {
		if(static_cast<unsigned>(x_nActiveIndex) != kIndexT){
			return nullptr;
		}
		return static_cast<std::tuple_element_t<kIndexT, X_TypeTuple> *>(static_cast<void *>(&x_vStorage));
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
	std::tuple_element_t<kIndexT, X_TypeTuple> *Require() noexcept {
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

	void Swap(Variant &rhs) noexcept(std::is_nothrow_move_constructible<Variant>::value) {
		auto temp = std::move_if_noexcept(*this);
		*this = std::move(rhs);
		rhs = std::move(temp);
	}

public:
	friend void swap(Variant &lhs, Variant &rhs) noexcept(noexcept(lhs.Swap(rhs))) {
		lhs.Swap(rhs);
	}
};

}

#endif

//template class MCF::Variant<int, long, double>;
#include <MCF/Core/Optional.hpp>
template class MCF::Optional<int>;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
//	MCF::Variant<int, long, double> v;
//	auto p = v.Get<2>();
//	std::printf("p = %p\n", (void *)p);
	return 0;
}
