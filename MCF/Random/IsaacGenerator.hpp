// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_RANDOM_ISAAC_GENERATOR_HPP_
#define MCF_RANDOM_ISAAC_GENERATOR_HPP_

#include "../Core/Clocks.hpp"
#include "../Core/Array.hpp"

namespace MCF {

class IsaacGenerator {
private:
	std::uint32_t x_u32Internal[256];
	std::uint32_t x_u32A;
	std::uint32_t x_u32B;
	std::uint32_t x_u32C;

	std::uint32_t x_au32Results[256];
	std::uint32_t x_u32Read;

public:
	explicit IsaacGenerator(std::uint32_t u32Seed = ReadTimestampCounter32()) noexcept {
		Init(u32Seed);
	}
	explicit IsaacGenerator(const Array<std::uint32_t, 8> &au32Seed) noexcept {
		Init(au32Seed);
	}

public:
	void Init(std::uint32_t u32Seed = ReadTimestampCounter32()) noexcept;
	void Init(const Array<std::uint32_t, 8> &au32Seed) noexcept;

	std::uint32_t Get() noexcept;

public:
	std::uint32_t operator()() noexcept {
		return Get();
	}
};

}

#endif
