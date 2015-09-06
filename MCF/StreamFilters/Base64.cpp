// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Base64.hpp"

namespace MCF {

// ========== Base64Encoder ==========
// 构造函数和析构函数。
Base64Encoder::Base64Encoder(const char *pchTable) noexcept {
	std::memcpy(x_abyTable, pchTable, sizeof(x_abyTable));
}

// 其他非静态成员函数。
void Base64Encoder::XDoInit(){
	x_uState = 0;
}
void Base64Encoder::XDoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	if(uSize > 6){
		switch(x_uState){
		case 0:
			break;

		case 1:
			x_ulWord = (x_ulWord << 8) | *pbyRead;
			++pbyRead;
			XOutput(x_abyTable[(x_ulWord >> 4) & 0x3F]);
		case 2:
			x_ulWord = (x_ulWord << 8) | *pbyRead;
			++pbyRead;
			XOutput(x_abyTable[(x_ulWord >> 6) & 0x3F]);
			XOutput(x_abyTable[ x_ulWord       & 0x3F]);
			x_uState = 0;
			break;

		default:
			ASSERT(false);
		}

		register auto i = (std::size_t)(pbyEnd - pbyRead) / 3;
		while(i != 0){
			unsigned long ulWord = pbyRead[0];
			ulWord = (ulWord << 8) | pbyRead[1];
			ulWord = (ulWord << 8) | pbyRead[2];
			pbyRead += 3;

			XOutput(x_abyTable[(ulWord >> 18) & 0x3F]);
			XOutput(x_abyTable[(ulWord >> 12) & 0x3F]);
			XOutput(x_abyTable[(ulWord >>  6) & 0x3F]);
			XOutput(x_abyTable[ ulWord        & 0x3F]);

			--i;
		}
	}
	while(pbyRead != pbyEnd){
		x_ulWord = (x_ulWord << 8) | *pbyRead;
		++pbyRead;

		switch(x_uState){
		case 0:
			XOutput(x_abyTable[(x_ulWord >> 2) & 0x3F]);
			x_uState = 1;
			break;

		case 1:
			XOutput(x_abyTable[(x_ulWord >> 4) & 0x3F]);
			x_uState = 2;
			break;

		case 2:
			XOutput(x_abyTable[(x_ulWord >> 6) & 0x3F]);
			XOutput(x_abyTable[ x_ulWord       & 0x3F]);
			x_uState = 0;
			break;

		default:
			ASSERT(false);
		}
	}
}
void Base64Encoder::XDoFinalize(){
	switch(x_uState){
	case 0:
		break;

	case 1:
		XOutput(x_abyTable[(x_ulWord << 4) & 0x30]);
		if(x_abyTable[64] != 0){
			XOutput(x_abyTable[64]);
			XOutput(x_abyTable[64]);
		}
		x_uState = 0;
		break;

	case 2:
		XOutput(x_abyTable[(x_ulWord << 2) & 0x3C]);
		if(x_abyTable[64] != 0){
			XOutput(x_abyTable[64]);
		}
		x_uState = 0;
		break;

	default:
		ASSERT(false);
	}
}

// ========== Base64Decoder ==========
// 构造函数和析构函数。
Base64Decoder::Base64Decoder(const char *pchTable) noexcept {
	std::memset(x_aschTable, -1, sizeof(x_aschTable));
	for(std::size_t i = 0; i < 64; ++i){
		x_aschTable[(unsigned char)pchTable[i]] = i;
	}
}

// 其他非静态成员函数。
void Base64Decoder::XDoInit(){
	x_uState = 0;
}
void Base64Decoder::XDoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	while(pbyRead != pbyEnd){
		const int nDigit  = x_aschTable[*pbyRead];
		++pbyRead;
		if(nDigit == -1){
			continue;
		}

		x_ulWord = (x_ulWord << 6) | (unsigned char)nDigit;
		switch(x_uState){
		case 0:
			x_uState = 1;
			break;

		case 1:
			XOutput((unsigned char)(x_ulWord >> 4));
			x_uState = 2;
			break;

		case 2:
			XOutput((unsigned char)(x_ulWord >> 2));
			x_uState = 3;
			break;

		case 3:
			XOutput((unsigned char)x_ulWord);
			x_uState = 0;
			break;

		default:
			ASSERT(false);
		}
	}
}
void Base64Decoder::XDoFinalize(){
}

}
