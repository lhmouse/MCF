// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "IsaacRng.hpp"
#include "../Utilities/Algorithms.hpp"
using namespace MCF;

// http://www.burtleburtle.net/bob/c/readable.c

// 构造函数和析构函数。
IsaacRng::IsaacRng(std::uint32_t u32Seed) noexcept {
	Init(u32Seed);
}
IsaacRng::IsaacRng(const std::uint32_t (&au32Seed)[8]) noexcept {
	Init(au32Seed);
}

// 其他非静态成员函数。
void IsaacRng::Init(std::uint32_t u32Seed) noexcept {
	std::uint32_t au32RealSeed[8];
	FillN(au32RealSeed, 8, u32Seed);
	Init(au32RealSeed);
}
void IsaacRng::Init(const std::uint32_t (&au32Seed)[8]) noexcept {
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
		CopyN(xm_u32Internal + i, au32Temp, 8);
	}
	for(std::size_t i = 0; i < 256; i += 8){
		for(std::size_t j = 0; j < 8; ++j){
			au32Temp[j] += xm_u32Internal[i + j];
		}
		Mix();
		CopyN(xm_u32Internal + i, au32Temp, 8);
	}

	xm_u32A = 0;
	xm_u32B = 0;
	xm_u32C = 0;

	xm_u32Read = 0;
}

std::uint32_t IsaacRng::Get() noexcept {
	if(xm_u32Read == 0){
		++xm_u32C;
		xm_u32B += xm_u32C;

		for(std::size_t i = 0; i < 256; i += 4){
			register std::uint32_t x, y;

#define SPEC_0	(xm_u32A ^= (xm_u32A << 13))
#define SPEC_1	(xm_u32A ^= (xm_u32A >>  6))
#define SPEC_2	(xm_u32A ^= (xm_u32A <<  2))
#define SPEC_3	(xm_u32A ^= (xm_u32A >> 16))

#define STEP(j_, spec_)	\
			x = xm_u32Internal[i + j_];	\
			spec_;	\
			xm_u32A += xm_u32Internal[(i + j_ + 128) % 256];	\
			y = xm_u32Internal[(x >> 2) % 256] + xm_u32A + xm_u32B;	\
			xm_u32Internal[i + j_] = y;	\
			xm_u32B = xm_u32Internal[(y >> 10) % 256] + x;	\
			xm_u32Results[i + j_] = xm_u32B;

			STEP(0, SPEC_0);
			STEP(1, SPEC_1);
			STEP(2, SPEC_2);
			STEP(3, SPEC_3);
		}
	}
	const auto u32Ret = xm_u32Results[xm_u32Read];
	xm_u32Read = (xm_u32Read + 1) % 256;
	return u32Ret;
}
