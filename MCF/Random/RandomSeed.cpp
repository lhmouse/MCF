// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RandomSeed.hpp"
using namespace MCF;

namespace MCF {

std::uint32_t GenerateRandSeed() noexcept {
	register std::uint32_t ret __asm__("eax");
	__asm__ __volatile__(
		"rdtsc \n"
		: "=a"(ret)
		:
		: "dx"
	);
	return ret;
}

}
