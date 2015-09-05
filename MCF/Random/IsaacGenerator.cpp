// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "IsaacGenerator.hpp"
#include "../Utilities/CopyMoveFill.hpp"

namespace MCF {

// http://www.burtleburtle.net/bob/rand/isaacafa.html
// http://www.burtleburtle.net/bob/c/readable.c

// 其他非静态成员函数。
void IsaacGenerator::Init(std::uint32_t u32Seed) noexcept {
	std::uint32_t au32RealSeed[8];
	FillN(au32RealSeed, 8, u32Seed);
	Init(au32RealSeed);
}
void IsaacGenerator::Init(const std::uint32_t (&au32Seed)[8]) noexcept {
	std::uint32_t au32Temp[8];
	FillN(au32Temp, 8, 0x9E3779B9u);

	const auto Mix = [&]() noexcept {
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
		CopyN($u32Internal + i, au32Temp, 8);
	}
	for(std::size_t i = 0; i < 256; i += 8){
		for(std::size_t j = 0; j < 8; ++j){
			au32Temp[j] += $u32Internal[i + j];
		}
		Mix();
		CopyN($u32Internal + i, au32Temp, 8);
	}

	$u32A = 0;
	$u32B = 0;
	$u32C = 0;
	$RefreshInternal();

	$u32Read = 0;
}

void IsaacGenerator::$RefreshInternal() noexcept {
	++$u32C;
	$u32B += $u32C;

	for(std::size_t i = 0; i < 256; i += 4){
		const auto Step = [&](unsigned j, auto fnSpec){
			const auto x = $u32Internal[i + j];
			fnSpec($u32A);
			$u32A += $u32Internal[(i + j + 128) % 256];
			const auto y = $u32Internal[(x >> 2) % 256] + $u32A + $u32B;
			$u32Internal[i + j] = y;
			$u32B = $u32Internal[(y >> 10) % 256] + x;
			$u32Results[i + j] = $u32B;
		};

		Step(0, [](auto &a){ a ^= (a << 13); });
		Step(1, [](auto &a){ a ^= (a >>  6); });
		Step(2, [](auto &a){ a ^= (a <<  2); });
		Step(3, [](auto &a){ a ^= (a >> 16); });
	}
}

std::uint32_t IsaacGenerator::Get() noexcept {
	if($u32Read == 0){
		$RefreshInternal();
	}
	const auto u32Ret = $u32Results[$u32Read];
	$u32Read = ($u32Read + 1) % 256;
	return u32Ret;
}

}
