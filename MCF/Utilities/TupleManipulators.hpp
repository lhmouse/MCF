// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_TUPLE_MANIPULATORS_HPP_
#define MCF_UTILITIES_TUPLE_MANIPULATORS_HPP_

#include <tuple>
#include <utility>
#include <type_traits>
#include <initializer_list>

namespace MCF {

namespace Impl_TupleManipulators {
	template<typename FunctionT, typename TupleT, std::size_t ...kIndicesT>
	constexpr void Absorb(FunctionT &vFunction, TupleT &vTuple, std::index_sequence<kIndicesT...>){
		(void)std::initializer_list<int>{
			((void)std::forward<FunctionT>(vFunction)(
				std::forward<std::tuple_element_t<kIndicesT, TupleT> &&>(
				                         std::get<kIndicesT>(vTuple))), 1)...
		};
	}
	template<typename FunctionT, typename TupleT, std::size_t ...kIndicesT>
	constexpr void ReverseAbsorb(FunctionT &vFunction, TupleT &vTuple, std::index_sequence<kIndicesT...>){
		(void)std::initializer_list<int>{
			((void)std::forward<FunctionT>(vFunction)(
				std::forward<std::tuple_element_t<sizeof...(kIndicesT) - 1 - kIndicesT, TupleT> &&>(
				                         std::get<sizeof...(kIndicesT) - 1 - kIndicesT>(vTuple))), 1)...
		};
	}
}

// AbsorbTuple(foo, std::make_tuple(1, 2, 3)); -> { foo(1); foo(2); foo(3); }
template<typename FunctionT, typename ...ElementsT>
constexpr void AbsorbTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	Impl_TupleManipulators::Absorb<FunctionT>(vFunction, vTuple, std::index_sequence_for<ElementsT...>());
}

// ReverseAbsorbTuple(foo, std::make_tuple(1, 2, 3)); -> { foo(3); foo(2); foo(1); }
template<typename FunctionT, typename ...ElementsT>
constexpr void ReverseAbsorbTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	Impl_TupleManipulators::ReverseAbsorb<FunctionT>(vFunction, vTuple, std::index_sequence_for<ElementsT...>());
}

namespace Impl_TupleManipulators {
	template<typename FunctionT, typename TupleT, std::size_t ...kIndicesT>
	constexpr decltype(auto) Squeeze(FunctionT &vFunction, TupleT &vTuple, std::index_sequence<kIndicesT...>){
		return std::forward<FunctionT>(vFunction)(
			std::forward<std::tuple_element_t<kIndicesT, TupleT> &&>(
			                         std::get<kIndicesT>(vTuple))...);
	}
	template<typename FunctionT, typename TupleT, std::size_t ...kIndicesT>
	constexpr decltype(auto) ReverseSqueeze(FunctionT &vFunction, TupleT &vTuple, std::index_sequence<kIndicesT...>){
		return std::forward<FunctionT>(vFunction)(
			std::forward<std::tuple_element_t<sizeof...(kIndicesT) - 1 - kIndicesT, TupleT> &&>(
			                         std::get<sizeof...(kIndicesT) - 1 - kIndicesT>(vTuple))...);
	}
}

// SqueezeTuple(foo, std::make_tuple(1, 2, 3)); -> foo(1, 2, 3);
template<typename FunctionT, typename ...ElementsT>
constexpr decltype(auto) SqueezeTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	return Impl_TupleManipulators::Squeeze<FunctionT>(vFunction, vTuple, std::index_sequence_for<ElementsT...>());
}

// ReverseSqueezeTuple(foo, std::make_tuple(1, 2, 3)); -> foo(3, 2, 1);
template<typename FunctionT, typename ...ElementsT>
constexpr decltype(auto) ReverseSqueezeTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	return Impl_TupleManipulators::ReverseSqueeze<FunctionT>(vFunction, vTuple, std::index_sequence_for<ElementsT...>());
}

}

#endif
