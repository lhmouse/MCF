// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.Tyt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_BINARY_OPERATIONS_HPP_
#define MCF_CORE_BINARY_OPERATIONS_HPP_

#include "../Config.hpp"
#include <type_traits>
#include <utility>
#include <cstring>

namespace MCF {

template<typename Tx, typename Ty>
void BCopy(Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(!std::is_empty<Tx>::value, "Tx shall not be empty.");
	static_assert(!std::is_empty<Ty>::value, "Ty shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	std::memcpy(&vDst, &vSrc, sizeof(vDst));
}

template<typename Ty>
void BFill(Ty &vDst, bool bVal) noexcept {
	static_assert(!std::is_empty<Ty>::value, "Ty shall not be empty.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");

	std::memset(&vDst, bVal ? -1 : 0, sizeof(vDst));
}

template<typename Tx, typename Ty>
int BComp(const Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(!std::is_empty<Tx>::value, "Tx shall not be empty.");
	static_assert(!std::is_empty<Ty>::value, "Ty shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	return std::memcmp(&vDst, &vSrc, sizeof(vSrc));
}

template<typename Tx, typename Ty>
void BSwap(Tx &vDst, Ty &vSrc) noexcept {
	static_assert(!std::is_empty<Tx>::value, "Tx shall not be empty.");
	static_assert(!std::is_empty<Ty>::value, "Ty shall not be empty.");
	static_assert(std::is_trivial<Tx>::value, "Tx must be a trivial type.");
	static_assert(std::is_trivial<Ty>::value, "Ty must be a trivial type.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	std::swap(reinterpret_cast<unsigned char (&)[sizeof(Tx)]>(vDst), reinterpret_cast<unsigned char (&)[sizeof(Ty)]>(vSrc));
}

template<typename Ty>
void BZero(Ty &vDst) noexcept {
	BFill(vDst, false);
}

}

#endif
