// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_PARAMETER_PACK_MANIPULATORS_HPP_
#define MCF_UTILITIES_PARAMETER_PACK_MANIPULATORS_HPP_

#include <cstddef>

namespace MCF {

namespace Impl_ParameterPackManipulators {
	template<std::size_t kIndexT, typename FirstT, typename ...RemainingT>
	struct ParameterPackExpander {
		using Type = typename ParameterPackExpander<kIndexT - 1, RemainingT...>::Type;
	};
	template<typename FirstT, typename ...RemainingT>
	struct ParameterPackExpander<0, FirstT, RemainingT...> {
		using Type = FirstT;
	};
}

template<std::size_t kIndexT, typename ...TypesT>
using NthType = typename Impl_ParameterPackManipulators::ParameterPackExpander<kIndexT, TypesT...>::Type;

namespace Impl_ParameterPackManipulators {
	template<typename ToFindT, typename FirstT, typename ...RemainingT>
	struct FirstTypeFinder {
		static_assert(sizeof...(RemainingT) != 0, "Type not found.");

		enum : std::size_t {
			kIndex = FirstTypeFinder<ToFindT, RemainingT...>::kIndex + 1
		};
	};
	template<typename ToFindT, typename ...RemainingT>
	struct FirstTypeFinder<ToFindT, ToFindT, RemainingT...> {
		enum : std::size_t {
			kIndex = 0
		};
	};

	template<typename ...PackT>
	struct DummyTuple {
	};

	template<typename ToFindT, typename FirstT, typename ...TypesT, typename ...ReversedT>
	constexpr std::size_t FindLastTypeHelper(DummyTuple<FirstT, TypesT...>, DummyTuple<ReversedT...>) noexcept {
		return FindLastTypeHelper<ToFindT>(DummyTuple<TypesT...>(), DummyTuple<FirstT, ReversedT...>());
	}
	template<typename ToFindT, typename ...ReversedT>
	constexpr std::size_t FindLastTypeHelper(DummyTuple<>, DummyTuple<ReversedT...>) noexcept {
		return sizeof...(ReversedT) - 1 - FirstTypeFinder<ToFindT, ReversedT...>::kIndex;
	}
}

template<typename ToFindT, typename ...TypesT>
constexpr std::size_t FindFirstType() noexcept {
	return Impl_ParameterPackManipulators::FirstTypeFinder<ToFindT, TypesT...>::kIndex;
}

template<typename ToFindT, typename ...TypesT>
constexpr std::size_t FindLastType() noexcept {
	return Impl_ParameterPackManipulators::FindLastTypeHelper<ToFindT>(
		Impl_ParameterPackManipulators::DummyTuple<TypesT...>(), Impl_ParameterPackManipulators::DummyTuple<>());
}

}

#endif
