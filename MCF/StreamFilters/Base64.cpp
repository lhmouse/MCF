// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Base64.hpp"
using namespace MCF;

// ========== Base64Encoder ==========
// 构造函数和析构函数。
Base64Encoder::Base64Encoder(const char *pchTable) noexcept {
	std::memcpy(xm_abyTable, pchTable, sizeof(xm_abyTable));
}

// 其他非静态成员函数。
void Base64Encoder::xDoInit(){
	xm_uState = 0;
}
void Base64Encoder::xDoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	if(uSize > 6){
		switch(xm_uState){
		case 0:
			break;

		case 1:
			xm_ulWord = (xm_ulWord << 8) | *pbyRead;
			++pbyRead;
			xOutput(xm_abyTable[(xm_ulWord >> 4) & 0x3F]);
		case 2:
			xm_ulWord = (xm_ulWord << 8) | *pbyRead;
			++pbyRead;
			xOutput(xm_abyTable[(xm_ulWord >> 6) & 0x3F]);
			xOutput(xm_abyTable[ xm_ulWord       & 0x3F]);
			xm_uState = 0;
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

			xOutput(xm_abyTable[(ulWord >> 18) & 0x3F]);
			xOutput(xm_abyTable[(ulWord >> 12) & 0x3F]);
			xOutput(xm_abyTable[(ulWord >>  6) & 0x3F]);
			xOutput(xm_abyTable[ ulWord        & 0x3F]);

			--i;
		}
	}
	while(pbyRead != pbyEnd){
		xm_ulWord = (xm_ulWord << 8) | *pbyRead;
		++pbyRead;

		switch(xm_uState){
		case 0:
			xOutput(xm_abyTable[(xm_ulWord >> 2) & 0x3F]);
			xm_uState = 1;
			break;

		case 1:
			xOutput(xm_abyTable[(xm_ulWord >> 4) & 0x3F]);
			xm_uState = 2;
			break;

		case 2:
			xOutput(xm_abyTable[(xm_ulWord >> 6) & 0x3F]);
			xOutput(xm_abyTable[ xm_ulWord       & 0x3F]);
			xm_uState = 0;
			break;

		default:
			ASSERT(false);
		}
	}
}
void Base64Encoder::xDoFinalize(){
	switch(xm_uState){
	case 0:
		break;

	case 1:
		xOutput(xm_abyTable[(xm_ulWord << 4) & 0x30]);
		if(xm_abyTable[64] != 0){
			xOutput(xm_abyTable[64]);
			xOutput(xm_abyTable[64]);
		}
		xm_uState = 0;
		break;

	case 2:
		xOutput(xm_abyTable[(xm_ulWord << 2) & 0x3C]);
		if(xm_abyTable[64] != 0){
			xOutput(xm_abyTable[64]);
		}
		xm_uState = 0;
		break;

	default:
		ASSERT(false);
	}
}

// ========== Base64Decoder ==========
// 构造函数和析构函数。
Base64Decoder::Base64Decoder(const char *pchTable) noexcept {
	std::memset(xm_aschTable, -1, sizeof(xm_aschTable));
	for(std::size_t i = 0; i < 64; ++i){
		xm_aschTable[(unsigned char)pchTable[i]] = i;
	}
}

// 其他非静态成员函数。
void Base64Decoder::xDoInit(){
	xm_uState = 0;
}
void Base64Decoder::xDoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	while(pbyRead != pbyEnd){
		const int nDigit  = xm_aschTable[*pbyRead];
		++pbyRead;
		if(nDigit == -1){
			continue;
		}

		xm_ulWord = (xm_ulWord << 6) | (unsigned char)nDigit;
		switch(xm_uState){
		case 0:
			xm_uState = 1;
			break;

		case 1:
			xOutput((unsigned char)(xm_ulWord >> 4));
			xm_uState = 2;
			break;

		case 2:
			xOutput((unsigned char)(xm_ulWord >> 2));
			xm_uState = 3;
			break;

		case 3:
			xOutput((unsigned char)xm_ulWord);
			xm_uState = 0;
			break;

		default:
			ASSERT(false);
		}
	}
}
void Base64Decoder::xDoFinalize(){
}
