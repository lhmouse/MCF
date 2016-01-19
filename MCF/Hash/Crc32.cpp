// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Crc32.hpp"
#include "../Core/Array.hpp"
#include "../Utilities/Endian.hpp"

// http://www.relisoft.com/science/CrcOptim.html
// 1. 原文提供的是正序（权较大位向权较小位方向）的 Crc 计算，而这里使用的是反序（权较小位向权较大位方向）。
// 2. 原文的 CRC 余数的初始值是 0；此处以 -1 为初始值，计算完成后进行按位反。

// 按照 IEEE 802.3 描述的算法，除数为 0xEDB88320。

namespace MCF {

namespace {
	template<unsigned kRoundT, std::uint32_t kRegT>
	struct ElementGenerator {
		static constexpr std::uint32_t kValue = ElementGenerator<kRoundT + 1, (kRegT >> 1) ^ ((kRegT & 1) ? 0xEDB88320 : 0)>::kValue;
	};
	template<std::uint32_t kRegT>
	struct ElementGenerator<8, kRegT> {
		static constexpr std::uint32_t kValue = kRegT;
	};

	template<std::size_t ...kIndices>
	constexpr Array<std::uint32_t, sizeof...(kIndices)> GenerateTable(std::index_sequence<kIndices...>) noexcept {
		return { ElementGenerator<0, kIndices>::kValue... };
	}

	constexpr auto kCrcTable = GenerateTable(std::make_index_sequence<256>());
}

// 构造函数和析构函数。
Crc32::Crc32() noexcept
	: x_bInited(false)
{
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
		x_u32Reg = kCrcTable[(x_u32Reg ^ byData) & 0xFF] ^ (x_u32Reg >> 8);
	};

	register auto pbyRead = static_cast<const unsigned char *>(pData);
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
