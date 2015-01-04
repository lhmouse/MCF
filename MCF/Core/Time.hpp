// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_TIME_HPP_
#define MCF_CORE_TIME_HPP_

#include <cstdint>

namespace MCF {

inline std::uint32_t ReadTimestampCounterLow() noexcept {
	std::uint32_t u32Ret;
	__asm__ __volatile__(
		"rdtsc \n"
		: "=a"(u32Ret) : : "dx"
		);
	return u32Ret;
}
inline std::uint64_t ReadTimestampCounter() noexcept {
	std::uint64_t u64Ret;
	__asm__ __volatile__(
		"rdtsc \n"
#ifdef _WIN64
		"shl rdx, 32 \n"
		"or rax, rdx \n"
		: "=a"(u64Ret) : : "dx"
#else
		: "=A"(u64Ret) : :
#endif
		);
	return u64Ret;
}

// 单位是毫秒。
extern std::uint64_t GetUtcTime() noexcept;
extern std::uint64_t GetLocalTime() noexcept;

extern std::uint64_t GetFastMonoClock() noexcept;
extern double GetHiResMonoClock() noexcept;

}

#endif
