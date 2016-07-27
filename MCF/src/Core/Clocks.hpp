// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_CLOCKS_HPP_
#define MCF_CORE_CLOCKS_HPP_

#include <MCFCRT/env/clocks.h>

namespace MCF {

inline std::uint32_t ReadTimeStampCounter32() noexcept {
	return ::_MCFCRT_ReadTimeStampCounter32();
}
inline std::uint64_t ReadTimeStampCounter64() noexcept {
	return ::_MCFCRT_ReadTimeStampCounter64();
}

inline std::uint64_t GetUtcClock() noexcept {
	return ::_MCFCRT_GetUtcClock();
}
inline std::uint64_t GetLocalClock() noexcept {
	return ::_MCFCRT_GetLocalClock();
}

inline std::uint64_t GetUtcClockFromLocal(std::uint64_t u64LocalClock) noexcept {
	return ::_MCFCRT_GetUtcClockFromLocal(u64LocalClock);
}
inline std::uint64_t GetLocalClockFromUtc(std::uint64_t u64UtcClock) noexcept {
	return ::_MCFCRT_GetLocalClockFromUtc(u64UtcClock);
}

inline std::uint64_t GetFastMonoClock() noexcept {
	return ::_MCFCRT_GetFastMonoClock();
}
inline double GetHiResMonoClock() noexcept {
	return ::_MCFCRT_GetHiResMonoClock();
}

}

#endif
