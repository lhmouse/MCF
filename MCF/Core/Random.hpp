// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_RANDOM_HPP__
#define __MCF_RANDOM_HPP__

#include <cstdint>
#include <cstddef>

namespace MCF {

extern std::uint32_t GenRandomSeed() noexcept;

}

#endif
