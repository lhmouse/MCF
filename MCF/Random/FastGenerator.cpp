// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "FastGenerator.hpp"

namespace MCF {

// http://en.wikipedia.org/wiki/Linear_congruential_generator
// MMIX by Donald Knuth

namespace {
	enum : std::uint64_t {
		kMultiplier = 6364136223846793005ull,
		kIncrement  = 1442695040888963407ull,
	};
}

// 其他非静态成员函数。
void FastGenerator::Init(std::uint32_t u32Seed) noexcept {
	auto u64Seed = 0x0123456789ABCDEFull | u32Seed;
	for(unsigned i = 0; i < 8; ++i){
		u64Seed = u64Seed * kMultiplier + kIncrement;
	}
	x_u64Seed = u64Seed;
}

std::uint32_t FastGenerator::Get() noexcept {
	const auto u64NewSeed = x_u64Seed * kMultiplier + kIncrement;
	x_u64Seed = u64NewSeed;
	return u64NewSeed >> 32;
}

}
