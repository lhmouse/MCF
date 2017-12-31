// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_COUNT_LEADING_TRAILING_ZEROES_HPP_
#define MCF_CORE_COUNT_LEADING_TRAILING_ZEROES_HPP_

#include <climits>

namespace MCF {

constexpr unsigned CountLeadingZeroes(unsigned char byValue) noexcept {
	return (unsigned)__builtin_clz(byValue) - (unsigned)(sizeof(unsigned) - 1) * CHAR_BIT;
}
constexpr unsigned CountLeadingZeroes(unsigned short ushValue) noexcept {
	return (unsigned)__builtin_clz(ushValue) - (unsigned)(sizeof(unsigned) - 1) * CHAR_BIT;
}
constexpr unsigned CountLeadingZeroes(unsigned uValue) noexcept {
	return (unsigned)__builtin_clz(uValue);
}
constexpr unsigned CountLeadingZeroes(unsigned long ulValue) noexcept {
	return (unsigned)__builtin_clzl(ulValue);
}
constexpr unsigned CountLeadingZeroes(unsigned long long ullValue) noexcept {
	return (unsigned)__builtin_clzll(ullValue);
}

constexpr unsigned CountTrailingZeroes(unsigned char byValue) noexcept {
	return (unsigned)__builtin_ctz(byValue);
}
constexpr unsigned CountTrailingZeroes(unsigned short ushValue) noexcept {
	return (unsigned)__builtin_ctz(ushValue);
}
constexpr unsigned CountTrailingZeroes(unsigned uValue) noexcept {
	return (unsigned)__builtin_ctz(uValue);
}
constexpr unsigned CountTrailingZeroes(unsigned long ulValue) noexcept {
	return (unsigned)__builtin_ctzl(ulValue);
}
constexpr unsigned CountTrailingZeroes(unsigned long long ullValue) noexcept {
	return (unsigned)__builtin_ctzll(ullValue);
}

}

#endif
