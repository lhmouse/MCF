// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.Tyt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_BINARY_OPERATIONS_HPP_
#define MCF_CORE_BINARY_OPERATIONS_HPP_

#include <type_traits>
#include <cstring>

namespace MCF {

template<typename Ty, typename Tx>
void BCopy(Ty &dst, const Tx &src) noexcept {
	static_assert(!std::is_empty<Ty>::value,  "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(!std::is_empty<Tx>::value,  "Tx shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(sizeof(dst) == sizeof(src), "Source and destination sizes do not match.");

	std::memcpy(&dst, &src, sizeof(dst));
}

template<typename Ty>
void BFill(Ty &dst, bool bVal) noexcept {
	static_assert(!std::is_empty<Ty>::value,  "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");

	std::memset(&dst, -bVal, sizeof(dst));
}

template<typename Ty, typename Tx>
int BComp(const Ty &dst, const Tx &src) noexcept {
	static_assert(!std::is_empty<Ty>::value,  "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(!std::is_empty<Tx>::value,  "Tx shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(sizeof(dst) == sizeof(src), "Source and destination sizes do not match.");

	return std::memcmp(&dst, &src, sizeof(src));
}

template<typename Ty, typename Tx>
void BSwap(Ty &dst, Tx &src) noexcept {
	static_assert(!std::is_empty<Ty>::value,  "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(!std::is_empty<Tx>::value,  "Tx shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(sizeof(dst) == sizeof(src), "Source and destination sizes do not match.");

	for(std::size_t uIndex = 0; uIndex < sizeof(dst); ++uIndex){
		auto &chd = reinterpret_cast<char (&)[sizeof(dst)]>(dst)[uIndex];
		auto &chs = reinterpret_cast<char (&)[sizeof(src)]>(src)[uIndex];
		const char cht = chd;
		chd = chs;
		chs = cht;
	}
}

template<typename Ty>
void BZero(Ty &dst) noexcept {
	BFill(dst, false);
}

}

#endif
