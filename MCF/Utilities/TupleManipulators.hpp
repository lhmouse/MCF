// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_TUPLE_MANIPULATORS_HPP_
#define MCF_UTILITIES_TUPLE_MANIPULATORS_HPP_

#include <tuple>
#include <utility>
#include <type_traits>
#include "ParameterPackManipulators.hpp"

namespace MCF {

namespace Impl_TupleManipulators {
	template<typename FunctionT, typename TupleT>
	struct AbsorbTupleHelper {
		FunctionT &vFunction;
		TupleT &vTuple;

		constexpr AbsorbTupleHelper(FunctionT &vFunction_, TupleT &vTuple_) noexcept
			: vFunction(vFunction_), vTuple(vTuple_)
		{
		}

		template<typename VoidT>
		constexpr void Perform() const {
		}
		template<typename VoidT, typename FirstT, typename ...RemainingT>
		constexpr void Perform() const {
			Perform<VoidT, RemainingT...>();
			std::forward<FunctionT>(vFunction)(static_cast<FirstT>(std::get<sizeof...(RemainingT)>(vTuple)));
		}

		template<typename VoidT>
		constexpr void ReversePerform() const {
		}
		template<typename VoidT, typename FirstT, typename ...RemainingT>
		constexpr void ReversePerform() const {
			std::forward<FunctionT>(vFunction)(static_cast<FirstT>(std::get<sizeof...(RemainingT)>(vTuple)));
			ReversePerform<VoidT, RemainingT...>();
		}
	};
}

// AbsorbTuple(foo, std::make_tuple(1, 2, 3)); -> { foo(1); foo(2); foo(3); }
template<typename FunctionT, typename ...ElementsT>
constexpr FunctionT &&AbsorbTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	const auto vHelper = Impl_TupleManipulators::AbsorbTupleHelper<FunctionT, std::tuple<ElementsT...>>(vFunction, vTuple);
	vHelper.template Perform<void, ElementsT &&...>();
	return std::forward<FunctionT>(vFunction);
}

// ReverseAbsorbTuple(foo, std::make_tuple(1, 2, 3)); -> { foo(3); foo(2); foo(1); }
template<typename FunctionT, typename ...ElementsT>
constexpr FunctionT &&ReverseAbsorbTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	const auto vHelper = Impl_TupleManipulators::AbsorbTupleHelper<FunctionT, std::tuple<ElementsT...>>(vFunction, vTuple);
	vHelper.template ReversePerform<void, ElementsT &&...>();
	return std::forward<FunctionT>(vFunction);
}

namespace Impl_TupleManipulators {
	template<typename FunctionT, typename TupleT>
	struct SqueezeTupleHelper {
		FunctionT &vFunction;
		TupleT &vTuple;

		constexpr SqueezeTupleHelper(FunctionT &vFunction_, TupleT &vTuple_) noexcept
			: vFunction(vFunction_), vTuple(vTuple_)
		{
		}

		template<typename ...ReferencesT, std::size_t ...kIndicesT>
		constexpr decltype(auto) Perform(std::index_sequence<kIndicesT...>) const {
			return std::forward<FunctionT>(vFunction)(static_cast<ReferencesT>(std::get<kIndicesT>(vTuple))...);
		}

		template<typename ...ReferencesT, std::size_t ...kIndicesT>
		constexpr decltype(auto) ReversePerform(std::index_sequence<kIndicesT...>) const {
			return std::forward<FunctionT>(vFunction)(static_cast<ReferencesT>(std::get<sizeof...(kIndicesT) - 1 - kIndicesT>(vTuple))...);
		}
	};
}

// SqueezeTuple(foo, std::make_tuple(1, 2, 3)); -> foo(1, 2, 3);
template<typename FunctionT, typename ...ElementsT>
constexpr decltype(auto) SqueezeTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	const auto vHelper = Impl_TupleManipulators::SqueezeTupleHelper<FunctionT, std::tuple<ElementsT...>>(vFunction, vTuple);
	return vHelper.template Perform<ElementsT &&...>(std::index_sequence_for<ElementsT...>());
}

// ReverseSqueezeTuple(foo, std::make_tuple(1, 2, 3)); -> foo(3, 2, 1);
template<typename FunctionT, typename ...ElementsT>
constexpr decltype(auto) ReverseSqueezeTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	const auto vHelper = Impl_TupleManipulators::SqueezeTupleHelper<FunctionT, std::tuple<ElementsT...>>(vFunction, vTuple);
	return vHelper.template ReversePerform<ElementsT &&...>(std::index_sequence_for<ElementsT...>());
}

template<typename ToFindT, typename ...TypesT>
constexpr std::size_t FindFirstType(const std::tuple<TypesT...> &) noexcept {
	return FindFirstType<ToFindT, TypesT...>();
}

template<typename ToFindT, typename ...TypesT>
constexpr std::size_t FindLastType(const std::tuple<TypesT...> &) noexcept {
	return FindLastType<ToFindT, TypesT...>();
}

}

#endif
