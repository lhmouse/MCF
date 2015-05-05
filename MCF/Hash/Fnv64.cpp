// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Fnv64.hpp"
#include "../Utilities/Endian.hpp"

namespace MCF {

// http://www.isthe.com/chongo/tech/comp/fnv/

// 构造函数和析构函数。
Fnv64::Fnv64() noexcept
	: x_bInited(false)
{
}

// 其他非静态成员函数。
void Fnv64::Abort() noexcept {
	x_bInited = false;
}
void Fnv64::Update(const void *pData, std::size_t uSize) noexcept {
	if(!x_bInited){
		x_u64Reg = 14695981039346656037u;

		x_bInited = true;
	}

	const auto DoFnv64Byte = [&](unsigned char byData){
		x_u64Reg ^= byData;
		x_u64Reg *= 1099511628211u;
	};

	register auto pbyRead = (const unsigned char *)pData;
	const auto pbyEnd = pbyRead + uSize;

	if(uSize >= sizeof(std::uintptr_t) * 2){
		while(((std::uintptr_t)pbyRead & (sizeof(std::uintptr_t) - 1)) != 0){
			DoFnv64Byte(*pbyRead);
			++pbyRead;
		}
		register auto i = (std::size_t)(pbyEnd - pbyRead) / sizeof(std::uintptr_t);
		while(i != 0){
			register auto uWord = LoadLe(*(const std::uintptr_t *)pbyRead);
			pbyRead += sizeof(std::uintptr_t);
			for(unsigned j = 0; j < sizeof(std::uintptr_t); ++j){
				DoFnv64Byte(uWord & 0xFF);
				uWord >>= 8;
			}
			--i;
		}
	}
	while(pbyRead != pbyEnd){
		DoFnv64Byte(*pbyRead);
		++pbyRead;
	}
}
std::uint64_t Fnv64::Finalize() noexcept {
	if(x_bInited){
		x_bInited = false;
	}
	return x_u64Reg;
}

}
