// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UTILITIES_HPP__
#define __MCF_UTILITIES_HPP__

#include "String.hpp"
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

#ifdef NDEBUG
[[noreturn]]
#endif
extern void Bail(const wchar_t *pwszDescription);

extern UTF16String GetWin32ErrorDesc(unsigned long ulErrorCode);

extern unsigned int GetUnixTime() noexcept;
extern std::uint32_t GenRandSeed() noexcept;


typedef struct tagHiResCounter {
	enum : std::size_t {
		SECOND_BITS = 40
	};
	std::uint64_t u40Sec : SECOND_BITS;
	std::uint64_t u24Rem : 64 - SECOND_BITS;
} HI_RES_COUNTER;

extern HI_RES_COUNTER GetHiResCounter() noexcept;

template<typename T>
inline void ZeroObject(T &dst) noexcept {
	static_assert(std::is_trivial<T>::value, "ZeroObject(): Only trivial types are supported");
	__builtin_memset(&dst, 0, sizeof(dst));
}

}

#endif
