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
	template<typename FunctionT, typename ...ElementsT>
	struct AbsorbTupleHelper {
		FunctionT &vFunction;
		std::tuple<ElementsT...> &vTuple;

		constexpr AbsorbTupleHelper(FunctionT &vFunction_, std::tuple<ElementsT...> &vTuple_) noexcept
			: vFunction(vFunction_), vTuple(vTuple_)
		{
		}

		template<std::size_t ...kIndicesT>
		constexpr void Perform(std::index_sequence<kIndicesT...>) const {
			(void)std::initializer_list<int>{ ((void)std::forward<FunctionT>(vFunction)(std::forward<ElementsT>(std::get<kIndicesT>(vTuple))), 1)... };
		}
		template<std::size_t ...kIndicesT>
		constexpr void ReversePerform(std::index_sequence<kIndicesT...>) const {
			(void)std::initializer_list<int>{ ((void)std::forward<FunctionT>(vFunction)(std::forward<ElementsT>(std::get<sizeof...(kIndicesT) - 1 - kIndicesT>(vTuple))), 1)... };
		}
	};
}

// AbsorbTuple(foo, std::make_tuple(1, 2, 3)); -> { foo(1); foo(2); foo(3); }
template<typename FunctionT, typename ...ElementsT>
constexpr FunctionT &&AbsorbTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	return static_cast<void>(Impl_TupleManipulators::AbsorbTupleHelper<FunctionT, ElementsT...>(vFunction, vTuple).Perform(std::index_sequence_for<ElementsT...>())), std::forward<FunctionT>(vFunction);
}

// ReverseAbsorbTuple(foo, std::make_tuple(1, 2, 3)); -> { foo(3); foo(2); foo(1); }
template<typename FunctionT, typename ...ElementsT>
constexpr FunctionT &&ReverseAbsorbTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	return static_cast<void>(Impl_TupleManipulators::AbsorbTupleHelper<FunctionT, ElementsT...>(vFunction, vTuple).ReversePerform(std::index_sequence_for<ElementsT...>())), std::forward<FunctionT>(vFunction);
}

namespace Impl_TupleManipulators {
	template<typename FunctionT, typename ...ElementsT>
	struct SqueezeTupleHelper {
		FunctionT &vFunction;
		std::tuple<ElementsT...> &vTuple;

		constexpr SqueezeTupleHelper(FunctionT &vFunction_, std::tuple<ElementsT...> &vTuple_) noexcept
			: vFunction(vFunction_), vTuple(vTuple_)
		{
		}

		template<std::size_t ...kIndicesT>
		constexpr decltype(auto) Perform(std::index_sequence<kIndicesT...>) const {
			return std::forward<FunctionT>(vFunction)(std::forward<ElementsT>(std::get<kIndicesT>(vTuple))...);
		}
		template<std::size_t ...kIndicesT>
		constexpr decltype(auto) ReversePerform(std::index_sequence<kIndicesT...>) const {
			return std::forward<FunctionT>(vFunction)(std::forward<ElementsT>(std::get<sizeof...(kIndicesT) - 1 - kIndicesT>(vTuple))...);
		}
	};
}

// SqueezeTuple(foo, std::make_tuple(1, 2, 3)); -> foo(1, 2, 3);
template<typename FunctionT, typename ...ElementsT>
constexpr decltype(auto) SqueezeTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	return Impl_TupleManipulators::SqueezeTupleHelper<FunctionT, ElementsT...>(vFunction, vTuple).Perform(std::index_sequence_for<ElementsT...>());
}

// ReverseSqueezeTuple(foo, std::make_tuple(1, 2, 3)); -> foo(3, 2, 1);
template<typename FunctionT, typename ...ElementsT>
constexpr decltype(auto) ReverseSqueezeTuple(FunctionT &&vFunction, std::tuple<ElementsT...> vTuple){
	return Impl_TupleManipulators::SqueezeTupleHelper<FunctionT, ElementsT...>(vFunction, vTuple).ReversePerform(std::index_sequence_for<ElementsT...>());
}

}

#endif
