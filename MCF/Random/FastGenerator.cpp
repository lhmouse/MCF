// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "FastGenerator.hpp"
#include "../Utilities/CopyMoveFill.hpp"

namespace MCF {

// http://en.wikipedia.org/wiki/Linear_congruential_generator

// 其他非静态成员函数。
void FastGenerator::Init(std::uint32_t u32Seed) noexcept {
	xm_u64Seed = 0x0123456789ABCDEFull | u32Seed;
}

std::uint32_t FastGenerator::Get() noexcept {
	// MMIX by Donald Knuth
	const auto u64NewSeed = xm_u64Seed  * 6364136223846793005ull + 1442695040888963407ull;
	xm_u64Seed = u64NewSeed;
	return u64NewSeed >> 32;
}

}
