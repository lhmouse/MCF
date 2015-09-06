// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Crc32.hpp"
#include "../Utilities/Endian.hpp"

// http://www.relisoft.com/science/CrcOptim.html
// 1. 原文提供的是正序（权较大位向权较小位方向）的 Crc 计算，而这里使用的是反序（权较小位向权较大位方向）。
// 2. 原文的 Crc 余数的初始值是 0；此处以 -1 为初始值，计算完成后进行按位反。

namespace MCF {

// 构造函数和析构函数。
Crc32::Crc32(std::uint32_t u32Divisor) noexcept
	: x_bInited(false)
{
	for(std::uint32_t i = 0; i < 256; ++i){
		register std::uint32_t u32Reg = i;
		for(std::size_t j = 0; j < 8; ++j){
/*
			const bool bLowerBit = (u32Reg & 1) != 0;
			u32Reg >>= 1;
			if(bLowerBit){
				u32Reg ^= u32Divisor;
			}
*/
			__asm__ __volatile__(
				"shr %0, 1 \n"
				"sbb eax, eax \n"
				"and eax, %1 \n"
				"xor %0, eax \n"
				: "=r"(u32Reg)
				: "r"(u32Divisor), "0"(u32Reg)
				: "ax"
			);
		}
		x_au32Table[i] = u32Reg;
	}
}

// 其他非静态成员函数。
void Crc32::Abort() noexcept {
	x_bInited = false;
}
void Crc32::Update(const void *pData, std::size_t uSize) noexcept {
	if(!x_bInited){
		x_u32Reg = (std::uint32_t)-1;

		x_bInited = true;
	}

	const auto DoCrc32Byte = [&](unsigned char byData){
		x_u32Reg = x_au32Table[(x_u32Reg ^ byData) & 0xFF] ^ (x_u32Reg >> 8);
	};

	register auto pbyRead = (const unsigned char *)pData;
	const auto pbyEnd = pbyRead + uSize;

	if(uSize >= sizeof(std::uintptr_t) * 2){
		while(((std::uintptr_t)pbyRead & (sizeof(std::uintptr_t) - 1)) != 0){
			DoCrc32Byte(*pbyRead);
			++pbyRead;
		}
		register auto i = (std::size_t)(pbyEnd - pbyRead) / sizeof(std::uintptr_t);
		while(i != 0){
			register auto uWord = LoadLe(*(const std::uintptr_t *)pbyRead);
			pbyRead += sizeof(std::uintptr_t);
			for(unsigned j = 0; j < sizeof(std::uintptr_t); ++j){
				DoCrc32Byte(uWord & 0xFF);
				uWord >>= 8;
			}
			--i;
		}
	}
	while(pbyRead != pbyEnd){
		DoCrc32Byte(*pbyRead);
		++pbyRead;
	}
}
std::uint32_t Crc32::Finalize() noexcept {
	if(x_bInited){
		x_u32Reg = ~x_u32Reg;

		x_bInited = false;
	}
	return x_u32Reg;
}

}
