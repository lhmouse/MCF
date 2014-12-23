// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_MIN_MAX_HPP_
#define MCF_UTILITIES_MIN_MAX_HPP_

#include <type_traits>
#include <functional>

namespace MCF {

template<typename Tx, typename Ty, typename ComparatorT = std::less<void>>
constexpr decltype(auto) Min(Tx &&x, Ty &&y){
	static_assert(std::is_scalar<std::remove_reference_t<Tx>>::value && std::is_scalar<std::remove_reference_t<Ty>>::value,
		"Only scalar types are supported.");
	static_assert(std::is_signed<std::remove_reference_t<Tx>>::value == std::is_signed<std::remove_reference_t<Ty>>::value,
		"Comparison between signed and unsigned integers.");

	return ComparatorT()(std::forward<Tx>(x), std::forward<Ty>(y)) ? std::forward<Tx>(x) : std::forward<Ty>(y);
}
template<typename Tx, typename Ty, typename ComparatorT = std::less<void>, typename ...MoreT>
constexpr decltype(auto) Min(Tx &&x, Ty &&y, MoreT &&...vMore){
	return Min(Min(std::forward<Tx>(x), std::forward<Ty>(y)), std::forward<MoreT>(vMore)...);
}

template<typename Tx, typename Ty, typename ComparatorT = std::less<void>>
constexpr decltype(auto) Max(Tx &&x, Ty &&y){
	static_assert(std::is_scalar<std::remove_reference_t<Tx>>::value && std::is_scalar<std::remove_reference_t<Ty>>::value,
		"Only scalar types are supported.");
	static_assert(std::is_signed<std::remove_reference_t<Tx>>::value == std::is_signed<std::remove_reference_t<Ty>>::value,
		"Comparison between signed and unsigned integers.");

	return ComparatorT()(std::forward<Tx>(x), std::forward<Ty>(y)) ? std::forward<Ty>(y) : std::forward<Tx>(x);
}
template<typename Tx, typename Ty, typename ComparatorT = std::less<void>, typename ...MoreT>
constexpr decltype(auto) Max(Tx &&x, Ty &&y, MoreT &&...vMore){
	return Max(Max(std::forward<Tx>(x), std::forward<Ty>(y)), std::forward<MoreT>(vMore)...);
}

}

#endif
