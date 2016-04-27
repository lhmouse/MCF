// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_ENDIAN_HPP_
#define MCF_UTILITIES_ENDIAN_HPP_

#include <type_traits>
#include <cstdint>

namespace MCF {

namespace Impl_Endian {
	template<unsigned kSizeT>
	struct ByteSwapper;

	template<>
	struct ByteSwapper<1> {
		std::uint8_t operator()(std::uint8_t u8Val) const noexcept {
			return u8Val;
		}
	};
	template<>
	struct ByteSwapper<2> {
		std::uint16_t operator()(std::uint16_t u16Val) const noexcept {
			return __builtin_bswap16(u16Val);
		}
	};
	template<>
	struct ByteSwapper<4> {
		std::uint32_t operator()(std::uint32_t u32Val) const noexcept {
			return __builtin_bswap32(u32Val);
		}
	};
	template<>
	struct ByteSwapper<8> {
		std::uint64_t operator()(std::uint64_t u64Val) const noexcept {
			return __builtin_bswap64(u64Val);
		}
	};
}

template<typename ValueT>
inline ValueT LoadLe(const ValueT &vMem) noexcept {
	static_assert(std::is_integral<ValueT>::value, "ValueT must be an integral type.");

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return vMem;
#else
	return static_cast<ValueT>(Impl_Endian::ByteSwapper<sizeof(vMem)>()(static_cast<std::make_unsigned_t<ValueT>>(vMem)));
#endif
}
template<typename ValueT>
inline ValueT LoadBe(const ValueT &vMem) noexcept {
	static_assert(std::is_integral<ValueT>::value, "ValueT must be an integral type.");

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return static_cast<ValueT>(Impl_Endian::ByteSwapper<sizeof(vMem)>()(static_cast<std::make_unsigned_t<ValueT>>(vMem)));
#else
	return vMem;
#endif
}

template<typename ValueT>
inline void StoreLe(ValueT &vMem, std::common_type_t<ValueT> vVal) noexcept {
	static_assert(std::is_integral<ValueT>::value, "ValueT must be an integral type.");

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	vMem = vVal;
#else
	vMem = static_cast<ValueT>(Impl_Endian::ByteSwapper<sizeof(vMem)>()(static_cast<std::make_unsigned_t<ValueT>>(vVal)));
#endif
}
template<typename ValueT>
inline void StoreBe(ValueT &vMem, std::common_type_t<ValueT> vVal) noexcept {
	static_assert(std::is_integral<ValueT>::value, "ValueT must be an integral type.");

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	vMem = static_cast<ValueT>(Impl_Endian::ByteSwapper<sizeof(vMem)>()(static_cast<std::make_unsigned_t<ValueT>>(vVal)));
#else
	vMem = vVal;
#endif
}

}

#endif
