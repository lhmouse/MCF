// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_TIME_HPP_
#define MCF_TIME_HPP_

#include <cstdint>

namespace MCF {

extern std::uint64_t GetNtTime() noexcept;
extern std::uint64_t NtTimeFromUnixTime(std::uint64_t u64UnixTime) noexcept;
extern std::uint64_t UnixTimeFromNtTime(std::uint64_t u64NtTime) noexcept;

extern double GetHiResCounter() noexcept;

}

#endif
