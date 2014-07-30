// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Base64Filters.hpp"
#include "../Core/Utilities.hpp"
using namespace MCF;

// ========== Base64Encoder ==========
// 构造函数和析构函数。
Base64Encoder::Base64Encoder(const char *pchTable) noexcept
	: xm_uState(0)
{
	std::memcpy(xm_abyTable, pchTable, sizeof(xm_abyTable));
}

// 其他非静态成员函数。
void Base64Encoder::Abort() noexcept {
	xm_uState = 0;

	StreamFilterBase::Abort();
}
void Base64Encoder::Update(const void *pData, std::size_t uSize){
	auto pbyRead = (const unsigned char *)pData;
	for(std::size_t i = 0; i < uSize; ++i){
		xm_uWord = (xm_uWord << 8) | *pbyRead;

		switch(xm_uState){
		case 0:
			xOutput(xm_abyTable[(xm_uWord >> 2) & 0x3F]);
			xm_uState = 1;
			break;

		case 1:
			xOutput(xm_abyTable[(xm_uWord >> 4) & 0x3F]);
			xm_uState = 2;
			break;

		case 2:
			xOutput(xm_abyTable[(xm_uWord >> 6) & 0x3F]);
			xOutput(xm_abyTable[ xm_uWord       & 0x3F]);
			xm_uState = 0;
			break;

		default:
			ASSERT(false);
		}

		StreamFilterBase::Update(pbyRead, 1);
		++pbyRead;
	}
}
void Base64Encoder::Finalize(){
	switch(xm_uState){
	case 0:
		break;

	case 1:
		xOutput(xm_abyTable[(xm_uWord << 4) & 0x30]);
		if(xm_abyTable[64] != 0){
			xOutput(xm_abyTable[64]);
			xOutput(xm_abyTable[64]);
		}
		break;

	case 2:
		xOutput(xm_abyTable[(xm_uWord << 2) & 0x3C]);
		if(xm_abyTable[64] != 0){
			xOutput(xm_abyTable[64]);
		}
		break;
	}
	xm_uState = 0;

	StreamFilterBase::Finalize();
}

// ========== Base64Decoder ==========
// 构造函数和析构函数。
Base64Decoder::Base64Decoder(const char *pchTable) noexcept
	: xm_uState(0)
{
	std::memset(xm_abyTable, 0xFF, sizeof(xm_abyTable));
	for(std::size_t i = 0; i < 64; ++i){
		xm_abyTable[(unsigned char)pchTable[i]] = i;
	}
}

// 其他非静态成员函数。
void Base64Decoder::Abort() noexcept {
	xm_uState = 0;

	StreamFilterBase::Abort();
}
void Base64Decoder::Update(const void *pData, std::size_t uSize){
	auto pbyRead = (const unsigned char *)pData;
	for(std::size_t i = 0; i < uSize; ++i){
		const auto byDecoded = xm_abyTable[*pbyRead];
		if(byDecoded >= 64){
			continue;
		}
		xm_uWord = (xm_uWord << 6) | byDecoded;

		switch(xm_uState){
		case 0:
			xm_uState = 1;
			break;

		case 1:
			xOutput((unsigned char)(xm_uWord >> 4));
			xm_uState = 2;
			break;

		case 2:
			xOutput((unsigned char)(xm_uWord >> 2));
			xm_uState = 3;
			break;

		case 3:
			xOutput((unsigned char)xm_uWord);
			xm_uState = 0;
			break;

		default:
			ASSERT(false);
		}

		StreamFilterBase::Update(pbyRead, 1);
		++pbyRead;
	}
}
void Base64Decoder::Finalize(){
	xm_uState = 0;

	StreamFilterBase::Finalize();
}
