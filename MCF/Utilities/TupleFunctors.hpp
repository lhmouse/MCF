// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_TUPLE_FUNCTORS_HPP_
#define MCF_UTILITIES_TUPLE_FUNCTORS_HPP_

#include <type_traits>
#include <tuple>
#include <utility>
#include <memory>
#include <cstddef>

namespace MCF {

namespace Impl {
	template<typename FunctionT, typename ...ParamsT>
	struct AbsorbTupleHelper {
		using Tuple = std::tuple<ParamsT...>;

		template<typename VoidT>
		static void DoForwards(FunctionT &&, const Tuple &){
		}
		template<typename VoidT, typename FirstT, typename ...RemainingT>
		static void DoForwards(FunctionT &&vFunction, const Tuple &vTuple){
			DoForwards<VoidT, RemainingT...>(std::forward<FunctionT>(vFunction), vTuple);
			std::forward<FunctionT>(vFunction)(std::get<sizeof...(RemainingT)>(vTuple));
		}

		template<typename VoidT>
		static void DoReversely(FunctionT &&, const Tuple &){
		}
		template<typename VoidT, typename FirstT, typename ...RemainingT>
		static void DoReversely(FunctionT &&vFunction, const Tuple &vTuple){
			std::forward<FunctionT>(vFunction)(std::get<sizeof...(RemainingT)>(vTuple));
			DoReversely<VoidT, RemainingT...>(std::forward<FunctionT>(vFunction), vTuple);
		}
	};
}

// AbsorbTuple(foo, make_tuple(1, 2, 3)) -> foo(1, 2, 3);
template<typename FunctionT, typename ...ParamsT>
FunctionT &&AbsorbTuple(FunctionT &&vFunction, const std::tuple<ParamsT...> &vTuple){
	Impl::AbsorbTupleHelper<FunctionT, ParamsT...>::template DoForwards<void, ParamsT...>(
		std::forward<FunctionT>(vFunction), vTuple);
	return std::forward<FunctionT>(vFunction);
}

// ReverseAbsorbTuple(foo, make_tuple(1, 2, 3)) -> foo(3, 2, 1);
template<typename FunctionT, typename ...ParamsT>
FunctionT &&ReverseAbsorbTuple(FunctionT &&vFunction, const std::tuple<ParamsT...> &vTuple){
	Impl::AbsorbTupleHelper<FunctionT, ParamsT...>::template DoReversely<void, ParamsT...>(
		std::forward<FunctionT>(vFunction), vTuple);
	return std::forward<FunctionT>(vFunction);
}

namespace Impl {
	template<typename FunctionT, typename ...ParamsT>
	struct SqueezeTupleHelper {
		using Tuple = std::tuple<ParamsT...>;

		template<std::size_t ...IndicesT>
		static decltype(auto) DoForwards(FunctionT &&vFunction, const Tuple &vTuple, std::index_sequence<IndicesT...>){
			return std::forward<FunctionT>(vFunction)(std::get<IndicesT>(vTuple)...);
		}

		template<std::size_t ...IndicesT>
		static decltype(auto) DoReversely(FunctionT &&vFunction, const Tuple &vTuple, std::index_sequence<IndicesT...>){
			return std::forward<FunctionT>(vFunction)(std::get<sizeof...(ParamsT) - 1 - IndicesT>(vTuple)...);
		}
	};
}

// SqueezeTuple(foo, make_tuple(1, 2, 3)); -> foo(1, 2, 3);
template<typename FunctionT, typename ...ParamsT>
decltype(auto) SqueezeTuple(FunctionT &&vFunction, const std::tuple<ParamsT...> &vTuple){
	return Impl::SqueezeTupleHelper<FunctionT, ParamsT...>::template DoForwards(
		std::forward<FunctionT>(vFunction), vTuple, std::index_sequence_for<ParamsT...>());
}

// ReverseSqueezeTuple(foo, make_tuple(1, 2, 3)); -> foo(3, 2, 1);
template<typename FunctionT, typename ...ParamsT>
decltype(auto) ReverseSqueezeTuple(FunctionT &&vFunction, const std::tuple<ParamsT...> &vTuple){
	return Impl::SqueezeTupleHelper<FunctionT, ParamsT...>::template DoReversely(
		std::forward<FunctionT>(vFunction), vTuple, std::index_sequence_for<ParamsT...>());
}

// 简单包装器。
template<typename T,  typename ...ParamsT>
T MakeFromTuple(const std::tuple<ParamsT...> &vTuple){
	return SqueezeTuple([](const auto &...vParams){ return ObjectT(vParams...); }, vTuple);
}

}

#endif
