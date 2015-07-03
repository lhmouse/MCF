// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_TIME_HPP_
#define MCF_CORE_TIME_HPP_

#include "../../MCFCRT/env/system.h"

namespace MCF {

inline std::uint32_t ReadTimestampCounter32() noexcept {
	return MCF_ReadTimestampCounter32();
}
inline std::uint32_t ReadTimestampCounter64() noexcept {
	return MCF_ReadTimestampCounter64();
}

inline std::uint64_t GetUtcTime() noexcept {
	return MCF_GetUtcTime();
}
inline std::uint64_t GetLocalTime() noexcept {
	return MCF_GetLocalTime();
}

inline std::uint64_t GetFastMonoClock() noexcept {
	return MCF_GetFastMonoClock();
}
inline double GetHiResMonoClock() noexcept {
	return MCF_GetHiResMonoClock();
}

}

#endif
