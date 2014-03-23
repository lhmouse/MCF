// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TIME_HPP__
#define __MCF_TIME_HPP__

#include <cstdint>

namespace MCF {

extern std::uint64_t GetUnixTime() noexcept;

extern double GetHiResCounter() noexcept;

}

#endif
