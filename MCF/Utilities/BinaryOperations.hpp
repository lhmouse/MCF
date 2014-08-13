// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_BINARY_OPERATIONS_HPP_
#define MCF_BINARY_OPERATIONS_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

template<typename Tx, typename Ty>
constexpr void BCopy(Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "Only trivial types are supported.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	__builtin_memcpy(&vDst, &vSrc, sizeof(vDst));
}

template<typename T>
constexpr void BFill(T &vDst, bool bVal) noexcept {
	static_assert(std::is_trivial<T>::value, "Only trivial types are supported.");

	__builtin_memset(&vDst, bVal ? -1 : 0, sizeof(vDst));
}

template<typename Tx, typename Ty>
constexpr int BComp(const Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "Only trivial types are supported.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	return __builtin_memcmp(&vDst, &vSrc, sizeof(vSrc));
}

template<typename Tx, typename Ty>
constexpr void BSwap(Tx &vDst, Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "Only trivial types are supported.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	std::swap(vDst, reinterpret_cast<Tx &>(vSrc));
}

template<typename T>
constexpr void BZero(T &vDst) noexcept {
	BFill(vDst, false);
}

}

#endif
