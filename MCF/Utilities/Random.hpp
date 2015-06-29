// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_RANDOM_HPP_
#define MCF_UTILITIES_RANDOM_HPP_

#include <cstdint>

namespace MCF {

extern std::uint32_t GetRandomUint32() noexcept;
extern std::uint64_t GetRandomUint64() noexcept;
extern double GetRandomDouble() noexcept;

}

#endif
