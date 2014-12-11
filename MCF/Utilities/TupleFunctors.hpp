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
		static void DoForwards(FunctionT &, const Tuple &){
		}
		template<typename VoidT, typename FirstT, typename ...RemainingT>
		static void DoForwards(FunctionT &vFunction, const Tuple &vTuple){
			DoForwards<VoidT, RemainingT...>(vFunction, vTuple);
			std::forward<FunctionT>(vFunction)(std::get<sizeof...(RemainingT)>(vTuple));
		}

		template<typename VoidT>
		static void DoReversely(FunctionT &, const Tuple &){
		}
		template<typename VoidT, typename FirstT, typename ...RemainingT>
		static void DoReversely(FunctionT &vFunction, const Tuple &vTuple){
			std::forward<FunctionT>(vFunction)(std::get<sizeof...(RemainingT)>(vTuple));
			DoReversely<VoidT, RemainingT...>(vFunction, vTuple);
		}
	};
}

// AbsorbTuple(foo, make_tuple(1, 2, 3)) -> foo(1, 2, 3);
template<typename FunctionT, typename ...ParamsT>
FunctionT &&AbsorbTuple(FunctionT &&vFunction, const std::tuple<ParamsT...> &vTuple){
	Impl::AbsorbTupleHelper<FunctionT, ParamsT...>::template DoForwards<void, ParamsT...>(vFunction, vTuple);
	return std::forward<FunctionT>(vFunction);
}

// ReverseAbsorbTuple(foo, make_tuple(1, 2, 3)) -> foo(3, 2, 1);
template<typename FunctionT, typename ...ParamsT>
FunctionT &&ReverseAbsorbTuple(FunctionT &&vFunction, const std::tuple<ParamsT...> &vTuple){
	Impl::AbsorbTupleHelper<FunctionT, ParamsT...>::template DoReversely<void, ParamsT...>(vFunction, vTuple);
	return std::forward<FunctionT>(vFunction);
}

namespace Impl {
	template<typename FunctionT, typename ...ParamsT>
	struct SqueezeTupleHelper {
		using Tuple = std::tuple<ParamsT...>;

		template<typename VoidT, typename ...UnpackedT>
		static decltype(auto) DoForwards(FunctionT &vFunction, const Tuple &, UnpackedT &...vUnpacked){
			return std::forward<FunctionT>(vFunction)(std::forward<UnpackedT>(vUnpacked)...);
		}
		template<typename VoidT, typename FirstT, typename ...RemainingT, typename ...UnpackedT>
		static decltype(auto) DoForwards(FunctionT &vFunction, const Tuple &vTuple, UnpackedT &...vUnpacked){
			return DoForwards<VoidT, RemainingT...>(vFunction, vTuple, vUnpacked..., std::get<sizeof...(UnpackedT)>(vTuple));
		}

		template<typename VoidT, typename ...UnpackedT>
		static decltype(auto) DoReversely(FunctionT &vFunction, const Tuple &, UnpackedT &...vUnpacked){
			return std::forward<FunctionT>(vFunction)(std::forward<UnpackedT>(vUnpacked)...);
		}
		template<typename VoidT, typename FirstT, typename ...RemainingT, typename ...UnpackedT>
		static decltype(auto) DoReversely(FunctionT &vFunction, const Tuple &vTuple, UnpackedT &...vUnpacked){
			return DoReversely<VoidT, RemainingT...>(vFunction, vTuple, std::get<sizeof...(UnpackedT)>(vTuple), vUnpacked...);
		}
	};
}

// SqueezeTuple(foo, make_tuple(1, 2, 3)); -> foo(1, 2, 3);
template<typename FunctionT, typename ...ParamsT>
decltype(auto) SqueezeTuple(FunctionT &&vFunction, const std::tuple<ParamsT...> &vTuple){
	return Impl::SqueezeTupleHelper<FunctionT, ParamsT...>::template DoForwards<void, ParamsT...>(vFunction, vTuple);
}

// ReverseSqueezeTuple(foo, make_tuple(1, 2, 3)); -> foo(3, 2, 1);
template<typename FunctionT, typename ...ParamsT>
decltype(auto) ReverseSqueezeTuple(FunctionT &&vFunction, const std::tuple<ParamsT...> &vTuple){
	return Impl::SqueezeTupleHelper<FunctionT, ParamsT...>::template DoReversely<void, ParamsT...>(vFunction, vTuple);
}

// 简单包装器。
template<typename ClassT,  typename ...ParamsT>
ClassT MakeFromTuple(const std::tuple<ParamsT...> &vTuple){
	return SqueezeTuple([](auto &&...vParams){ return ClassT(std::move(vParams)...); }, vTuple);
}

}

#endif
