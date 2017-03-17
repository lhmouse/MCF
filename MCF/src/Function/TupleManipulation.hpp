// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_TUPLE_MANIPULATION_HPP_
#define MCF_FUNCTION_TUPLE_MANIPULATION_HPP_

#include "Invoke.hpp"
#include <type_traits>
#include <utility>
#include <tuple>

namespace MCF {

namespace Impl_TupleManipulators {
	template<typename FunctionT, std::size_t ...kIndicesT, typename GetterT>
	constexpr void RealAbsorb(FunctionT &vFunction, const std::index_sequence<kIndicesT...> &, const GetterT &vGetter){
		(void)(..., (void)Invoke(std::forward<FunctionT>(vFunction), vGetter(std::integral_constant<std::size_t, kIndicesT>())));
	}
}

// Absorb(foo, std::make_tuple(1, 2, 3)); -> { foo(1); foo(2); foo(3); }
template<typename FunctionT, typename TupleT>
constexpr FunctionT &&Absorb(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	Impl_TupleManipulators::RealAbsorb<FunctionT>(vFunction, std::make_index_sequence<kSize>(), [&](auto k) -> decltype(auto) { return std::get<k>(std::forward<TupleT>(vTuple)); });
	return std::forward<FunctionT>(vFunction);
}
// ReverseAbsorb(foo, std::make_tuple(1, 2, 3)); -> { foo(3); foo(2); foo(1); }
template<typename FunctionT, typename TupleT>
constexpr FunctionT &&ReverseAbsorb(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	Impl_TupleManipulators::RealAbsorb<FunctionT>(vFunction, std::make_index_sequence<kSize>(), [&](auto k) -> decltype(auto) { return std::get<kSize - 1 - k>(std::forward<TupleT>(vTuple)); });
	return std::forward<FunctionT>(vFunction);
}

namespace Impl_TupleManipulators {
	template<typename FunctionT, std::size_t ...kIndicesT, typename GetterT>
	constexpr decltype(auto) RealSqueeze(FunctionT &vFunction, const std::index_sequence<kIndicesT...> &, const GetterT &vGetter){
		return Invoke(std::forward<FunctionT>(vFunction), vGetter(std::integral_constant<std::size_t, kIndicesT>())...);
	}
}

// Squeeze(foo, std::make_tuple(1, 2, 3)); -> { foo(1); foo(2); foo(3); }
template<typename FunctionT, typename TupleT>
constexpr decltype(auto) Squeeze(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	return Impl_TupleManipulators::RealSqueeze<FunctionT>(vFunction, std::make_index_sequence<kSize>(), [&](auto k) -> decltype(auto) { return std::get<k>(std::forward<TupleT>(vTuple)); });
}
// ReverseSqueeze(foo, std::make_tuple(1, 2, 3)); -> { foo(3); foo(2); foo(1); }
template<typename FunctionT, typename TupleT>
constexpr decltype(auto) ReverseSqueeze(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	return Impl_TupleManipulators::RealSqueeze<FunctionT>(vFunction, std::make_index_sequence<kSize>(), [&](auto k) -> decltype(auto) { return std::get<kSize - 1 - k>(std::forward<TupleT>(vTuple)); });
}

namespace Impl_TupleManipulators {
	template<typename RetT, typename FunctionT, std::size_t ...kIndicesT, typename GetterT>
	constexpr decltype(auto) RealDesignatedSqueeze(FunctionT &vFunction, const std::index_sequence<kIndicesT...> &, const GetterT &vGetter){
		return DesignatedInvoke<RetT>(std::forward<FunctionT>(vFunction), vGetter(std::integral_constant<std::size_t, kIndicesT>())...);
	}
}

// DesignatedSqueeze<T>(foo, std::make_tuple(1, 2, 3)); -> { foo(1); foo(2); foo(3); }
template<typename RetT, typename FunctionT, typename TupleT>
constexpr decltype(auto) DesignatedSqueeze(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	return Impl_TupleManipulators::RealDesignatedSqueeze<RetT, FunctionT>(vFunction, std::make_index_sequence<kSize>(), [&](auto k) -> decltype(auto) { return std::get<k>(std::forward<TupleT>(vTuple)); });
}
// DesignatedReverseSqueeze<T>(foo, std::make_tuple(1, 2, 3)); -> { foo(3); foo(2); foo(1); }
template<typename RetT, typename FunctionT, typename TupleT>
constexpr decltype(auto) DesignatedReverseSqueeze(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	return Impl_TupleManipulators::RealDesignatedSqueeze<RetT, FunctionT>(vFunction, std::make_index_sequence<kSize>(), [&](auto k) -> decltype(auto) { return std::get<kSize - 1 - k>(std::forward<TupleT>(vTuple)); });
}

}

#endif
