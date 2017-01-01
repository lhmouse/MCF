// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_COUNT_LEADING_TRAILING_ZEROES_HPP_
#define MCF_CORE_COUNT_LEADING_TRAILING_ZEROES_HPP_

#include <cstdint>
#include <climits>
#include <cstddef>

namespace MCF {

inline unsigned CountLeadingZeroes(unsigned char by) noexcept {
	return (unsigned)__builtin_clz(by) - (unsigned)(sizeof(unsigned) - sizeof(unsigned char)) * CHAR_BIT;
}
inline unsigned CountLeadingZeroes(unsigned short ush) noexcept {
	return (unsigned)__builtin_clz(ush) - (unsigned)(sizeof(unsigned) - sizeof(unsigned short)) * CHAR_BIT;
}
inline unsigned CountLeadingZeroes(unsigned u) noexcept {
	return (unsigned)__builtin_clz(u);
}
inline unsigned CountLeadingZeroes(unsigned long ul) noexcept {
	return (unsigned)__builtin_clzl(ul);
}
inline unsigned CountLeadingZeroes(unsigned long long ull) noexcept {
	return (unsigned)__builtin_clzll(ull);
}

inline unsigned CountTrailingZeroes(unsigned char by) noexcept {
	return (unsigned)__builtin_ctz(by);
}
inline unsigned CountTrailingZeroes(unsigned short ush) noexcept {
	return (unsigned)__builtin_ctz(ush);
}
inline unsigned CountTrailingZeroes(unsigned u) noexcept {
	return (unsigned)__builtin_ctz(u);
}
inline unsigned CountTrailingZeroes(unsigned long ul) noexcept {
	return (unsigned)__builtin_ctzl(ul);
}
inline unsigned CountTrailingZeroes(unsigned long long ull) noexcept {
	return (unsigned)__builtin_ctzll(ull);
}

}

#endif
