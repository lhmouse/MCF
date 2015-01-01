// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_RANDOM_FAST_GENERATOR_HPP_
#define MCF_RANDOM_FAST_GENERATOR_HPP_

#include "RandomSeed.hpp"

namespace MCF {

class FastGenerator {
public:
	static std::uint32_t GlobalGet() noexcept;

private:
	std::uint64_t xm_u64Seed;

public:
	constexpr FastGenerator() noexcept
		: xm_u64Seed(0x0123456789ABCDEF)
	{
	}
	explicit FastGenerator(std::uint32_t u32Seed) noexcept {
		Init(u32Seed);
	}

public:
	void Init(std::uint32_t u32Seed = GenerateRandSeed()) noexcept;

	std::uint32_t Get() noexcept;
};

}

#endif
