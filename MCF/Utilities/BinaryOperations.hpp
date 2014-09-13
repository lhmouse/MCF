// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.Tyt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_BINARY_OPERATIONS_HPP_
#define MCF_BINARY_OPERATIONS_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

template<typename Tx, typename Ty>
constexpr void BCopy(Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(!std::is_empty<Tx>::value, "Tx shall not be empty.");
	static_assert(!std::is_empty<Ty>::value, "Ty shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	__builtin_memcpy(&vDst, &vSrc, sizeof(vDst));
}

template<typename Ty>
constexpr void BFill(Ty &vDst, bool bVal) noexcept {
	static_assert(!std::is_empty<Ty>::value, "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");

	__builtin_memset(&vDst, bVal ? -1 : 0, sizeof(vDst));
}

template<typename Tx, typename Ty>
constexpr int BComp(const Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(!std::is_empty<Tx>::value, "Tx shall not be empty.");
	static_assert(!std::is_empty<Ty>::value, "Ty shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	return __builtin_memcmp(&vDst, &vSrc, sizeof(vSrc));
}

template<typename Tx, typename Ty>
constexpr void BSwap(Tx &vDst, Ty &vSrc) noexcept {
	static_assert(!std::is_empty<Tx>::value, "Tx shall not be empty.");
	static_assert(!std::is_empty<Ty>::value, "Ty shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	using Array = unsigned char [sizeof(Tx)];
	std::swap(reinterpret_cast<Array &>(vDst), reinterpret_cast<Array &>(vSrc));
}

template<typename Ty>
constexpr void BZero(Ty &vDst) noexcept {
	BFill(vDst, false);
}

}

#endif
