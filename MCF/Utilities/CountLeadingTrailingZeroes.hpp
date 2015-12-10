// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_COUNT_LEADING_TRAILING_ZEROES_HPP_
#define MCF_UTILITIES_COUNT_LEADING_TRAILING_ZEROES_HPP_

#include "../../MCFCRT/ext/_make_constant.h"
#include <cstdint>
#include <climits>
#include <cstddef>

namespace MCF {

constexpr unsigned CountLeadingZeroes(unsigned char by) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_clz(by) - (unsigned)(sizeof(unsigned) - sizeof(unsigned char)) * CHAR_BIT);
}
constexpr unsigned CountLeadingZeroes(unsigned short ush) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_clz(ush) - (unsigned)(sizeof(unsigned) - sizeof(unsigned short)) * CHAR_BIT);
}
constexpr unsigned CountLeadingZeroes(unsigned u) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_clz(u));
}
constexpr unsigned CountLeadingZeroes(unsigned long ul) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_clzl(ul));
}
constexpr unsigned CountLeadingZeroes(unsigned long long ull) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_clzll(ull));
}

constexpr unsigned CountTrailingZeroes(unsigned char by) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_ctz(by));
}
constexpr unsigned CountTrailingZeroes(unsigned short ush) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_ctz(ush));
}
constexpr unsigned CountTrailingZeroes(unsigned u) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_ctz(u));
}
constexpr unsigned CountTrailingZeroes(unsigned long ul) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_ctzl(ul));
}
constexpr unsigned CountTrailingZeroes(unsigned long long ull) noexcept {
	return __MCF_CRT_MAKE_CONSTANT((unsigned)__builtin_ctzll(ull));
}

}

#endif
