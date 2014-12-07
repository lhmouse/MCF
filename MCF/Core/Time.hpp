// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_TIME_HPP_
#define MCF_CORE_TIME_HPP_

#include <functional>
#include <cstdint>

namespace MCF {

// NT 时间戳单位是 1e-7 秒。
// Unix 时间戳单位是秒。
extern std::uint64_t GetNtTime() noexcept;
extern std::uint64_t GetUnixTime() noexcept;
extern std::uint64_t NtTimeFromUnixTime(std::uint64_t u64UnixTime) noexcept;
extern std::uint64_t UnixTimeFromNtTime(std::uint64_t u64NtTime) noexcept;

// 单位是毫秒。
extern std::uint64_t GetFastMonoClock() noexcept;
extern double GetHiResMonoClock() noexcept;

}

#endif
