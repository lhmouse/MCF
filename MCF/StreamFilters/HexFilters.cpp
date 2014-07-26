// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "HexFilters.hpp"
using namespace MCF;

namespace {

constexpr unsigned char HEX_DUAL_TABLE[] = "00112233445566778899aAbBcCdDeEfF";

}

// ========== HexEncoder ==========
// 构造函数和析构函数。
HexEncoder::HexEncoder(bool bUpperCase) noexcept
	: xm_bUpperCase(bUpperCase)
{
}

// 其他非静态成员函数。
void HexEncoder::Update(const void *pData, std::size_t uSize){
	auto pbyRead = (const unsigned char *)pData;
	for(std::size_t i = 0; i < uSize; ++i){
		const auto by = *pbyRead;

		unsigned char abyHex[2];
		abyHex[0] = HEX_DUAL_TABLE[((std::uintptr_t)by >> 4) * 2 + (xm_bUpperCase ? 1 : 0)];
		abyHex[1] = HEX_DUAL_TABLE[(by & 0x0F) * 2 + (xm_bUpperCase ? 1 : 0)];

		xOutput(abyHex, sizeof(abyHex));

		StreamFilterBase::Update(pbyRead, 1);
		++pbyRead;
	}
}

// ========== HexDecoder ==========
// 构造函数和析构函数。
HexDecoder::HexDecoder() noexcept
	: xm_nHigh(-1)
{
}

// 其他非静态成员函数。
void HexDecoder::Abort() noexcept {
	xm_nHigh = -1;

	StreamFilterBase::Abort();
}
void HexDecoder::Update(const void *pData, std::size_t uSize){
	auto pbyRead = (const unsigned char *)pData;
	for(std::size_t i = 0; i < uSize; ++i){
		const auto by = *pbyRead;
		unsigned char j = 0;
		while(j < 16){
			if((HEX_DUAL_TABLE[j * 2] == by) || (HEX_DUAL_TABLE[j * 2 + 1] == by)){
				break;
			}
			++j;
		}
		if(j < 16){
			if(xm_nHigh == -1){
				xm_nHigh = j;
			} else {
				xOutput((unsigned char)((xm_nHigh << 4) | j));
				xm_nHigh = -1;
			}
		}

		StreamFilterBase::Update(pbyRead, 1);
		++pbyRead;
	}
}
void HexDecoder::Finalize(){
	xm_nHigh = -1;

	StreamFilterBase::Finalize();
}
