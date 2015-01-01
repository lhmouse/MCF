// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_TIME_HPP_
#define MCF_CORE_TIME_HPP_

#include <functional>
#include <cstdint>

namespace MCF {

// 单位是毫秒。
extern std::uint64_t GetUtcTime() noexcept;
extern std::uint64_t GetLocalTime() noexcept;

extern std::uint64_t GetFastMonoClock() noexcept;
extern double GetHiResMonoClock() noexcept;

}

#endif
