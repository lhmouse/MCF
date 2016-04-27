// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Hex.hpp"

namespace MCF {

// ========== HexEncoder ==========
void HexEncoder::X_DoInit(){
}
void HexEncoder::X_DoUpdate(const void *pData, std::size_t uSize){
	static constexpr unsigned char kHexTable[] = "00112233445566778899aAbBcCdDeEfF";

	for(std::size_t i = 0; i < uSize; ++i){
		const unsigned uByte = static_cast<const unsigned char *>(pData)[i];
		unsigned char abyHex[2];
		abyHex[0] = kHexTable[(uByte >> 4) * 2 + (unsigned)x_bUpperCase];
		abyHex[1] = kHexTable[(uByte & 0x0F) * 2 + (unsigned)x_bUpperCase];
		X_Output(abyHex, sizeof(abyHex));
	}
}
void HexEncoder::X_DoFinalize(){
}

// ========== HexDecoder ==========
void HexDecoder::X_DoInit(){
	x_nHigh = -1;
}
void HexDecoder::X_DoUpdate(const void *pData, std::size_t uSize){
	static constexpr signed char kHexReverseTable[256] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	};

	for(std::size_t i = 0; i < uSize; ++i){
		const int nDigit = kHexReverseTable[static_cast<const unsigned char *>(pData)[i]];
		if(nDigit == -1){
			continue;
		}
		if(x_nHigh == -1){
			x_nHigh = nDigit;
		} else {
			X_Output((unsigned char)((x_nHigh << 4) | nDigit));
			x_nHigh = -1;
		}
	}
}
void HexDecoder::X_DoFinalize(){
}

}
