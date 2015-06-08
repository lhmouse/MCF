// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_MIN_MAX_HPP_
#define MCF_UTILITIES_MIN_MAX_HPP_

#include <functional>
#include <utility>

namespace MCF {

namespace Impl_MinMax {
	template<typename T>
	T ForwardAsLvalueOrPrvalue(T &&val){
		return std::forward<T>(val);
	}
}

template<typename ComparatorT = std::less<void>, typename FirstT>
constexpr decltype(auto) Min(FirstT &&first){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(std::forward<FirstT>(first));
}
template<typename ComparatorT = std::less<void>, typename FirstT>
constexpr decltype(auto) Max(FirstT &&first){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(std::forward<FirstT>(first));
}

template<typename ComparatorT = std::less<void>, typename FirstT, typename SecondT, typename ...RemainingT>
constexpr decltype(auto) Min(FirstT &&first, SecondT &&second, RemainingT &&...remaining){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(Min<ComparatorT>(
			ComparatorT()(first, second) ? std::forward<FirstT>(first) : std::forward<SecondT>(second),
			std::forward<RemainingT>(remaining)...));
}
template<typename ComparatorT = std::less<void>, typename FirstT, typename SecondT, typename ...RemainingT>
constexpr decltype(auto) Max(FirstT &&first, SecondT &&second, RemainingT &&...remaining){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(
		Max<ComparatorT>(
			!ComparatorT()(first, second) ? std::forward<FirstT>(first) : std::forward<SecondT>(second),
			std::forward<RemainingT>(remaining)...));
}

}

#endif
