// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_RANDOM_ISAAC_GENERATOR_HPP_
#define MCF_RANDOM_ISAAC_GENERATOR_HPP_

#include "../Core/Time.hpp"

namespace MCF {

class IsaacGenerator {
private:
	std::uint32_t $u32Internal[256];
	std::uint32_t $u32A;
	std::uint32_t $u32B;
	std::uint32_t $u32C;

	std::uint32_t $u32Results[256];
	std::uint32_t $u32Read;

public:
	explicit IsaacGenerator(std::uint32_t u32Seed = ReadTimestampCounter32()) noexcept {
		Init(u32Seed);
	}
	explicit IsaacGenerator(const std::uint32_t (&au32Seed)[8]) noexcept {
		Init(au32Seed);
	}

private:
	void $RefreshInternal() noexcept;

public:
	void Init(std::uint32_t u32Seed = ReadTimestampCounter32()) noexcept;
	void Init(const std::uint32_t (&au32Seed)[8]) noexcept;

	std::uint32_t Get() noexcept;

public:
	std::uint32_t operator()() noexcept {
		return Get();
	}
};

}

#endif
