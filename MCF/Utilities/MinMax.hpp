// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_MIN_MAX_HPP_
#define MCF_MIN_MAX_HPP_

#include <type_traits>
#include <functional>

namespace MCF {

template<typename Tx, typename Ty, typename Comparator = std::less<void>>
constexpr auto Min(Tx x, Ty y) noexcept {
	static_assert(
		std::is_scalar<Tx>::value && std::is_scalar<Ty>::value,
		"Only scalar types are supported."
	);
	static_assert(
		std::is_signed<Tx>::value == std::is_signed<Ty>::value,
		"Comparison between signed and unsigned integers."
	);

	return Comparator()(x, y) ? x : y;
}
template<typename Tx, typename Ty, typename Comparator = std::less<void>, typename... More>
constexpr auto Min(Tx x, Ty y, More... vMore) noexcept {
	return Min(Min(x, y), vMore...);
}

template<typename Tx, typename Ty, typename Comparator = std::less<void>>
constexpr auto Max(Tx x, Ty y) noexcept {
	static_assert(
		std::is_scalar<Tx>::value && std::is_scalar<Ty>::value,
		"Only scalar types are supported."
	);
	static_assert(
		std::is_signed<Tx>::value == std::is_signed<Ty>::value,
		"Comparison between signed and unsigned integers."
	);

	return Comparator()(x, y) ? y : x;
}
template<typename Tx, typename Ty, typename Comparator = std::less<void>, typename... More>
constexpr auto Max(Tx x, Ty y, More... vMore) noexcept {
	return Max(Max(x, y), vMore...);
}

}

#endif
