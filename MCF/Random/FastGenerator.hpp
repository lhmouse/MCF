// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_RANDOM_FAST_GENERATOR_HPP_
#define MCF_RANDOM_FAST_GENERATOR_HPP_

#include "../Core/Time.hpp"

namespace MCF {

class FastGenerator {
private:
	std::uint64_t x_u64Seed;

public:
	explicit FastGenerator(std::uint32_t u32Seed = ReadTimestampCounter32()) noexcept {
		Init(u32Seed);
	}

public:
	void Init(std::uint32_t u32Seed = ReadTimestampCounter32()) noexcept;

	std::uint32_t Get() noexcept;

public:
	std::uint32_t operator()() noexcept {
		return Get();
	}
};

}

#endif
