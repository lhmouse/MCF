// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_BYTE_SWAP_HPP_
#define MCF_BYTE_SWAP_HPP_

#include "BitsOf.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

namespace Impl {
	template<std::size_t BIT_COUNT_T>
	struct ByteSwapHelper {
		static_assert((BIT_COUNT_T, false), "Not supported.");
	};

	template<>
	struct ByteSwapHelper<8> {
		static constexpr std::uint8_t Do(std::uint8_t uVal) noexcept {
			return uVal;
		}
	};
	template<>
	struct ByteSwapHelper<16> {
		static constexpr std::uint16_t Do(std::uint16_t uVal) noexcept {
			return __MCF_MAKE_CONSTANT(__builtin_bswap16(uVal));
		}
	};
	template<>
	struct ByteSwapHelper<32> {
		static constexpr std::uint32_t Do(std::uint32_t uVal) noexcept {
			return __MCF_MAKE_CONSTANT(__builtin_bswap32(uVal));
		}
	};
	template<>
	struct ByteSwapHelper<64> {
		static constexpr std::uint64_t Do(std::uint64_t uVal) noexcept {
			return __MCF_MAKE_CONSTANT(__builtin_bswap64(uVal));
		}
	};
}

constexpr unsigned char ByteSwap(unsigned char by) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned char)>::Do(by);
}
constexpr unsigned short ByteSwap(unsigned short ush) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned short)>::Do(ush);
}
constexpr unsigned ByteSwap(unsigned u) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned)>::Do(u);
}
constexpr unsigned long ByteSwap(unsigned long ul) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned long)>::Do(ul);
}
constexpr unsigned long long ByteSwap(unsigned long long ull) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned long long)>::Do(ull);
}

}

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#	define BYTE_SWAP_IF_BE(expr)	(::MCF::ByteSwap(expr))
#	define BYTE_SWAP_IF_LE(expr)	(expr)
#else
#	define BYTE_SWAP_IF_BE(expr)	(expr)
#	define BYTE_SWAP_IF_LE(expr)	(::MCF::ByteSwap(expr))
#endif

#endif
