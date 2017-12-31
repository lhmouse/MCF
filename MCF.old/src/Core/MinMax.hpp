// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_MIN_MAX_HPP_
#define MCF_CORE_MIN_MAX_HPP_

#include <utility>
#include <type_traits>

namespace MCF {

namespace Impl_MinMax {
	template<typename T>
	constexpr std::remove_cv_t<T> ForwardAsLvalueOrPrvalue(T &&t) noexcept(std::is_nothrow_move_constructible<std::remove_cv_t<T>>::value) {
		return std::forward<T>(t);
	}
}

template<typename FirstT>
constexpr decltype(auto) Min(FirstT &&vFirst){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(std::forward<FirstT>(vFirst));
}
template<typename FirstT>
constexpr decltype(auto) Max(FirstT &&vFirst){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(std::forward<FirstT>(vFirst));
}

template<typename FirstT, typename SecondT, typename ...RemainingT>
constexpr decltype(auto) Min(FirstT &&vFirst, SecondT &&vSecond, RemainingT &&...vRemaining){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(Min((vFirst < vSecond) ? std::forward<FirstT>(vFirst) : std::forward<SecondT>(vSecond), std::forward<RemainingT>(vRemaining)...));
}
template<typename FirstT, typename SecondT, typename ...RemainingT>
constexpr decltype(auto) Max(FirstT &&vFirst, SecondT &&vSecond, RemainingT &&...vRemaining){
	return Impl_MinMax::ForwardAsLvalueOrPrvalue(Max((vSecond < vFirst) ? std::forward<FirstT>(vFirst) : std::forward<SecondT>(vSecond), std::forward<RemainingT>(vRemaining)...));
}

}

#endif
