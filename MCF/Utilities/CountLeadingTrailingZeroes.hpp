// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_COUNT_LEADING_TRAILING_ZEROES_HPP_
#define MCF_UTILITIES_COUNT_LEADING_TRAILING_ZEROES_HPP_

#include "../../MCFCRT/ext/_make_constant.h"
#include <cstdint>
#include <climits>
#include <cstddef>

namespace MCF {

constexpr unsigned char CountLeadingZeroes(unsigned char by) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clz(by)
		- (sizeof(unsigned) - sizeof(unsigned char)) * (std::size_t)CHAR_BIT);
}
constexpr unsigned char CountLeadingZeroes(unsigned short ush) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clz(ush)
		- (sizeof(unsigned) - sizeof(unsigned short)) * (std::size_t)CHAR_BIT);
}
constexpr unsigned char CountLeadingZeroes(unsigned u) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clz(u));
}
constexpr unsigned char CountLeadingZeroes(unsigned long ul) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clzl(ul));
}
constexpr unsigned char CountLeadingZeroes(unsigned long long ull) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clzll(ull));
}

constexpr unsigned char CountTrailingZeroes(unsigned char by) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctz(by));
}
constexpr unsigned char CountTrailingZeroes(unsigned short ush) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctz(ush));
}
constexpr unsigned char CountTrailingZeroes(unsigned u) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctz(u));
}
constexpr unsigned char CountTrailingZeroes(unsigned long ul) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctzl(ul));
}
constexpr unsigned char CountTrailingZeroes(unsigned long long ull) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctzll(ull));
}

}

#endif
