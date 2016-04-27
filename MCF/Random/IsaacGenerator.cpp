// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "IsaacGenerator.hpp"
#include "../Utilities/CopyMoveFill.hpp"

namespace MCF {

// http://www.burtleburtle.net/bob/rand/isaacafa.html
// http://www.burtleburtle.net/bob/c/readable.c

void IsaacGenerator::X_RefreshInternal() noexcept {
	++x_u32C;
	x_u32B += x_u32C;

	for(std::size_t i = 0; i < 256; i += 4){
		const auto Step = [&](unsigned j, auto fnSpec){
			const auto x = x_u32Internal[i + j];
			fnSpec(x_u32A);
			x_u32A += x_u32Internal[(i + j + 128) % 256];
			const auto y = x_u32Internal[(x >> 2) % 256] + x_u32A + x_u32B;
			x_u32Internal[i + j] = y;
			x_u32B = x_u32Internal[(y >> 10) % 256] + x;
			x_au32Results[i + j] = x_u32B;
		};

		Step(0, [](auto &a){ a ^= (a << 13); });
		Step(1, [](auto &a){ a ^= (a >>  6); });
		Step(2, [](auto &a){ a ^= (a <<  2); });
		Step(3, [](auto &a){ a ^= (a >> 16); });
	}
}

void IsaacGenerator::Init(std::uint32_t u32Seed) noexcept {
	Array<std::uint32_t, 8> au32Seed;
	Fill(au32Seed.GetBegin(), au32Seed.GetEnd(), u32Seed);
	Init(au32Seed);
}
void IsaacGenerator::Init(const Array<std::uint32_t, 8> &au32Seed) noexcept {
	std::uint32_t au32Temp[8];
	FillN(au32Temp, 8, 0x9E3779B9u);

	const auto Mix = [&](){
		au32Temp[0] ^= (au32Temp[1] << 11); au32Temp[3] += au32Temp[0]; au32Temp[1] += au32Temp[2];
		au32Temp[1] ^= (au32Temp[2] >>  2); au32Temp[4] += au32Temp[1]; au32Temp[2] += au32Temp[3];
		au32Temp[2] ^= (au32Temp[3] <<  8); au32Temp[5] += au32Temp[2]; au32Temp[3] += au32Temp[4];
		au32Temp[3] ^= (au32Temp[4] >> 16); au32Temp[6] += au32Temp[3]; au32Temp[4] += au32Temp[5];
		au32Temp[4] ^= (au32Temp[5] << 10); au32Temp[7] += au32Temp[4]; au32Temp[5] += au32Temp[6];
		au32Temp[5] ^= (au32Temp[6] >>  4); au32Temp[0] += au32Temp[5]; au32Temp[6] += au32Temp[7];
		au32Temp[6] ^= (au32Temp[7] <<  8); au32Temp[1] += au32Temp[6]; au32Temp[7] += au32Temp[0];
		au32Temp[7] ^= (au32Temp[0] >>  9); au32Temp[2] += au32Temp[7]; au32Temp[0] += au32Temp[1];
	};

	for(std::size_t i = 0; i < 4; ++i){
		Mix();
	}
	for(std::size_t i = 0; i < 256; i += 8){
		for(std::size_t j = 0; j < 8; ++j){
			au32Temp[j] += au32Seed[j];
		}
		Mix();
		CopyN(x_u32Internal + i, au32Temp, 8);
	}
	for(std::size_t i = 0; i < 256; i += 8){
		for(std::size_t j = 0; j < 8; ++j){
			au32Temp[j] += x_u32Internal[i + j];
		}
		Mix();
		CopyN(x_u32Internal + i, au32Temp, 8);
	}
	x_u32A = 0;
	x_u32B = 0;
	x_u32C = 0;
	X_RefreshInternal();

	x_uRead = 0;
}

std::uint32_t IsaacGenerator::Get() noexcept {
	if(x_uRead == 0){
		X_RefreshInternal();
	}
	const auto u32Ret = x_au32Results[x_uRead];
	x_uRead = (x_uRead + 1) % 256;
	return u32Ret;
}

}
