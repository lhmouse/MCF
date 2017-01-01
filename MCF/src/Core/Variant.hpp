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
	int x_nActiveIndex;

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
	Variant() noexcept
		: x_nActiveIndex(-1)
	{
	}
	template<typename ParamT,
		std::enable_if_t<
			!std::is_base_of<Variant, std::decay_t<ParamT>>::value,
			int> = 0>
	Variant(ParamT &&rhs)
		: Variant()
	{
		Set(std::forward<ParamT>(rhs));
	}
	Variant(const Variant &rhs) noexcept((std::is_nothrow_copy_constructible<ElementsT>::value && ...))
		: Variant()
	{
		rhs.X_VisitPointer([this](auto k, auto p){ this->Set<k>(*p); });
	}
	Variant(Variant &&rhs) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...))
		: Variant()
	{
		rhs.X_VisitPointer([this](auto k, auto p){ this->Set<k>(std::move(*p)); });
	}
	Variant &operator=(const Variant &rhs) noexcept((std::is_nothrow_copy_constructible<ElementsT>::value && ...)) {
		rhs.X_VisitPointer([this](auto k, auto p){ this->Set<k>(*p); });
		return *this;
	}
	Variant &operator=(Variant &&rhs) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...)) {
		rhs.X_VisitPointer([this](auto k, auto p){ this->Set<k>(std::move(*p)); });
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
		X_VisitPointer([&](auto, auto p){ Destruct<>(p); });
		x_nActiveIndex = -1;
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
	template<unsigned kIndexT, typename ...ParamsT>
	void Set(ParamsT &&...vParams){
		Clear();

		void *const pElementRaw = &x_vStorage;
		const auto pElement = static_cast<std::tuple_element_t<kIndexT, X_TypeTuple> *>(pElementRaw);
		Construct<>(pElement, std::forward<ParamsT>(vParams)...);
		x_nActiveIndex = static_cast<int>(kIndexT);
	}
	template<typename ElementT>
	void Set(ElementT &&vElement){
		Set<Impl_Variant::IndexByType<std::decay_t<ElementT>, ElementsT...>::value>(std::forward<ElementT>(vElement));
	}

	template<typename FunctionT>
	void Visit(FunctionT &&vFunction) const {
		X_VisitPointer([&](auto, auto p){ return std::forward<FunctionT>(vFunction)(*p); });
	}
	template<typename FunctionT>
	void Visit(FunctionT &&vFunction){
		X_VisitPointer([&](auto, auto p){ return std::forward<FunctionT>(vFunction)(*p); });
	}

	void Swap(Variant &rhs) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...)) {
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
