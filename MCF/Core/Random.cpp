// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Random.hpp"
using namespace MCF;

namespace MCF {

std::uint32_t GenRandSeed() noexcept {
	register std::uint32_t ret __asm__("ax");
	__asm__ __volatile__(
		"rdtsc \n"
		: "=a"(ret)
		:
		: "dx"
	);
	return ret;
}

// ========== Random ==========
// http://en.wikipedia.org/wiki/RC4

// 构造函数和析构函数。
Random::Random(std::uint32_t u32Seed) noexcept {
	for(std::size_t i = 0; i < 256; ++i){
		xm_abyBox[i] = i;
	}
	unsigned char j = 0;
	for(std::size_t i = 0; i < 256; i += 4){
		for(std::size_t k = 0; k < 4; ++k){
			const unsigned char b0 = xm_abyBox[i + k];
			j += b0 + (u32Seed >> (k * 8));
			const unsigned char b1 = xm_abyBox[j];
			xm_abyBox[i + k] = b1;
			xm_abyBox[j] = b0;
		}
	}
	xm_byI = 0;
	xm_byJ = 0;
}

// 其他非静态成员函数。
std::uint8_t Random::xGetByte() noexcept {
	++xm_byI;
	const unsigned char b0 = xm_abyBox[xm_byI];
	xm_byJ += b0;
	const unsigned char b1 = xm_abyBox[xm_byJ];
	xm_abyBox[xm_byI] = b1;
	xm_abyBox[xm_byJ] = b0;
	return xm_abyBox[(b0 + b1) & 0xFF];
}

}
