// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_CLOCKS_HPP_
#define MCF_CORE_CLOCKS_HPP_

#include "../../MCFCRT/env/clocks.h"

namespace MCF {

inline std::uint32_t ReadTimestampCounter32() noexcept {
	return ::MCFCRT_ReadTimestampCounter32();
}
inline std::uint64_t ReadTimestampCounter64() noexcept {
	return ::MCFCRT_ReadTimestampCounter64();
}

inline std::uint64_t GetUtcClock() noexcept {
	return ::MCFCRT_GetUtcClock();
}
inline std::uint64_t GetLocalClock() noexcept {
	return ::MCFCRT_GetLocalClock();
}

inline std::uint64_t GetFastMonoClock() noexcept {
	return ::MCFCRT_GetFastMonoClock();
}
inline double GetHiResMonoClock() noexcept {
	return ::MCFCRT_GetHiResMonoClock();
}

}

#endif
