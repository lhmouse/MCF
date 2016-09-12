// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_TUPLE_MANIPULATORS_HPP_
#define MCF_CORE_TUPLE_MANIPULATORS_HPP_

#include <type_traits>
#include <utility>
#include <tuple>

namespace MCF {

namespace Impl_TupleManipulators {
	template<std::size_t ...kIndicesT>
	struct IndexSequence {
	};

	template<std::size_t kCurrentT, std::size_t kIncrementT, std::size_t kCountT, std::size_t ...kGeneratedT>
	struct IndexSequenceGenerator {
		static constexpr decltype(auto) DoIt() noexcept {
			return IndexSequenceGenerator<kCurrentT + kIncrementT, kIncrementT, kCountT - 1, kGeneratedT..., kCurrentT>::DoIt();
		}
	};
	template<std::size_t kCurrentT, std::size_t kIncrementT, std::size_t ...kGeneratedT>
	struct IndexSequenceGenerator<kCurrentT, kIncrementT, 0, kGeneratedT...> {
		static constexpr decltype(auto) DoIt() noexcept {
			return IndexSequence<kGeneratedT...>();
		}
	};

	template<typename FunctionT, typename TupleT, std::size_t ...kIndicesT>
	constexpr void RealAbsorb(FunctionT &vFunction, TupleT &vTuple, const IndexSequence<kIndicesT...> &){
		((void)std::forward<FunctionT>(vFunction)(std::get<kIndicesT>(std::forward<TupleT>(vTuple))), ...);
	}

	template<typename FunctionT, typename TupleT, std::size_t ...kIndicesT>
	constexpr decltype(auto) RealSqueeze(FunctionT &vFunction, TupleT &vTuple, const IndexSequence<kIndicesT...> &){
		return std::forward<FunctionT>(vFunction)(std::get<kIndicesT>(std::forward<TupleT>(vTuple))...);
	}
}

// Absorb(foo, std::make_tuple(1, 2, 3)); -> { foo(1); foo(2); foo(3); }
template<typename FunctionT, typename TupleT>
constexpr void Absorb(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	Impl_TupleManipulators::RealAbsorb<FunctionT, TupleT>(vFunction, vTuple,
		Impl_TupleManipulators::IndexSequenceGenerator<0, 1, kSize>::DoIt());
}
// ReverseAbsorb(foo, std::make_tuple(1, 2, 3)); -> { foo(3); foo(2); foo(1); }
template<typename FunctionT, typename TupleT>
constexpr void ReverseAbsorb(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	Impl_TupleManipulators::RealAbsorb<FunctionT, TupleT>(vFunction, vTuple,
		Impl_TupleManipulators::IndexSequenceGenerator<kSize - 1, (std::size_t)-1, kSize>::DoIt());
}

// Squeeze(foo, std::make_tuple(1, 2, 3)); -> { foo(1); foo(2); foo(3); }
template<typename FunctionT, typename TupleT>
constexpr decltype(auto) Squeeze(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	return Impl_TupleManipulators::RealSqueeze<FunctionT, TupleT>(vFunction, vTuple,
		Impl_TupleManipulators::IndexSequenceGenerator<0, 1, kSize>::DoIt());
}
// ReverseSqueeze(foo, std::make_tuple(1, 2, 3)); -> { foo(3); foo(2); foo(1); }
template<typename FunctionT, typename TupleT>
constexpr decltype(auto) ReverseSqueeze(FunctionT &&vFunction, TupleT &&vTuple){
	constexpr auto kSize = std::tuple_size<std::decay_t<TupleT>>::value;
	return Impl_TupleManipulators::RealSqueeze<FunctionT, TupleT>(vFunction, vTuple,
		Impl_TupleManipulators::IndexSequenceGenerator<kSize - 1, (std::size_t)-1, kSize>::DoIt());
}

}

#endif
