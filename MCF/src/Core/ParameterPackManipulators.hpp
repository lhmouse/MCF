// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_PARAMETER_PACK_MANIPULATORS_HPP_
#define MCF_CORE_PARAMETER_PACK_MANIPULATORS_HPP_

#include <type_traits>
#include <cstddef>

namespace MCF {

namespace Impl_ParameterPackManipulators {
	template<std::size_t kIndexT, typename FirstT, typename ...RemainingT>
	struct Expander {
		using Type = typename Expander<kIndexT - 1, RemainingT...>::Type;
	};
	template<typename FirstT, typename ...RemainingT>
	struct Expander<0, FirstT, RemainingT...> {
		using Type = FirstT;
	};
}

template<std::size_t kIndexT, typename ...TypesT>
using NthType = typename Impl_ParameterPackManipulators::Expander<kIndexT, TypesT...>::Type;

namespace Impl_ParameterPackManipulators {
	template<typename ToFindT, std::ptrdiff_t kIndexT, int kIncrementT, std::ptrdiff_t kCountOfTypesT, typename ...TypesT>
	struct TypeFinder {
		static constexpr auto kIndex = std::is_same<ToFindT, typename Expander<kIndexT, TypesT...>::Type>::value
			? kIndexT
			: TypeFinder<ToFindT, kIndexT + kIncrementT, kIncrementT, kCountOfTypesT, TypesT...>::kIndex;
	};
	template<typename ToFindT, int kIncrementT, std::ptrdiff_t kCountOfTypesT, typename ...TypesT>
	struct TypeFinder<ToFindT, -1, kIncrementT, kCountOfTypesT, TypesT...> {
		static constexpr auto kIndex = (std::size_t)-1;
	};
	template<typename ToFindT, int kIncrementT, std::ptrdiff_t kCountOfTypesT, typename ...TypesT>
	struct TypeFinder<ToFindT, kCountOfTypesT, kIncrementT, kCountOfTypesT, TypesT...> {
		static constexpr auto kIndex = (std::size_t)-1;
	};
}

template<typename ToFindT, typename ...TypesT>
constexpr std::size_t FindFirstType() noexcept {
	return Impl_ParameterPackManipulators::TypeFinder<ToFindT, 0, 1, sizeof...(TypesT), TypesT...>::kIndex;
}
template<typename ToFindT, typename ...TypesT>
constexpr std::size_t FindLastType() noexcept {
	return Impl_ParameterPackManipulators::TypeFinder<ToFindT, sizeof...(TypesT) - 1, -1, sizeof...(TypesT), TypesT...>::kIndex;
}

}

#endif
