// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.Tyt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_BINARY_OPERATIONS_HPP_
#define MCF_CORE_BINARY_OPERATIONS_HPP_

#include <type_traits>
#include <cstring>

namespace MCF {

template<typename Ty, typename Tx>
inline void BCopy(Ty &y, const Tx &x) noexcept {
	static_assert(!std::is_empty<Ty>::value,  "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(!std::is_empty<Tx>::value,  "Tx shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(sizeof(y) == sizeof(x), "Source and destination sizes do not match.");

	const auto py = reinterpret_cast<char (&)[sizeof(y)]>(y);
	const auto px = reinterpret_cast<const char (&)[sizeof(x)]>(x);
	std::memcpy(py, px, sizeof(*py));
}

template<typename Ty>
inline void BFill(Ty &y, bool bVal) noexcept {
	static_assert(!std::is_empty<Ty>::value,  "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");

	const auto py = reinterpret_cast<char (&)[sizeof(y)]>(y);
	std::memset(py, -bVal, sizeof(*py));
}

template<typename Ty>
inline void BZero(Ty &y) noexcept {
	BFill(y, false);
}

template<typename Ty, typename Tx>
inline int BComp(const Ty &y, const Tx &x) noexcept {
	static_assert(!std::is_empty<Ty>::value,  "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(!std::is_empty<Tx>::value,  "Tx shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(sizeof(y) == sizeof(x), "Source and destination sizes do not match.");

	const auto py = reinterpret_cast<const char (&)[sizeof(y)]>(y);
	const auto px = reinterpret_cast<const char (&)[sizeof(x)]>(x);
	return std::memcmp(py, px, sizeof(*py));
}

template<typename Ty, typename Tx>
inline void BSwap(Ty &y, Tx &x) noexcept {
	static_assert(!std::is_empty<Ty>::value,  "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(!std::is_empty<Tx>::value,  "Tx shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(sizeof(y) == sizeof(x), "Source and destination sizes do not match.");

	const auto py = reinterpret_cast<char (&)[sizeof(y)]>(y);
	const auto px = reinterpret_cast<char (&)[sizeof(x)]>(x);
#pragma GCC ivdep
	for(std::size_t i = 0; i < sizeof(*py); ++i){
		const char t = py[i];
		py[i] = px[i];
		px[i] = t;
	}
}

}

#endif
