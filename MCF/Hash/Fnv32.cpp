// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Fnv32.hpp"
#include "../Utilities/Endian.hpp"

namespace MCF {

// http://www.isthe.com/chongo/tech/comp/fnv/

// 构造函数和析构函数。
Fnv32::Fnv32() noexcept
	: x_bInited(false)
{
}

// 其他非静态成员函数。
void Fnv32::Abort() noexcept {
	x_bInited = false;
}
void Fnv32::Update(const void *pData, std::size_t uSize) noexcept {
	if(!x_bInited){
		x_u32Reg = 2166136261u;

		x_bInited = true;
	}

	const auto DoFnv32Byte = [&](unsigned char byData){
		x_u32Reg ^= byData;
		x_u32Reg *= 16777619u;
	};

	register auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	if(uSize >= sizeof(std::uintptr_t) * 2){
		while(((std::uintptr_t)pbyRead & (sizeof(std::uintptr_t) - 1)) != 0){
			DoFnv32Byte(*pbyRead);
			++pbyRead;
		}
		register auto i = (std::size_t)(pbyEnd - pbyRead) / sizeof(std::uintptr_t);
		while(i != 0){
			register auto uWord = LoadLe(*(const std::uintptr_t *)pbyRead);
			pbyRead += sizeof(std::uintptr_t);
			for(unsigned j = 0; j < sizeof(std::uintptr_t); ++j){
				DoFnv32Byte(uWord & 0xFF);
				uWord >>= 8;
			}
			--i;
		}
	}
	while(pbyRead != pbyEnd){
		DoFnv32Byte(*pbyRead);
		++pbyRead;
	}
}
std::uint32_t Fnv32::Finalize() noexcept {
	if(x_bInited){
		x_bInited = false;
	}
	return x_u32Reg;
}

}
