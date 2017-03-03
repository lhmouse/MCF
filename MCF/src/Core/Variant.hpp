// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VARIANT_HPP_
#define MCF_CORE_VARIANT_HPP_

#include "AlignedStorage.hpp"
#include "Exception.hpp"
#include "ConstructDestruct.hpp"
#include <type_traits>
#include <utility>
#include <tuple>
#include <cstddef>

namespace MCF {

namespace Impl_Variant {
	template<typename CvVoidT, typename VisitorT, typename... CvElementsT, std::size_t ...kIndicesT>
	void VisitPointer(CvVoidT *pStorage, unsigned uActiveIndex, VisitorT &vVisitor, const std::index_sequence<kIndicesT...> &){
		(void)(... && !((uActiveIndex == kIndicesT) &&
			((void)std::forward<VisitorT>(vVisitor)(std::integral_constant<unsigned, kIndicesT>(), static_cast<CvElementsT *>(pStorage)), true)));
	}

	template<typename TargetT, typename FirstT, typename ...RestT>
	struct IndexByType
		: std::integral_constant<unsigned, IndexByType<TargetT, RestT...>::value + 1>
	{
	};
	template<typename TargetT, typename ...RestT>
	struct IndexByType<TargetT, TargetT, RestT...>
		: std::integral_constant<unsigned, 0>
	{
	};
}

template<typename ...ElementsT>
class Variant {
private:
	using X_TypeTuple = std::tuple<ElementsT...>;

private:
	AlignedStorage<ElementsT...> x_vStorage;
	int x_nActiveIndex = -1;

private:
	template<typename VisitorT>
	void X_VisitPointer(VisitorT &&vVisitor) const {
		Impl_Variant::VisitPointer<const void, VisitorT, const ElementsT...>(
			&x_vStorage, static_cast<unsigned>(x_nActiveIndex), vVisitor, std::index_sequence_for<ElementsT...>());
	}
	template<typename VisitorT>
	void X_VisitPointer(VisitorT &&vVisitor){
		Impl_Variant::VisitPointer<void, VisitorT, ElementsT...>(
			&x_vStorage, static_cast<unsigned>(x_nActiveIndex), vVisitor, std::index_sequence_for<ElementsT...>());
	}

public:
	Variant() noexcept {
	}
	template<typename vOtherT,
		std::enable_if_t<
			!std::is_base_of<Variant, std::decay_t<vOtherT>>::value,
			int> = 0>
	Variant(vOtherT &&vOther){
		Reset(std::forward<vOtherT>(vOther));
	}
	Variant(const Variant &vOther) noexcept((std::is_nothrow_copy_constructible<ElementsT>::value && ...)) {
		vOther.X_VisitPointer([this](auto k, auto p){ this->Reset<k>(*p); });
	}
	Variant(Variant &&vOther) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...)) {
		vOther.X_VisitPointer([this](auto k, auto p){ this->Reset<k>(std::move(*p)); });
	}
	Variant &operator=(const Variant &vOther) noexcept((std::is_nothrow_copy_constructible<ElementsT>::value && ...)) {
		vOther.X_VisitPointer([this](auto k, auto p){ this->Reset<k>(*p); });
		return *this;
	}
	Variant &operator=(Variant &&vOther) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...)) {
		vOther.X_VisitPointer([this](auto k, auto p){ this->Reset<k>(std::move(*p)); });
		return *this;
	}
	~Variant(){
		Reset();
	}

public:
	int GetActiveIndex() const noexcept {
		return x_nActiveIndex;
	}

	template<unsigned kIndexT>
	const std::tuple_element_t<kIndexT, X_TypeTuple> *Get() const noexcept {
		const auto uActiveIndex = static_cast<unsigned>(x_nActiveIndex);
		if(uActiveIndex != kIndexT){
			return nullptr;
		}
		const void *const pElementRaw = &x_vStorage;
		return static_cast<const std::tuple_element_t<kIndexT, X_TypeTuple> *>(pElementRaw);
	}
	template<unsigned kIndexT>
	std::tuple_element_t<kIndexT, X_TypeTuple> *Get() noexcept {
		const auto uActiveIndex = static_cast<unsigned>(x_nActiveIndex);
		if(uActiveIndex != kIndexT){
			return nullptr;
		}
		void *const pElementRaw = &x_vStorage;
		return static_cast<std::tuple_element_t<kIndexT, X_TypeTuple> *>(pElementRaw);
	}
	template<typename ElementT>
	const ElementT *Get() const noexcept {
		return Get<Impl_Variant::IndexByType<std::decay_t<ElementT>, ElementsT...>::value>();
	}
	template<typename ElementT>
	ElementT *Get() noexcept {
		return Get<Impl_Variant::IndexByType<std::decay_t<ElementT>, ElementsT...>::value>();
	}
	template<unsigned kIndexT>
	const std::tuple_element_t<kIndexT, X_TypeTuple> &Require() const {
		const auto pElement = Get<kIndexT>();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Variant: 未设定活动元素或活动元素类型不匹配。"));
		}
		return pElement;
	}
	template<unsigned kIndexT>
	std::tuple_element_t<kIndexT, X_TypeTuple> *Require(){
		const auto pElement = Get<kIndexT>();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Variant: 未设定活动元素或活动元素类型不匹配。"));
		}
		return pElement;
	}
	template<typename ElementT>
	const ElementT *Require() const {
		const auto pElement = Get<ElementT>();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Variant: 未设定活动元素或活动元素类型不匹配。"));
		}
		return pElement;
	}
	template<typename ElementT>
	ElementT *Require(){
		const auto pElement = Get<ElementT>();
		if(!pElement){
			MCF_THROW(Exception, ERROR_NOT_READY, Rcntws::View(L"Variant: 未设定活动元素或活动元素类型不匹配。"));
		}
		return pElement;
	}

	Variant &Reset() noexcept {
		X_VisitPointer([&](auto, auto p){ Destruct<>(p); });
		x_nActiveIndex = -1;
		return *this;
	}
	template<unsigned kIndexT, typename ...ParamsT>
	Variant &Reset(ParamsT &&...vParams){
		Reset();

		void *const pElementRaw = &x_vStorage;
		const auto pElement = static_cast<std::tuple_element_t<kIndexT, X_TypeTuple> *>(pElementRaw);
		Construct<>(pElement, std::forward<ParamsT>(vParams)...);
		x_nActiveIndex = static_cast<int>(kIndexT);
		return *this;
	}
	template<typename ElementT>
	Variant &Reset(ElementT &&vElement){
		return Reset<Impl_Variant::IndexByType<std::decay_t<ElementT>, ElementsT...>::value>(std::forward<ElementT>(vElement));
	}

	template<typename FunctionT>
	void Visit(FunctionT &&vFunction) const {
		X_VisitPointer([&](auto, auto p){ return std::forward<FunctionT>(vFunction)(*p); });
	}
	template<typename FunctionT>
	void Visit(FunctionT &&vFunction){
		X_VisitPointer([&](auto, auto p){ return std::forward<FunctionT>(vFunction)(*p); });
	}

	void Swap(Variant &vOther) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...)) {
		auto vTemp = std::move_if_noexcept(*this);
		*this = std::move(vOther);
		vOther = std::move(vTemp);
	}

public:
	friend void swap(Variant &vSelf, Variant &vOther) noexcept(noexcept(vSelf.Swap(vOther))) {
		vSelf.Swap(vOther);
	}
};

}

#endif
