// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Crc32.hpp"
#include "../Utilities/Endian.hpp"
using namespace MCF;

namespace {

// http://www.relisoft.com/science/CrcOptim.html
// 1. 原文提供的是正序（权较大位向权较小位方向）的 Crc 计算，而这里使用的是反序（权较小位向权较大位方向）。
// 2. 原文的 Crc 余数的初始值是 0；此处以 -1 为初始值，计算完成后进行按位反。
void BuildTable(std::uint32_t (&au32Table)[0x100], std::uint32_t u32Divisor) noexcept {
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
		au32Table[i] = u32Reg;
	}
}
inline void DoCrc32Byte(std::uint32_t &u32Reg, const std::uint32_t (&au32Table)[0x100], unsigned char byData) noexcept {
	u32Reg = au32Table[(u32Reg ^ byData) & 0xFF] ^ (u32Reg >> 8);
}

}

// 构造函数和析构函数。
Crc32::Crc32(std::uint32_t u32Divisor) noexcept
	: xm_bInited(false)
{
	ASSERT(u32Divisor != 0);

	BuildTable(xm_au32Table, u32Divisor);
}

// 其他非静态成员函数。
void Crc32::Abort() noexcept{
	xm_bInited = false;
}
void Crc32::Update(const void *pData, std::size_t uSize) noexcept {
	if(!xm_bInited){
		xm_u32Reg = (std::uint32_t)-1;

		xm_bInited = true;
	}

	register auto pbyRead = (const unsigned char *)pData;
	const auto pbyEnd = pbyRead + uSize;

	if(uSize >= sizeof(std::uintptr_t) * 2){
		while(((std::uintptr_t)pbyRead & (sizeof(std::uintptr_t) - 1)) != 0){
			DoCrc32Byte(xm_u32Reg, xm_au32Table, *pbyRead);
			++pbyRead;
		}
		register auto i = (std::size_t)(pbyEnd - pbyRead) / sizeof(std::uintptr_t);
		while(i != 0){
			register auto uWord = LoadLe(*(const std::uintptr_t *)pbyRead);
			pbyRead += sizeof(std::uintptr_t);
			for(unsigned j = 0; j < sizeof(std::uintptr_t); ++j){
				DoCrc32Byte(xm_u32Reg, xm_au32Table, uWord & 0xFF);
				uWord >>= 8;
			}
			--i;
		}
	}
	while(pbyRead != pbyEnd){
		DoCrc32Byte(xm_u32Reg, xm_au32Table, *pbyRead);
		++pbyRead;
	}
}
std::uint32_t Crc32::Finalize() noexcept {
	if(xm_bInited){
		xm_u32Reg = ~xm_u32Reg;

		xm_bInited = false;
	}
	return xm_u32Reg;
}
