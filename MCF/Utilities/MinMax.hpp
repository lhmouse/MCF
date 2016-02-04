// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_MIN_MAX_HPP_
#define MCF_UTILITIES_MIN_MAX_HPP_

#include "../Function/Comparators.hpp"
#include <utility>

namespace MCF {

namespace Impl_MinMax {
	template<typename T>
	constexpr T ForwardAsLvalueOrPrvalue(T &&t){
		return std::forward<T>(t);
	}

	template<typename ComparatorT, typename FirstT>
	constexpr decltype(auto) MinAsGlvalue(FirstT &&vFirst){
		return std::forward<FirstT>(vFirst);
	}
	template<typename ComparatorT, typename FirstT>
	constexpr decltype(auto) MaxAsGlvalue(FirstT &&vFirst){
		return std::forward<FirstT>(vFirst);
	}

	template<typename ComparatorT, typename FirstT, typename SecondT, typename ...RemainingT>
	constexpr decltype(auto) MinAsGlvalue(FirstT &&vFirst, SecondT &&vSecond, RemainingT &&...vRemaining){
		return MinAsGlvalue<ComparatorT>(ComparatorT()(vFirst, vSecond) ? std::forward<FirstT>(vFirst) : std::forward<SecondT>(vSecond), std::forward<RemainingT>(vRemaining)...);
	}
	template<typename ComparatorT, typename FirstT, typename SecondT, typename ...RemainingT>
	constexpr decltype(auto) MaxAsGlvalue(FirstT &&vFirst, SecondT &&vSecond, RemainingT &&...vRemaining){
		return MaxAsGlvalue<ComparatorT>(ComparatorT()(vSecond, vFirst) ? std::forward<FirstT>(vFirst) : std::forward<SecondT>(vSecond), std::forward<RemainingT>(vRemaining)...);
	}
}

template<typename ComparatorT = Less, typename ...ParamsT>
constexpr decltype(auto) Min(ParamsT &&...vParams){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(Impl_MinMax::MinAsGlvalue<ComparatorT>(std::forward<ParamsT>(vParams)...));
}
template<typename ComparatorT = Less, typename ...ParamsT>
constexpr decltype(auto) Max(ParamsT &&...vParams){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(Impl_MinMax::MaxAsGlvalue<ComparatorT>(std::forward<ParamsT>(vParams)...));
}

}

#endif
