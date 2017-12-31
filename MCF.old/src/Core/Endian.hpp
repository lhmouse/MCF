// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs resermed.

#ifndef MCF_CORE_ENDIAN_HPP_
#define MCF_CORE_ENDIAN_HPP_

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace MCF {

namespace Impl_Endian {
	template<typename ValueT>
	inline ValueT SwapBytes(const std::integral_constant<std::size_t, 1> &, ValueT v) noexcept {
		return static_cast<ValueT>(                 (static_cast<std::uint8_t >(v)));
	}
	template<typename ValueT>
	inline ValueT SwapBytes(const std::integral_constant<std::size_t, 2> &, ValueT v) noexcept {
		return static_cast<ValueT>(__builtin_bswap16(static_cast<std::uint16_t>(v)));
	}
	template<typename ValueT>
	inline ValueT SwapBytes(const std::integral_constant<std::size_t, 4> &, ValueT v) noexcept {
		return static_cast<ValueT>(__builtin_bswap32(static_cast<std::uint32_t>(v)));
	}
	template<typename ValueT>
	inline ValueT SwapBytes(const std::integral_constant<std::size_t, 8> &, ValueT v) noexcept {
		return static_cast<ValueT>(__builtin_bswap64(static_cast<std::uint64_t>(v)));
	}
}

template<typename ValueT>
inline ValueT LoadLe(const ValueT &m) noexcept {
	static_assert(std::is_integral<ValueT>::value, "ValueT must be an integral type.");

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return m;
#else
	return Impl_Endian::SwapBytes(std::integral_constant<std::size_t, sizeof(ValueT)>(), m);
#endif
}
template<typename ValueT>
inline ValueT LoadBe(const ValueT &m) noexcept {
	static_assert(std::is_integral<ValueT>::value, "ValueT must be an integral type.");

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return Impl_Endian::SwapBytes(std::integral_constant<std::size_t, sizeof(ValueT)>(), m);
#else
	return m;
#endif
}

template<typename ValueT>
inline void StoreLe(ValueT &m, std::common_type_t<ValueT> v) noexcept {
	static_assert(std::is_integral<ValueT>::value, "ValueT must be an integral type.");

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	m = v;
#else
	m = Impl_Endian::SwapBytes<>(std::integral_constant<std::size_t, sizeof(ValueT)>(), v);
#endif
}
template<typename ValueT>
inline void StoreBe(ValueT &m, std::common_type_t<ValueT> v) noexcept {
	static_assert(std::is_integral<ValueT>::value, "ValueT must be an integral type.");

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	m = Impl_Endian::SwapBytes<>(std::integral_constant<std::size_t, sizeof(ValueT)>(), v);
#else
	m = v;
#endif
}

}

#endif
