// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CRC32.hpp"
using namespace MCF;

// http://www.relisoft.com/science/CrcOptim.html
// 1. 原文提供的是正序（权较大位向权较小位方向）的 CRC 计算，而这里使用的是反序（权较小位向权较大位方向）。
// 2. 原文的 CRC 余数的初始值是 0；此处以 -1 为初始值，计算完成后进行按位反。

// 构造函数和析构函数。
CRC32::CRC32(std::uint32_t u32Divisor) noexcept {
	ASSERT(u32Divisor != 0);

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
				"shr eax, 1 \n"
				"sbb edx, edx \n"
				"and edx, esi \n"
				"xor eax, edx \n"
				: "=a"(u32Reg)
				: "0"(u32Reg), "S"(u32Divisor)
				: "edx"
			);
		}
		xm_au32Table[i] = u32Reg;
	}
	xm_bInited = false;
}

// 其他非静态成员函数。
void CRC32::Update(const void *pData, std::size_t uSize, bool bIsEndOfStream) noexcept {
	if(!xm_bInited){
		xm_u32Reg = 0xFFFFFFFFu;
		xm_bInited = true;
	}

	register auto pbyRead = (const unsigned char *)pData;
	const auto pbyEnd = pbyRead + uSize;
	register auto u32Reg = xm_u32Reg;

#define PUSH(by)	u32Reg = xm_au32Table[(u32Reg & 0xFF) ^ (by)] ^ (u32Reg >> 8)

	if(uSize >= sizeof(std::uintptr_t) * 2){
		while(((std::uintptr_t)pbyRead & (sizeof(std::uintptr_t) - 1)) != 0){
			PUSH(*(pbyRead++));
		}
		register auto uWordCount = (std::size_t)(pbyEnd - pbyRead) / sizeof(std::uintptr_t);
		while(uWordCount != 0){
			register auto uWord = *(const std::uintptr_t *)pbyRead;
			pbyRead += sizeof(std::uintptr_t);
			for(std::size_t i = 0; i < sizeof(std::uintptr_t); ++i){
				PUSH(uWord & 0xFF);
				uWord >>= 8;
			}
			--uWordCount;
		}
	}
	while(pbyRead != pbyEnd){
		PUSH(*(pbyRead++));
	}
	xm_u32Reg = u32Reg;

	if(bIsEndOfStream){
		xm_bInited = false;
	}
}
std::uint32_t CRC32::GetResult() const noexcept {
	return ~xm_u32Reg;
}
